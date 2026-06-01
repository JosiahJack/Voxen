#include "mod.h"
#define LINE_LEN_MAX 81920
Entity EDefs[MAX_ENTITIES];
#define GEOMETRY_LOD_CARD_MODEL_IDX 178
void* CopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n) { u8 *d=(u8 *)dst; const u8 *s=(const u8 *)src; while (n--) {*d++=*s++;} return dst; } // memcpy replacement
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
    EDefs[285].cardchunk = EDefs[291].cardchunk = EDefs[298].cardchunk = EDefs[299].cardchunk = EDefs[300].cardchunk = EDefs[301].cardchunk = EDefs[303].cardchunk                        = false;
    /*  0 chunk_black*/                 EDefs[  0].modelIndex=178; EDefs[  0].texIndex=0;
    /*  1 chunk_blocker*/               EDefs[  1].modelIndex=178; EDefs[  1].texIndex=1230;EDefs[  1].normIndex=160; EDefs[  1].specIndex=1230;
    /*  2 chunk_bridg1_1*/              EDefs[  2].modelIndex=661; EDefs[  2].texIndex=44;  EDefs[  2].normIndex=43;
    /*  3 chunk_bridg1_1flipx*/         EDefs[  3].modelIndex=667; EDefs[  3].texIndex=44;
    /*  4 chunk_bridg1_2*/              EDefs[  4].modelIndex=662; EDefs[  4].texIndex=45;
    /*  5 chunk_bridg1_3*/              EDefs[  5].modelIndex=20;  EDefs[  5].texIndex=47;
    /*  6 chunk_bridg1_3_slice45*/      EDefs[  6].modelIndex=21;  EDefs[  6].texIndex=47;
    /*  7 chunk_bridg1_3flipx*/         EDefs[  7].modelIndex=663; EDefs[  7].texIndex=47;
    /*  8 chunk_bridg1_4*/              EDefs[  8].modelIndex=22;  EDefs[  8].texIndex=48;
    /*  9 chunk_bridg1_4_slice32*/      EDefs[  9].modelIndex=23;  EDefs[  9].texIndex=48;
    /* 10 chunk_bridg1_4_slice32flipx*/ EDefs[ 10].modelIndex=24;  EDefs[ 10].texIndex=48;
    /* 11 chunk_bridg1_5*/              EDefs[ 11].modelIndex=25;  EDefs[ 11].texIndex=50;  EDefs[ 11].glowIndex=49;
    /* 12 chunk_bridg2_2*/              EDefs[ 12].modelIndex=26;  EDefs[ 12].texIndex=53;
    /* 13 chunk_bridg2_3*/              EDefs[ 13].modelIndex=27;  EDefs[ 13].texIndex=56;  EDefs[ 13].glowIndex=54;  EDefs[ 13].normIndex=55;
    /* 14 chunk_bridg2_4*/              EDefs[ 14].modelIndex=28;  EDefs[ 14].texIndex=57;
    /* 15 chunk_bridg2_5*/              EDefs[ 15].modelIndex=29;  EDefs[ 15].texIndex=59;  EDefs[ 15].normIndex=58;
    /* 16 chunk_bridg2_6*/              EDefs[ 16].modelIndex=30;  EDefs[ 16].texIndex=60;
    /* 17 chunk_bridg2_7*/              EDefs[ 17].modelIndex=664; EDefs[ 17].texIndex=61;
    /* 18 chunk_bridg2_8*/              EDefs[ 18].modelIndex=31;  EDefs[ 18].texIndex=62;
    /* 19 chunk_bridg2_9*/              EDefs[ 19].modelIndex=32;  EDefs[ 19].texIndex=64;  EDefs[ 19].glowIndex=63;
    /* 20 chunk_crate_impenetrable*/    EDefs[ 20].modelIndex=61;  EDefs[ 20].texIndex=150;
    /* 21 chunk_cyberpanel*/            EDefs[ 21].modelIndex=178; EDefs[ 21].texIndex=151; EDefs[ 21].glowIndex=151;
    /* 22 chunk_cyberpanel_slice45*/    EDefs[ 22].modelIndex=180; EDefs[ 22].texIndex=152; EDefs[ 22].glowIndex=152;
    /* 23 chunk_eng1_1*/                EDefs[ 23].modelIndex=96;  EDefs[ 23].texIndex=254;
    /* 24 chunk_eng1_1d*/               EDefs[ 24].modelIndex=95;  EDefs[ 24].texIndex=253;
    /* 25 chunk_eng1_2*/                EDefs[ 25].modelIndex=98;  EDefs[ 25].texIndex=256;
    /* 26 chunk_eng1_2d*/               EDefs[ 26].modelIndex=97;  EDefs[ 26].texIndex=255;
    /* 27 chunk_eng1_3*/                EDefs[ 27].modelIndex=100; EDefs[ 27].texIndex=259; EDefs[ 27].glowIndex=258;
    /* 28 chunk_eng1_3d*/               EDefs[ 28].modelIndex=99;  EDefs[ 28].texIndex=257;
    /* 29 chunk_eng1_4*/                EDefs[ 29].modelIndex=101; EDefs[ 29].texIndex=260;
    /* 30 chunk_eng1_5*/                EDefs[ 30].modelIndex=103; EDefs[ 30].texIndex=262;
    /* 31 chunk_eng1_5_slice45lh*/      EDefs[ 31].modelIndex=104; EDefs[ 31].texIndex=262;
    /* 32 chunk_eng1_5_slice45rh*/      EDefs[ 32].modelIndex=105; EDefs[ 32].texIndex=262;
    /* 33 chunk_eng1_5d*/               EDefs[ 33].modelIndex=102; EDefs[ 33].texIndex=261;
    /* 34 chunk_eng1_6*/                EDefs[ 34].modelIndex=107; EDefs[ 34].texIndex=266; EDefs[ 34].glowIndex=265;
    /* 35 chunk_eng1_6d*/               EDefs[ 35].modelIndex=106; EDefs[ 35].texIndex=264; EDefs[ 35].glowIndex=263;
    /* 36 chunk_eng1_7*/                EDefs[ 36].modelIndex=108; EDefs[ 36].texIndex=269; EDefs[ 36].glowIndex=268;
    /* 37 chunk_eng1_7d*/               EDefs[ 37].modelIndex=665; EDefs[ 37].texIndex=267;
    /* 38 chunk_eng1_8*/                EDefs[ 38].modelIndex=109; EDefs[ 38].texIndex=271; EDefs[ 38].glowIndex=270;
    /* 39 chunk_eng1_9*/                EDefs[ 39].modelIndex=111; EDefs[ 39].texIndex=273; EDefs[ 39].glowIndex=251;
    /* 40 chunk_eng1_9d*/               EDefs[ 40].modelIndex=110; EDefs[ 40].texIndex=272;
    /* 41 chunk_eng2_1*/                EDefs[ 41].modelIndex=113; EDefs[ 41].texIndex=276;
    /* 42 chunk_eng2_1_slice45*/        EDefs[ 42].modelIndex=116; EDefs[ 42].texIndex=276;
    /* 43 chunk_eng2_1_slice384high*/   EDefs[ 43].modelIndex=114; EDefs[ 43].texIndex=276;
    /* 44 chunk_eng2_1_slice384highrh*/ EDefs[ 44].modelIndex=115; EDefs[ 44].texIndex=276;
    /* 45 chunk_eng2_1d*/               EDefs[ 45].modelIndex=112; EDefs[ 45].texIndex=275; EDefs[ 45].glowIndex=274;
    /* 46 chunk_eng2_2*/                EDefs[ 46].modelIndex=117; EDefs[ 46].texIndex=279;
    /* 47 chunk_eng2_2d*/               EDefs[ 47].modelIndex=666; EDefs[ 47].texIndex=277;
    /* 48 chunk_eng2_3*/                EDefs[ 48].modelIndex=119; EDefs[ 48].texIndex=282;
    /* 49 chunk_eng2_3d*/               EDefs[ 49].modelIndex=118; EDefs[ 49].texIndex=281;
    /* 50 chunk_eng2_4*/                EDefs[ 50].modelIndex=178; EDefs[ 50].texIndex=283;
    /* 51 chunk_eng2_5*/                EDefs[ 51].modelIndex=120; EDefs[ 51].texIndex=285; EDefs[ 51].normIndex=284;
    /* 52 chunk_eng2_5_slice45*/        EDefs[ 52].modelIndex=121; EDefs[ 52].texIndex=285; EDefs[ 52].normIndex=284;
    /* 53 chunk_eng2_6 (wall pump)*/                               EDefs[ 53].texIndex=141; EDefs[ 53].glowIndex=142; EDefs[ 53].numclips=1; EDefs[ 53].animationNum=21;
    /* 54 chunk_exec1_1*/               EDefs[ 54].modelIndex=124; EDefs[ 54].texIndex=287;
    /* 55 chunk_exec1_1d*/              EDefs[ 55].modelIndex=123; EDefs[ 55].texIndex=286;
    /* 56 chunk_exec1_2*/               EDefs[ 56].modelIndex=126; EDefs[ 56].texIndex=291; EDefs[ 56].glowIndex=290;
    /* 57 chunk_exec1_2d*/              EDefs[ 57].modelIndex=125; EDefs[ 57].texIndex=289; EDefs[ 57].glowIndex=288;
    /* 58 chunk_exec2_1*/               EDefs[ 58].modelIndex=127; EDefs[ 58].texIndex=292;
    /* 59 chunk_exec2_2*/               EDefs[ 59].modelIndex=129; EDefs[ 59].texIndex=295;
    /* 60 chunk_exec2_2d*/              EDefs[ 60].modelIndex=128; EDefs[ 60].texIndex=294; EDefs[ 60].glowIndex=293;
    /* 61 chunk_exec2_3*/               EDefs[ 61].modelIndex=130; EDefs[ 61].texIndex=296;
    /* 62 chunk_exec2_4*/               EDefs[ 62].modelIndex=131; EDefs[ 62].texIndex=297;
    /* 63 chunk_exec2_4_slice45*/       EDefs[ 63].modelIndex=132; EDefs[ 63].texIndex=297;
    /* 64 chunk_exec2_5*/               EDefs[ 64].modelIndex=133; EDefs[ 64].texIndex=298; EDefs[ 64].specIndex=1257;
    /* 65 chunk_exec2_6*/               EDefs[ 65].modelIndex=134; EDefs[ 65].texIndex=299; EDefs[ 65].specIndex=1257;
    /* 66 chunk_exec2_7*/               EDefs[ 66].modelIndex=133; EDefs[ 66].texIndex=300; EDefs[ 66].specIndex=1257;
    /* 67 chunk_exec3_1*/               EDefs[ 67].modelIndex=127; EDefs[ 67].texIndex=303;
    /* 68 chunk_exec3_1d*/              EDefs[ 68].modelIndex=135; EDefs[ 68].texIndex=302; EDefs[68].glowIndex=301;
    /* 69 chunk_exec3_2*/               EDefs[ 69].modelIndex=129; EDefs[ 69].texIndex=304;
    /* 70 chunk_exec3_4*/               EDefs[ 70].modelIndex=178; EDefs[ 70].texIndex=305;
    /* 71 chunk_exec4_1*/               EDefs[ 71].modelIndex=136; EDefs[ 71].texIndex=307; EDefs[ 71].glowIndex=306;
    /* 72 chunk_exec4_2*/               EDefs[ 72].modelIndex=137; EDefs[ 72].texIndex=308;
    /* 73 chunk_exec4_3*/               EDefs[ 73].modelIndex=138; EDefs[ 73].texIndex=309;
    /* 74 chunk_exec4_4*/               EDefs[ 74].modelIndex=139; EDefs[ 74].texIndex=311;
    /* 75 chunk_exec4_5*/               EDefs[ 75].modelIndex=178; EDefs[ 75].texIndex=312;
    /* 76 chunk_exec4_6*/               EDefs[ 76].modelIndex=141; EDefs[ 76].texIndex=313;
    /* 77 chunk_exec6_1*/               EDefs[ 77].modelIndex=142; EDefs[ 77].texIndex=315; EDefs[ 77].glowIndex=314;
    /* 78 chunk_exteriorpanel1*/        EDefs[ 78].modelIndex=131; EDefs[ 78].texIndex=1228;
    /* 79 chunk_fan1*/                                             EDefs[ 79].texIndex=96;  EDefs[ 79].glowIndex=192; EDefs[ 79].numclips=1; EDefs[ 79].animationNum=22;  
    /* 80 chunk_flight1_1*/             EDefs[ 80].modelIndex=146; EDefs[ 80].texIndex=319;
    /* 81 chunk_flight1_1b*/            EDefs[ 81].modelIndex=146; EDefs[ 81].texIndex=318;
    /* 82 chunk_flight1_2*/             EDefs[ 82].modelIndex=147; EDefs[ 82].texIndex=320;
    /* 83 chunk_flight1_2_slice45rh*/   EDefs[ 83].modelIndex=149; EDefs[ 83].texIndex=320;
    /* 84 unused */
    /* 85 chunk_flight1_4*/             EDefs[ 85].modelIndex=151; EDefs[ 85].texIndex=322;
    /* 86 chunk_flight1_5*/             EDefs[ 86].modelIndex=147; EDefs[ 86].texIndex=323;
    /* 87 chunk_flight1_5_slice45lh*/   EDefs[ 87].modelIndex=148; EDefs[ 87].texIndex=323;
    /* 88 chunk_flight1_6*/             EDefs[ 88].modelIndex=152; EDefs[ 88].texIndex=325;
    /* 89 chunk_flight2_1*/             EDefs[ 89].modelIndex=153; EDefs[ 89].texIndex=326;
    /* 90 chunk_flight2_2*/             EDefs[ 90].modelIndex=154; EDefs[ 90].texIndex=327;
    /* 91 chunk_flight2_2_slice45*/     EDefs[ 91].modelIndex=155; EDefs[ 91].texIndex=327;
    /* 92 chunk_flight2_3*/             EDefs[ 92].modelIndex=156; EDefs[ 92].texIndex=328;
    /* 93 chunk_grove1_1*/              EDefs[ 93].modelIndex=189; EDefs[ 93].texIndex=362;
    /* 94 chunk_grove1_2*/              EDefs[ 94].modelIndex=178; EDefs[ 94].texIndex=363;
    /* 95 chunk_grove1_2_slice45*/      EDefs[ 95].modelIndex=180; EDefs[ 95].texIndex=363;
    /* 96 chunk_grove1_3*/              EDefs[ 96].modelIndex=178; EDefs[ 96].texIndex=364;
    /* 97 chunk_grove1_4*/              EDefs[ 97].modelIndex=178; EDefs[ 97].texIndex=365;
    /* 98 chunk_grove1_5*/              EDefs[ 98].modelIndex=178; EDefs[ 98].texIndex=367;
    /* 99 chunk_grove1_6*/              EDefs[ 99].modelIndex=178; EDefs[ 99].texIndex=368;
    /*100 chunk_grove1_7*/              EDefs[100].modelIndex=178; EDefs[100].texIndex=369;
    /*101 chunk_grove2_1*/              EDefs[101].modelIndex=190; EDefs[101].texIndex=370;
    /*102 chunk_grove2_2*/              EDefs[102].modelIndex=190; EDefs[102].texIndex=371;
    /*103 chunk_grove2_3*/              EDefs[103].modelIndex=191; EDefs[103].texIndex=372;
    /*104 chunk_grove2_4*/              EDefs[104].modelIndex=341; EDefs[104].texIndex=374; EDefs[104].glowIndex=373;
    /*105 chunk_grove2_5*/              EDefs[105].modelIndex=192; EDefs[105].texIndex=375;
    /*106 chunk_grove2_6*/              EDefs[106].modelIndex=192; EDefs[106].texIndex=376;
    /*107 chunk_grove2_7*/              EDefs[107].modelIndex=191; EDefs[107].texIndex=378;
    /*108 chunk_grove2_8*/              EDefs[108].modelIndex=191; EDefs[108].texIndex=379;
    /*109 chunk_grove2_9*/              EDefs[109].modelIndex=191; EDefs[109].texIndex=385;
    /*110 chunk_grove2_9b*/             EDefs[110].modelIndex=191; EDefs[110].texIndex=381;
    /*111 chunk_grove2_9c*/             EDefs[111].modelIndex=191; EDefs[111].texIndex=383;
    /*112 chunk_lift1*/                 EDefs[112].modelIndex=213; EDefs[112].texIndex=1246; EDefs[112].glowIndex=1247;
    /*113 chunk_maint1_1*/              EDefs[113].modelIndex=218; EDefs[113].texIndex=430;
    /*114 chunk_maint1_2*/              EDefs[114].modelIndex=220; EDefs[114].texIndex=432;
    /*115 chunk_maint1_2d*/             EDefs[115].modelIndex=219; EDefs[115].texIndex=431;
    /*116 chunk_maint1_3*/              EDefs[116].modelIndex=222; EDefs[116].texIndex=436; EDefs[116].glowIndex=435; EDefs[116].specIndex=437;
    /*117 chunk_maint1_3b*/             EDefs[117].modelIndex=221; EDefs[117].texIndex=434; EDefs[117].glowIndex=433;
    /*118 chunk_maint1_4*/              EDefs[118].modelIndex=224; EDefs[118].texIndex=441; EDefs[118].glowIndex=440;
    /*119 chunk_maint1_4b*/             EDefs[119].modelIndex=223; EDefs[119].texIndex=439; EDefs[119].glowIndex=438;
    /*120 chunk_maint1_5*/              EDefs[120].modelIndex=225; EDefs[120].texIndex=443; EDefs[120].glowIndex=442;
    /*121 chunk_maint1_6*/              EDefs[121].modelIndex=226; EDefs[121].texIndex=96;
    /*122 chunk_maint1_7*/              EDefs[122].modelIndex=227; EDefs[122].texIndex=447; EDefs[122].glowIndex=446;
    /*123 chunk_blockerflightbay*/      EDefs[123].modelIndex=178; EDefs[123].normIndex=160; EDefs[123].texIndex=1230; EDefs[123].specIndex=1242; EDefs[123].collider=COLLIDER_TYPE_BOX; EDefs[123].colliderCenter=(Vector3){0.0f,1.44f,0.0f}; EDefs[123].colliderSize=(Vector3){2.56f,0.32f,2.56f}; EDefs[123].colliderMeshIndex=U16_MAX;
    /*124 chunk_maint1_9*/              EDefs[124].modelIndex=606; EDefs[124].texIndex=450;
    /*125 chunk_maint1_9d*/             EDefs[125].modelIndex=620; EDefs[125].texIndex=449; EDefs[125].glowIndex=448;
    /*126 chunk_maint2_1*/              EDefs[126].modelIndex=230; EDefs[126].texIndex=455;
    /*127 chunk_maint2_1b*/             EDefs[127].modelIndex=228; EDefs[127].texIndex=451;
    /*128 chunk_maint2_1d*/             EDefs[128].modelIndex=229; EDefs[128].texIndex=453; EDefs[128].glowIndex=452;
    /*129 chunk_maint2_2*/              EDefs[129].modelIndex=230; EDefs[129].texIndex=457;
    /*130 chunk_maint2_3*/              EDefs[130].modelIndex=232; EDefs[130].texIndex=460;
    /*131 chunk_maint2_3d*/             EDefs[131].modelIndex=231; EDefs[131].texIndex=459; EDefs[131].glowIndex=458;
    /*132 chunk_maint2_4*/              EDefs[132].modelIndex=233; EDefs[132].texIndex=464; EDefs[132].glowIndex=463;
    /*133 chunk_maint2_4d*/             EDefs[133].modelIndex=233; EDefs[133].texIndex=462; EDefs[133].glowIndex=461;
    /*134 chunk_maint2_5*/              EDefs[134].modelIndex=235; EDefs[134].texIndex=468; EDefs[134].glowIndex=467;
    /*135 chunk_maint2_5d*/             EDefs[135].modelIndex=234; EDefs[135].texIndex=466; EDefs[135].glowIndex=465;
    /*136 chunk_maint2_6*/              EDefs[136].modelIndex=236; EDefs[136].texIndex=472; EDefs[136].glowIndex=471;
    /*137 chunk_maint2_6d*/             EDefs[137].modelIndex=238; EDefs[137].texIndex=470; EDefs[137].glowIndex=470;
    /*138 chunk_maint2_7*/              EDefs[138].modelIndex=238; EDefs[138].texIndex=476; EDefs[138].glowIndex=475;
    /*139 chunk_maint2_7d*/             EDefs[139].modelIndex=237; EDefs[139].texIndex=474; EDefs[139].glowIndex=473;
    /*140 chunk_maint2_8*/              EDefs[140].modelIndex=239; EDefs[140].texIndex=478; EDefs[140].glowIndex=477;
    /*141 chunk_maint2_9*/              EDefs[141].modelIndex=240; EDefs[141].texIndex=480; EDefs[141].glowIndex=479;
    /*142 chunk_maint2_9_slice45RH*/    EDefs[142].modelIndex=242; EDefs[142].texIndex=480; EDefs[142].glowIndex=479;
    /*143 chunk_maint2_9_slice128_top*/ EDefs[143].modelIndex=241; EDefs[143].texIndex=480; EDefs[143].glowIndex=479;
    /*144 chunk_maint3_1*/              EDefs[144].modelIndex=244; EDefs[144].texIndex=483;
    /*145 chunk_maint3_1_slice32_lh*/   EDefs[145].modelIndex=246; EDefs[145].texIndex=483;
    /*146 chunk_maint3_1_slice32_rh*/   EDefs[146].modelIndex=245; EDefs[146].texIndex=483;
    /*147 chunk_maint3_1_slice45*/      EDefs[147].modelIndex=247; EDefs[147].texIndex=483;
    /*148 chunk_maint3_1d*/             EDefs[148].modelIndex=243; EDefs[148].texIndex=482; EDefs[148].glowIndex=481;
    /*149 chunk_med1_1*/                EDefs[149].modelIndex=249; EDefs[149].texIndex=486; EDefs[149].specIndex=1256; EDefs[149].normIndex=1255;
    /*150 chunk_med1_1_half_top*/       EDefs[150].modelIndex=250; EDefs[150].texIndex=486; EDefs[150].specIndex=1256; EDefs[150].normIndex=1255;
    /*151 chunk_med1_1_slice128high*/   EDefs[151].modelIndex=251; EDefs[151].texIndex=486; EDefs[151].specIndex=1256; EDefs[151].normIndex=1255;
    /*152 chunk_med1_1_slice192RH*/     EDefs[152].modelIndex=252; EDefs[152].texIndex=486; EDefs[152].specIndex=1256; EDefs[152].normIndex=1255;
    /*153 chunk_med1_1_slice256*/       EDefs[153].modelIndex=253; EDefs[153].texIndex=486; EDefs[153].specIndex=1256; EDefs[153].normIndex=1255;
    /*154 chunk_med1_1d*/               EDefs[154].modelIndex=248; EDefs[154].texIndex=485; EDefs[154].glowIndex=484; EDefs[154].specIndex=1236; EDefs[154].normIndex=1255;
    /*155 chunk_med1_2*/                EDefs[155].modelIndex=255; EDefs[155].texIndex=489; EDefs[155].glowIndex=488; EDefs[155].specIndex=1256;
    /*156 chunk_med1_2d*/               EDefs[156].modelIndex=254; EDefs[156].texIndex=487; EDefs[156].specIndex=1256;
    /*157 chunk_med1_3*/                EDefs[157].modelIndex=257; EDefs[157].texIndex=493; EDefs[157].glowIndex=492; EDefs[157].specIndex=1256;
    /*158 chunk_med1_3d*/               EDefs[158].modelIndex=256; EDefs[158].texIndex=491; EDefs[158].glowIndex=490; EDefs[158].specIndex=1256;
    /*159 chunk_med1_4*/                EDefs[159].modelIndex=258; EDefs[159].texIndex=494; EDefs[159].specIndex=1256;
    /*160 chunk_med1_5*/                EDefs[160].modelIndex=669; EDefs[160].texIndex=495; EDefs[160].specIndex=1256;
    /*161 chunk_med1_6*/                EDefs[161].modelIndex=259; EDefs[161].texIndex=496; EDefs[161].normIndex=509; EDefs[161].specIndex=1256;
    /*162 chunk_med1_7*/                EDefs[162].modelIndex=262; EDefs[162].texIndex=499; EDefs[162].specIndex=1268; EDefs[162].normIndex=498;
    /*163 chunk_med1_7_slice14_64*/     EDefs[163].modelIndex=263; EDefs[163].texIndex=499; EDefs[163].specIndex=1268; EDefs[163].normIndex=1254;
    /*164 chunk_med1_7_slice45_320lh*/  EDefs[164].modelIndex=264; EDefs[164].texIndex=499; EDefs[164].specIndex=1268; EDefs[164].normIndex=1254;
    /*165 chunk_med1_7_slice45_320rh*/  EDefs[165].modelIndex=265; EDefs[165].texIndex=499; EDefs[165].specIndex=1268; EDefs[165].normIndex=1254;
    /*166 chunk_med1_7_slice96high*/    EDefs[166].modelIndex=266; EDefs[166].texIndex=499; EDefs[166].specIndex=1268; EDefs[166].normIndex=1254;
    /*167 chunk_med1_7d*/               EDefs[167].modelIndex=260; EDefs[167].texIndex=497; EDefs[167].specIndex=1269; EDefs[167].normIndex=1270;
    /*168 chunk_med1_7d_slice128*/      EDefs[168].modelIndex=261; EDefs[168].texIndex=497; EDefs[168].specIndex=1269; EDefs[168].normIndex=1270;
    /*169 chunk_med1_8*/                EDefs[169].modelIndex=268; EDefs[169].texIndex=503; EDefs[169].normIndex=502; EDefs[169].specIndex=1242;
    /*170 chunk_med1_8d*/               EDefs[170].modelIndex=267; EDefs[170].texIndex=501; EDefs[170].normIndex=163; EDefs[170].specIndex=1242;
    /*171 chunk_med1_9*/                EDefs[171].modelIndex=278; EDefs[171].texIndex=507; EDefs[171].normIndex=506; EDefs[171].specIndex=1267;
    /*172 unused*/
    /*173 unused*/
    /*174 chunk_med1_9d*/               EDefs[174].modelIndex=269; EDefs[174].texIndex=505; EDefs[174].normIndex=504; EDefs[174].specIndex=1267;
    /*175 unused*/
    /*176 chunk_med1_9d_ofs112_90*/     EDefs[176].modelIndex=270; EDefs[176].texIndex=505; EDefs[176].normIndex=504; EDefs[176].specIndex=1267; EDefs[176].collider=COLLIDER_TYPE_MESH; EDefs[176].colliderMeshIndex=270;
    /*177 chunk_med1_9d_ofs144_90*/     EDefs[177].modelIndex=272; EDefs[177].texIndex=505; EDefs[177].normIndex=504; EDefs[177].specIndex=1267; EDefs[177].collider=COLLIDER_TYPE_MESH; EDefs[177].colliderMeshIndex=272;
    /*178 chunk_med2_1*/                EDefs[178].modelIndex=280; EDefs[178].texIndex=513; EDefs[178].specIndex=1254; EDefs[178].glowIndex=511; EDefs[178].normIndex=512;
    /*179 chunk_med2_1_slice32RH*/      EDefs[179].modelIndex=281; EDefs[179].texIndex=513; EDefs[179].normIndex=512; EDefs[179].specIndex=1254;
    /*180 chunk_med2_1d*/               EDefs[180].modelIndex=279; EDefs[180].glowIndex=508; EDefs[180].texIndex=510; EDefs[180].specIndex=1254;
    /*181 chunk_med2_2*/                EDefs[181].modelIndex=283; EDefs[181].texIndex=517; EDefs[181].glowIndex=516; EDefs[181].specIndex=1242;
    /*182 chunk_med2_2_half_bottom*/    EDefs[182].modelIndex=284; EDefs[182].texIndex=517; EDefs[182].glowIndex=516; EDefs[182].specIndex=1242;
    /*183 chunk_med2_2d*/               EDefs[183].modelIndex=282; EDefs[183].texIndex=515; EDefs[183].glowIndex=516; EDefs[183].specIndex=1242;
    /*184 chunk_med2_3*/                EDefs[184].modelIndex=286; EDefs[184].texIndex=521; EDefs[184].glowIndex=520; EDefs[184].specIndex=1242;
    /*185 chunk_med2_3d*/               EDefs[185].modelIndex=285; EDefs[185].texIndex=519; EDefs[185].glowIndex=518; EDefs[185].specIndex=1242;
    /*186 chunk_med2_4*/                EDefs[186].modelIndex=287; EDefs[186].texIndex=523; EDefs[186].glowIndex=522; EDefs[186].specIndex=1242;
    /*187 chunk_med2_5*/                EDefs[187].modelIndex=288; EDefs[187].texIndex=527; EDefs[187].glowIndex=526; EDefs[187].specIndex=539; EDefs[187].collider=COLLIDER_TYPE_BOX; EDefs[187].colliderCenter=(Vector3){0.0f,1.44f,0.0f}; EDefs[187].colliderSize=(Vector3){2.56f,0.32f,2.56f}; EDefs[187].colliderMeshIndex=U16_MAX;
    /*188 chunk_med2_6*/                EDefs[188].modelIndex=289; EDefs[188].texIndex=528; EDefs[188].specIndex=1271;
    /*189 chunk_med2_7*/                EDefs[189].modelIndex=290; EDefs[189].texIndex=530; EDefs[189].glowIndex=529; EDefs[189].specIndex=1245;
    /*190 chunk_med2_8*/                EDefs[190].modelIndex=291; EDefs[190].texIndex=531; EDefs[190].specIndex=1242;
    /*191 chunk_med2_8_half_top*/       EDefs[191].modelIndex=292; EDefs[191].texIndex=531; EDefs[191].specIndex=1242;
    /*192 chunk_med2_8_slice32RH*/      EDefs[192].modelIndex=293; EDefs[192].texIndex=531; EDefs[192].specIndex=1242;
    /*193 chunk_med2_8_slice45*/        EDefs[193].modelIndex=294; EDefs[193].texIndex=531; EDefs[193].specIndex=1242;
    /*194 chunk_med2_9*/                EDefs[194].modelIndex=296; EDefs[194].texIndex=535; EDefs[194].glowIndex=534; EDefs[194].specIndex=1242;
    /*195 chunk_med2_9d*/               EDefs[195].modelIndex=295; EDefs[195].texIndex=533; EDefs[195].glowIndex=532; EDefs[195].specIndex=1242;
    /*196 chunk_med3_1*/                EDefs[196].modelIndex=297; EDefs[196].texIndex=536; EDefs[196].specIndex=1236;
    /*197 chunk_rad1_1*/                EDefs[197].modelIndex=501; EDefs[197].texIndex=660; EDefs[197].glowIndex=659; EDefs[197].specIndex=1231;
    /*198 chunk_rad1_2*/                EDefs[198].modelIndex=501; EDefs[198].texIndex=662; EDefs[198].glowIndex=661; EDefs[198].specIndex=1231;
    /*199 chunk_reac1_1*/               EDefs[199].modelIndex=502; EDefs[199].texIndex=664; EDefs[199].specIndex=1243;
    /*200 chunk_reac1_1_slice45*/       EDefs[200].modelIndex=339; EDefs[200].texIndex=664; EDefs[200].specIndex=1243;
    /*201 chunk_reac1_2*/               EDefs[201].modelIndex=503; EDefs[201].texIndex=665; EDefs[201].specIndex=1243;
    /*202 chunk_reac1_3*/               EDefs[202].modelIndex=504; EDefs[202].texIndex=666; EDefs[202].specIndex=1243;
    /*203 chunk_reac1_4*/               EDefs[203].modelIndex=505; EDefs[203].texIndex=668; EDefs[203].glowIndex=667; EDefs[203].specIndex=669;
    /*204 chunk_reac1_5*/               EDefs[204].modelIndex=506; EDefs[204].texIndex=671; EDefs[204].glowIndex=670; EDefs[204].specIndex=1239;
    /*205 chunk_reac1_6*/               EDefs[205].modelIndex=507; EDefs[205].texIndex=673; EDefs[205].glowIndex=672; EDefs[205].specIndex=1243;
    /*206 chunk_reac1_7*/               EDefs[206].modelIndex=342; EDefs[206].texIndex=676; EDefs[206].glowIndex=675; EDefs[206].specIndex=1243;
    /*207 chunk_reac1_8*/               EDefs[207].modelIndex=508; EDefs[207].texIndex=678; EDefs[207].glowIndex=678; EDefs[207].specIndex=1243;
    /*208 chunk_reac1_9*/               EDefs[208].modelIndex=509; EDefs[208].texIndex=680; EDefs[208].glowIndex=680; EDefs[208].specIndex=1243;
    /*209 chunk_reac2_1*/               EDefs[209].modelIndex=512; EDefs[209].texIndex=682; EDefs[209].specIndex=1235;
    /*210 chunk_reac2_1_slice45LH*/     EDefs[210].modelIndex=514; EDefs[210].texIndex=682; EDefs[210].specIndex=1235;
    /*211 chunk_reac2_1_slice45LH_up*/  EDefs[211].modelIndex=515; EDefs[211].texIndex=682; EDefs[211].specIndex=1235;
    /*212 chunk_reac2_1_slice45RH*/     EDefs[212].modelIndex=516; EDefs[212].texIndex=682; EDefs[212].specIndex=1235;
    /*213 chunk_reac2_1_slice45RH_up*/  EDefs[213].modelIndex=517; EDefs[213].texIndex=682; EDefs[213].specIndex=1235;
    /*214 chunk_reac2_1b*/              EDefs[214].modelIndex=510; EDefs[214].texIndex=681; EDefs[214].specIndex=1235;
    /*215 chunk_reac2_1bmirror*/        EDefs[215].modelIndex=511; EDefs[215].texIndex=681; EDefs[215].specIndex=1235;
    /*216 chunk_reac2_1mirror*/         EDefs[216].modelIndex=513; EDefs[216].texIndex=682; EDefs[216].specIndex=1235;
    /*217 chunk_reac2_2*/               EDefs[217].modelIndex=518; EDefs[217].texIndex=684; EDefs[217].glowIndex=683; EDefs[217].specIndex=1235;
    /*218 chunk_reac2_4*/               EDefs[218].modelIndex=519; EDefs[218].texIndex=685; EDefs[218].specIndex=1235;
    /*219 chunk_reac2_4_slice128lower*/ EDefs[219].modelIndex=340; EDefs[219].texIndex=685; EDefs[219].specIndex=1235;
    /*220 chunk_reac2_5*/               EDefs[220].modelIndex=520; EDefs[220].texIndex=687; EDefs[220].glowIndex=686;
    /*221 chunk_reac2_6*/               EDefs[221].modelIndex=521; EDefs[221].texIndex=689; EDefs[221].glowIndex=688;
    /*222 chunk_reac2_7*/               EDefs[222].modelIndex=522; EDefs[222].texIndex=691; EDefs[222].glowIndex=690;
    /*223 chunk_reac2_8*/               EDefs[223].modelIndex=523; EDefs[223].texIndex=693; EDefs[223].glowIndex=692;
    /*224 chunk_reac2_9*/               EDefs[224].modelIndex=524; EDefs[224].texIndex=694;
    /*225 chunk_reac2_1*/               EDefs[225].modelIndex=525; EDefs[225].texIndex=696; EDefs[225].glowIndex=695;
    /*226 chunk_reac3_2*/               EDefs[226].modelIndex=526; EDefs[226].texIndex=697;
    /*227 chunk_reac3_3*/               EDefs[227].modelIndex=527; EDefs[227].texIndex=698;
    /*228 chunk_reac3_4*/               EDefs[228].modelIndex=528; EDefs[228].texIndex=699;
    /*229 chunk_reac3_5*/               EDefs[229].modelIndex=529; EDefs[229].texIndex=701; EDefs[229].glowIndex=700;
    /*230 chunk_reac3_6*/               EDefs[230].modelIndex=530; EDefs[230].texIndex=703; EDefs[230].glowIndex=702;
    /*231 chunk_reac3_7*/               EDefs[231].modelIndex=531; EDefs[231].texIndex=704; EDefs[231].specIndex=705;
    /*232 chunk_reac4_1*/               EDefs[232].modelIndex=532; EDefs[232].texIndex=707; EDefs[232].glowIndex=706;
    /*233 chunk_reac4_1_slice45lh*/     EDefs[233].modelIndex=533; EDefs[233].texIndex=707;
    /*234 chunk_reac4_2*/               EDefs[234].modelIndex=534; EDefs[234].texIndex=709; EDefs[234].glowIndex=708;
    /*235 chunk_reac5_1*/               EDefs[235].modelIndex=535; EDefs[235].texIndex=711; EDefs[235].glowIndex=710;
    /*236 chunk_reac5_2*/               EDefs[236].modelIndex=536; EDefs[236].texIndex=713; EDefs[236].glowIndex=712;
    /*237 chunk_reac5_3*/               EDefs[237].modelIndex=537; EDefs[237].texIndex=715; EDefs[237].glowIndex=714;
    /*238 chunk_reac6_1*/               EDefs[238].modelIndex=538; EDefs[238].texIndex=716;
    /*239 chunk_reac6_2*/               EDefs[239].modelIndex=539; EDefs[239].texIndex=717;
    /*240 chunk_reac6_3*/               EDefs[240].modelIndex=539; EDefs[240].texIndex=719; EDefs[240].glowIndex=718;
    /*241 chunk_sci1_1*/                EDefs[241].modelIndex=540; EDefs[241].texIndex=722; 
    /*242 chunk_sci1_1_slice45_toplh*/  EDefs[242].modelIndex=542; EDefs[242].texIndex=722; 
    /*243 chunk_sci1_1_slice45_toprh*/  EDefs[243].modelIndex=543; EDefs[243].texIndex=722; 
    /*244 chunk_sci1_1d*/               EDefs[244].modelIndex=541; EDefs[244].texIndex=721; 
    /*245 chunk_sci1_2*/                EDefs[245].modelIndex=545; EDefs[245].texIndex=724; 
    /*246 chunk_sci1_2_slice45lh*/      EDefs[246].modelIndex=546; EDefs[246].texIndex=724; 
    /*247 chunk_sci1_2_slice45lh_up*/   EDefs[247].modelIndex=547; EDefs[247].texIndex=724; 
    /*248 chunk_sci1_2_slice45rh*/      EDefs[248].modelIndex=548; EDefs[248].texIndex=724; 
    /*249 chunk_sci1_2_slice45rh_up*/   EDefs[249].modelIndex=549; EDefs[249].texIndex=724; 
    /*250 chunk_sci1_2d*/               EDefs[250].modelIndex=544; EDefs[250].texIndex=723; 
    /*251 chunk_sci1_3*/                EDefs[251].modelIndex=550; EDefs[251].texIndex=726; EDefs[251].glowIndex=725; 
    /*252 chunk_sci1_4*/                EDefs[252].modelIndex=498; EDefs[252].texIndex=727; 
    /*253 chunk_sci1_5*/                EDefs[253].modelIndex=551; EDefs[253].texIndex=728; 
    /*254 chunk_sci1_6*/                EDefs[254].modelIndex=552; EDefs[254].texIndex=729; 
    /*255 chunk_sci1_6_slice45*/        EDefs[255].modelIndex=553; EDefs[255].texIndex=729; 
    /*256 chunk_sci1_7*/                EDefs[256].modelIndex=555; EDefs[256].texIndex=731; 
    /*257 chunk_sci1_7d*/               EDefs[257].modelIndex=554; EDefs[257].texIndex=730; 
    /*258 chunk_sci1_8*/                EDefs[258].modelIndex=557; EDefs[258].texIndex=734; 
    /*259 chunk_sci1_8d*/               EDefs[259].modelIndex=556; EDefs[259].texIndex=733; 
    /*260 chunk_sci1_9*/                EDefs[260].modelIndex=559; EDefs[260].texIndex=737; 
    /*261 chunk_sci1_9d*/               EDefs[261].modelIndex=558; EDefs[261].texIndex=736; EDefs[261].glowIndex=735; 
    /*262 chunk_sci2_1*/                EDefs[262].modelIndex=561; EDefs[262].texIndex=739; 
    /*263 chunk_sci2_1_slice45lh*/      EDefs[263].modelIndex=563; EDefs[263].texIndex=739; 
    /*264 chunk_sci2_1_slice45rh*/      EDefs[264].modelIndex=562; EDefs[264].texIndex=739; 
    /*265 chunk_sci2_1d*/               EDefs[265].modelIndex=560; EDefs[265].texIndex=738; 
    /*266 chunk_sci2_2*/                EDefs[266].modelIndex=565; EDefs[266].texIndex=742; EDefs[266].glowIndex=741; 
    /*267 chunk_sci2_2d*/               EDefs[267].modelIndex=564; EDefs[267].texIndex=740; 
    /*268 chunk_sci2_3*/                EDefs[268].modelIndex=566; EDefs[268].texIndex=744; EDefs[268].glowIndex=743; 
    /*269 chunk_sci2_4*/                EDefs[269].modelIndex=567; EDefs[269].texIndex=745; 
    /*270 chunk_sci2_5*/                EDefs[270].modelIndex=569; EDefs[270].texIndex=747; 
    /*271 chunk_sci2_5d*/               EDefs[271].modelIndex=568; EDefs[271].texIndex=746; 
    /*272 chunk_sci3_1*/                EDefs[272].modelIndex=571; EDefs[272].texIndex=749; 
    /*273 chunk_sci3_1d*/               EDefs[273].modelIndex=570; EDefs[273].texIndex=748; 
    /*274 chunk_sci3_2*/                EDefs[274].modelIndex=572; EDefs[274].texIndex=750; 
    /*275 chunk_sci3_3*/                EDefs[275].modelIndex=573; EDefs[275].texIndex=752; EDefs[275].glowIndex=751; 
    /*276 chunk_sci3_4*/                EDefs[276].modelIndex=574; EDefs[276].texIndex=754; 
    /*277 chunk_sci3_5*/                EDefs[277].modelIndex=575; EDefs[277].texIndex=756; EDefs[277].glowIndex=755; 
    /*278 chunk_sci3_6*/                EDefs[278].modelIndex=576; EDefs[278].texIndex=758; EDefs[278].glowIndex=757; 
    /*279 chunk_screen*/                EDefs[279].modelIndex=5988;EDefs[279].texIndex=881; 
    /*280 chunk_sec1_1*/                EDefs[280].modelIndex=178; EDefs[280].texIndex=787; EDefs[280].specIndex=787; 
    /*281 chunk_sec1_1b*/               EDefs[281].modelIndex=178; EDefs[281].texIndex=785; EDefs[281].specIndex=785; 
    /*282 chunk_sec1_1c*/               EDefs[282].modelIndex=577; EDefs[282].texIndex=786; EDefs[282].specIndex=786; 
    /*283 chunk_sec1_1c_slice45*/       EDefs[283].modelIndex=580; EDefs[283].texIndex=786; EDefs[283].specIndex=786; 
    /*284 chunk_sec1_1c_slice64highlh*/ EDefs[284].modelIndex=581; EDefs[284].texIndex=786; EDefs[284].specIndex=786; 
    /*285 chunk_sec1_1c_slice64highrh*/ EDefs[285].modelIndex=582; EDefs[285].texIndex=786; EDefs[285].specIndex=786; 
    /*286 unused*/
    /*287 unused*/
    /*288 chunk_sec1_2*/                EDefs[288].modelIndex=584; EDefs[288].texIndex=789; EDefs[288].specIndex=1233; 
    /*289 chunk_sec1_2b*/               EDefs[289].modelIndex=583; EDefs[289].texIndex=788; EDefs[289].specIndex=1233; 
    /*290 chunk_sec1_3*/                EDefs[290].modelIndex=585; EDefs[290].texIndex=790; EDefs[290].specIndex=1233; 
    /*291 chunk_sec1_3_slice45*/        EDefs[291].modelIndex=586; EDefs[291].texIndex=790; EDefs[291].specIndex=1233; 
    /*292 chunk_stor1_1*/               EDefs[292].modelIndex=597; EDefs[292].texIndex=824; EDefs[292].glowIndex=823; 
    /*293 chunk_stor1_2*/               EDefs[293].modelIndex=598; EDefs[293].texIndex=825; 
    /*294 chunk_stor1_3*/               EDefs[294].modelIndex=598; EDefs[294].texIndex=826; 
    /*295 chunk_stor1_4*/               EDefs[295].modelIndex=599; EDefs[295].texIndex=827; 
    /*296 chunk_stor1_5*/               EDefs[296].modelIndex=600; EDefs[296].texIndex=828; 
    /*297 chunk_stor1_6*/               EDefs[297].modelIndex=601; EDefs[297].texIndex=829; 
    /*298 chunk_stor1_6_slice128_up_lh*/EDefs[298].modelIndex=602; EDefs[298].texIndex=829; 
    /*299 chunk_stor1_6_slice128_up_rh*/EDefs[299].modelIndex=603; EDefs[299].texIndex=829; 
    /*300 chunk_stor1_6_slice192lh*/    EDefs[300].modelIndex=604; EDefs[300].texIndex=829; 
    /*301 chunk_stor1_6_slice192rh*/    EDefs[301].modelIndex=605; EDefs[301].texIndex=829; 
    /*302 chunk_stor1_7*/               EDefs[302].modelIndex=606; EDefs[302].texIndex=833; EDefs[302].specIndex=834; EDefs[302].normIndex=832; 
    /*303 chunk_stor1_7_slice45*/       EDefs[303].modelIndex=607; EDefs[303].texIndex=833; EDefs[303].specIndex=834; EDefs[303].normIndex=832; 
    /*304 chunk_stor1_7d*/              EDefs[304].modelIndex=620; EDefs[304].texIndex=831; EDefs[304].glowIndex=830; EDefs[304].normIndex=832; EDefs[304].specIndex=834; 
    /*305 chunk_teleporter*/            EDefs[305].modelIndex=178; EDefs[305].texIndex=1166; 
    /*306 chunk_white*/                 EDefs[306].modelIndex=178; EDefs[306].texIndex=881;

    // Item Definitions
    /*307 item_paper_wad*/  EDefs[307].modelIndex=487; EDefs[307].texIndex=1250; EDefs[307].collider= COLLIDER_TYPE_SPHERE; EDefs[307].colliderCenter=(Vector3){-0.001254f,-0.001190498f,0.006335999f}; EDefs[307].colliderSize.x=0.0451f; EDefs[307].mass=0.06f; EDefs[307].angularDrag=0.05f; EDefs[307].dynamicFriction=0.6f; EDefs[307].staticFriction=0.6f;
    /*308 item_warecasing*/ EDefs[308].modelIndex=637; EDefs[308].texIndex=1251;  EDefs[308].mass=0.8f; EDefs[308].angularDrag=0.05f; EDefs[308].dynamicFriction=0.6f;  EDefs[308].staticFriction=0.6f;
    /*309 item_beaker*/     EDefs[309].modelIndex=14; EDefs[309].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[309].colliderMeshIndex=682;  EDefs[309].texIndex=36;  EDefs[309].specIndex=1242;  EDefs[309].mass=0.28f; EDefs[309].angularDrag=0.05f; EDefs[309].dynamicFriction=0.6f;  EDefs[309].staticFriction=0.6f;
    /*310 item_beverage*/   EDefs[310].modelIndex=18;  EDefs[310].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[310].colliderMeshIndex=683;  EDefs[310].texIndex=37;  EDefs[310].mass=0.12f; EDefs[310].angularDrag=0.05f; EDefs[310].dynamicFriction=0.6f;  EDefs[310].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[311].path,"item_skull",11); EDefs[311].modelIndex=593;  EDefs[311].mass=0.451f; EDefs[311].angularDrag=0.05f; EDefs[311].dynamicFriction=0.6f;  EDefs[311].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[312].path,"item_arm",9); EDefs[312].modelIndex=7;  EDefs[312].texIndex=28;  EDefs[312].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[312].colliderMeshIndex=678;  EDefs[312].mass=1.0f; EDefs[312].angularDrag=0.05f; EDefs[312].dynamicFriction=0.6f;  EDefs[312].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[313].path,"item_audiolog",14); EDefs[313].modelIndex=11;  EDefs[313].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[313].colliderMeshIndex=679;  EDefs[313].texIndex=52;  EDefs[313].glowIndex=80;  EDefs[313].mass=0.2f; EDefs[313].angularDrag=0.05f; EDefs[313].dynamicFriction=0.6f;  EDefs[313].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[314].path,"weapon_grenadefrag",19); EDefs[314].modelIndex=182;  EDefs[314].mass=1.0f; EDefs[314].angularDrag=0.05f; EDefs[314].dynamicFriction=0.6f;  EDefs[314].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[315].path,"weapon_grenadeconc",19); EDefs[315].modelIndex=165;  EDefs[315].mass=1.0f; EDefs[315].angularDrag=0.05f; EDefs[315].dynamicFriction=0.6f;  EDefs[315].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[316].path,"weapon_grenadeemp",18); EDefs[316].modelIndex=168;  EDefs[316].mass=1.0f; EDefs[316].angularDrag=0.05f; EDefs[316].dynamicFriction=0.6f;  EDefs[316].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[317].path,"weapon_grenadeearth",20); EDefs[317].modelIndex=181;  EDefs[317].mass=1.0f; EDefs[317].angularDrag=0.05f; EDefs[317].dynamicFriction=0.6f;  EDefs[317].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[318].path,"weapon_grenademine",19); EDefs[318].modelIndex=184;  EDefs[318].mass=1.0f; EDefs[318].angularDrag=0.05f; EDefs[318].dynamicFriction=0.6f;  EDefs[318].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[319].path,"weapon_grenadenitro",20); EDefs[319].modelIndex=185;  EDefs[319].mass=1.0f; EDefs[319].angularDrag=0.05f; EDefs[319].dynamicFriction=0.6f;  EDefs[319].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[320].path,"weapon_grenadegas",18); EDefs[320].modelIndex=183;  EDefs[320].mass=1.0f; EDefs[320].angularDrag=0.05f; EDefs[320].dynamicFriction=0.6f;  EDefs[320].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[321].path,"item_patch_berserk",19); EDefs[321].modelIndex=488;  EDefs[321].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[321].colliderMeshIndex=491;  EDefs[321].texIndex=590;  EDefs[321].mass=0.12f; EDefs[321].angularDrag=0.05f; EDefs[321].dynamicFriction=0.6f;  EDefs[321].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[322].path,"item_patch_detox",17); EDefs[322].modelIndex=488;  EDefs[322].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[322].colliderMeshIndex=491;  EDefs[322].texIndex=591;  EDefs[322].mass=0.12f; EDefs[322].angularDrag=0.05f; EDefs[322].dynamicFriction=0.6f;  EDefs[322].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[323].path,"item_patch_genius",18); EDefs[323].modelIndex=488;  EDefs[323].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[323].colliderMeshIndex=491;  EDefs[323].texIndex=592;  EDefs[323].mass=0.12f; EDefs[323].angularDrag=0.05f; EDefs[323].dynamicFriction=0.6f;  EDefs[323].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[324].path,"item_patch_medi",16); EDefs[324].modelIndex=488;  EDefs[324].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[324].colliderMeshIndex=491;  EDefs[324].texIndex=600;  EDefs[324].mass=0.12f; EDefs[324].angularDrag=0.05f; EDefs[324].dynamicFriction=0.6f;  EDefs[324].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[325].path,"item_patch_reflex",18); EDefs[325].modelIndex=488;  EDefs[325].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[325].colliderMeshIndex=491;  EDefs[325].texIndex=641;  EDefs[325].mass=0.12f;  EDefs[325].angularDrag=0.05f; EDefs[325].dynamicFriction=0.6f;  EDefs[325].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[326].path,"item_patch_sight",17); EDefs[326].modelIndex=488;  EDefs[326].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[326].colliderMeshIndex=491;  EDefs[326].texIndex=646;  EDefs[326].mass=0.12f;  EDefs[326].angularDrag=0.05f; EDefs[326].dynamicFriction=0.6f;  EDefs[326].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[327].path,"item_patch_staminup",20); EDefs[327].modelIndex=488;  EDefs[327].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[327].colliderMeshIndex=491;  EDefs[327].texIndex=647;  EDefs[327].mass=0.12f; EDefs[327].angularDrag=0.05f; EDefs[327].dynamicFriction=0.6f;  EDefs[327].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[328].path,"item_hw_system",15); EDefs[328].modelIndex=207;  EDefs[328].texIndex=405;  EDefs[328].glowIndex=404;  EDefs[328].mass=0.17f;  EDefs[328].angularDrag=0.05f; EDefs[328].dynamicFriction=0.6f;  EDefs[328].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[329].path,"item_hw_navunit",16); EDefs[329].modelIndex=204;  EDefs[329].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[329].colliderMeshIndex=696;  EDefs[329].texIndex=1258;  EDefs[329].glowIndex=1259;  EDefs[329].mass=0.1f; EDefs[329].angularDrag=0.05f; EDefs[329].dynamicFriction=0.6f;  EDefs[329].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[330].path,"item_hw_ereader",16); EDefs[330].modelIndex=200;  EDefs[330].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[330].colliderMeshIndex=692;  EDefs[330].mass=0.12f;  EDefs[330].angularDrag=0.05f; EDefs[330].dynamicFriction=0.6f;  EDefs[330].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[331].path,"item_hw_sensaround",19); EDefs[331].modelIndex=205;  EDefs[331].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[331].colliderMeshIndex=697;  EDefs[331].mass=0.12f; EDefs[331].angularDrag=0.05f; EDefs[331].dynamicFriction=0.6f;  EDefs[331].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[332].path,"item_hw_targetid",17); EDefs[332].modelIndex=208;  EDefs[332].mass=0.08f;  EDefs[332].angularDrag=0.05f; EDefs[332].dynamicFriction=0.6f;  EDefs[332].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[333].path,"item_hw_shield",15); EDefs[333].modelIndex=206;  EDefs[333].mass=0.14f;  EDefs[333].angularDrag=0.05f; EDefs[333].dynamicFriction=0.6f;  EDefs[333].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[334].path,"item_hw_bio",12); EDefs[334].modelIndex=197;  EDefs[334].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[334].colliderMeshIndex=689;  EDefs[334].mass=0.1f;   EDefs[334].angularDrag=0.05f; EDefs[334].dynamicFriction=0.6f;  EDefs[334].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[335].path,"item_hw_lantern",16); EDefs[335].modelIndex=203;  EDefs[335].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[335].colliderMeshIndex=695;  EDefs[335].mass=0.11f;  EDefs[335].angularDrag=0.05f; EDefs[335].dynamicFriction=0.6f;  EDefs[335].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[336].path,"item_hw_envirosuit",19); EDefs[336].modelIndex=199;  EDefs[336].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[336].colliderMeshIndex=691;  EDefs[336].mass=0.451f;   EDefs[336].angularDrag=0.05f; EDefs[336].dynamicFriction=0.6f;  EDefs[336].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[337].path,"item_hw_booster",16); EDefs[337].modelIndex=198;  EDefs[337].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[337].colliderMeshIndex=690;  EDefs[337].mass=0.16f;  EDefs[337].angularDrag=0.05f; EDefs[337].dynamicFriction=0.6f;  EDefs[337].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[338].path,"item_hw_jumpjets",17); EDefs[338].modelIndex=202;  EDefs[338].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[338].colliderMeshIndex=694;  EDefs[338].mass=0.32f; EDefs[338].angularDrag=0.05f;  EDefs[338].dynamicFriction=0.6f;  EDefs[338].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[339].path,"item_hw_infrared",17); EDefs[339].modelIndex=201;  EDefs[339].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[339].colliderMeshIndex=693;  EDefs[339].mass=0.1f; EDefs[339].angularDrag=0.05f;  EDefs[339].dynamicFriction=0.6f;  EDefs[339].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[340].path,"item_fireextinguisher",22); EDefs[340].modelIndex=144;  EDefs[340].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[340].colliderMeshIndex=684;  EDefs[340].mass=1.3f; EDefs[340].angularDrag=0.05f; EDefs[340].dynamicFriction=0.6f;  EDefs[340].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[341].path,"item_access_card_admin",23); EDefs[341].modelIndex=0;  EDefs[341].texIndex=9;  EDefs[341].glowIndex=82;  EDefs[341].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[341].colliderMeshIndex=672;  EDefs[341].mass=0.2f;  EDefs[341].angularDrag=0.05f; EDefs[341].dynamicFriction=0.6f;  EDefs[341].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[342].path,"item_workerhelmet",18); EDefs[342].modelIndex=648;  EDefs[342].mass=1.2f;  EDefs[342].angularDrag=0.05f; EDefs[342].dynamicFriction=0.6f;  EDefs[342].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[343].path,"weapon_mk3",11); EDefs[343].modelIndex=646;  EDefs[343].mass=0.75f;  EDefs[343].angularDrag=0.05f; EDefs[343].dynamicFriction=0.6f;  EDefs[343].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[344].path,"weapon_blaster",15); EDefs[344].modelIndex=638;  EDefs[344].mass=0.5f;  EDefs[344].angularDrag=0.05f;  EDefs[344].gravity=1.0f; EDefs[344].dynamicFriction=0.6f;  EDefs[344].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[345].path,"weapon_dartgun",15); EDefs[345].modelIndex=640;  EDefs[345].texIndex=876;  EDefs[345].mass=0.3f;  EDefs[345].angularDrag=0.05f; EDefs[345].dynamicFriction=0.6f;  EDefs[345].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[346].path,"weapon_flechette",17); EDefs[346].modelIndex=642;  EDefs[346].mass=0.4f; EDefs[346].angularDrag=0.05f; EDefs[346].dynamicFriction=0.6f;  EDefs[346].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[347].path,"weapon_ionrifle",16); EDefs[347].modelIndex=643;  EDefs[347].mass=0.8f; EDefs[347].angularDrag=0.05f; EDefs[347].dynamicFriction=0.6f;  EDefs[347].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[348].path,"weapon_rapier",14); EDefs[348].modelIndex=653;  EDefs[348].mass=0.3f;  EDefs[348].angularDrag=0.05f; EDefs[348].dynamicFriction=0.6f;  EDefs[348].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[349].path,"weapon_pipe",12); EDefs[349].modelIndex=649;  EDefs[349].texIndex=887;  EDefs[349].mass=0.85f; EDefs[349].angularDrag=0.05f;  EDefs[349].dynamicFriction=0.6f;  EDefs[349].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[350].path,"weapon_magnum",14); EDefs[350].modelIndex=644;  EDefs[350].mass=0.6f; EDefs[350].angularDrag=0.05f; EDefs[350].dynamicFriction=0.6f;  EDefs[350].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[351].path,"weapon_magpulse",16); EDefs[351].modelIndex=645;  EDefs[351].mass=0.65f; EDefs[351].angularDrag=0.05f; EDefs[351].dynamicFriction=0.6f;  EDefs[351].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[352].path,"weapon_pistol",14); EDefs[352].modelIndex=650;  EDefs[352].texIndex=878;  EDefs[352].mass=0.3f; EDefs[352].angularDrag=0.05f; EDefs[352].dynamicFriction=0.6f;  EDefs[352].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[353].path,"weapon_plasma",14); EDefs[353].modelIndex=651;  EDefs[353].mass=1.2f; EDefs[353].angularDrag=0.05f; EDefs[353].dynamicFriction=0.6f;  EDefs[353].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[354].path,"weapon_railgun",15); EDefs[354].modelIndex=652;  EDefs[354].mass=1.0f; EDefs[354].angularDrag=0.05f; EDefs[354].dynamicFriction=0.6f;  EDefs[354].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[355].path,"weapon_riotgun",15); EDefs[355].modelIndex=654;  EDefs[355].mass=0.55f; EDefs[355].angularDrag=0.05f; EDefs[355].dynamicFriction=0.6f;  EDefs[355].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[356].path,"weapon_skorpion",16); EDefs[356].modelIndex=655;  EDefs[356].mass=1.3f;  EDefs[356].angularDrag=0.05f; EDefs[356].dynamicFriction=0.6f;  EDefs[356].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[357].path,"weapon_sparqbeam",17); EDefs[357].modelIndex=656;  EDefs[357].mass=0.3f; EDefs[357].angularDrag=0.05f; EDefs[357].dynamicFriction=0.6f;  EDefs[357].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[358].path,"weapon_stungun",15); EDefs[358].modelIndex=657;  EDefs[358].mass=0.3f; EDefs[358].angularDrag=0.05f; EDefs[358].dynamicFriction=0.6f;  EDefs[358].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[359].path,"item_battery",13); EDefs[359].modelIndex=13;  EDefs[359].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[359].colliderMeshIndex=680;  EDefs[359].mass=0.3f; EDefs[359].angularDrag=0.05f;  EDefs[359].gravity=1.0f;  EDefs[359].kinematic=false;  EDefs[359].dynamicFriction=0.6f;  EDefs[359].staticFriction=0.6f;  EDefs[359].bounciness=0.0f;  EDefs[359].frictionCombine=PHYS_COMBINE_AVG;  EDefs[359].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[360].path,"item_battery_icad",18); EDefs[360].modelIndex=13;  EDefs[360].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[360].colliderMeshIndex=680;  EDefs[360].mass=0.35f;  EDefs[360].angularDrag=0.05f;  EDefs[360].gravity=1.0f;  EDefs[360].kinematic=false;  EDefs[360].dynamicFriction=0.6f;  EDefs[360].staticFriction=0.6f;  EDefs[360].bounciness=0.0f;  EDefs[360].frictionCombine=PHYS_COMBINE_AVG;  EDefs[360].bounceCombine=PHYS_COMBINE_AVG; 
    /*item_logic_probe*/           EDefs[361].modelIndex=217; EDefs[361].texIndex=427;  EDefs[361].mass=0.15f; EDefs[361].angularDrag=0.05f;  EDefs[361].gravity=1.0f;  EDefs[361].kinematic=false;  EDefs[361].dynamicFriction=0.6f;  EDefs[361].staticFriction=0.6f;  EDefs[361].bounciness=0.0f;  EDefs[361].frictionCombine=PHYS_COMBINE_AVG;  EDefs[361].bounceCombine=PHYS_COMBINE_AVG; 
    /*item_healthkit*/             EDefs[362].modelIndex=196; EDefs[362].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[362].colliderMeshIndex=688;  EDefs[362].mass=0.25f;  EDefs[362].angularDrag=0.05f;  EDefs[362].gravity=1.0f;  EDefs[362].kinematic=false;  EDefs[362].dynamicFriction=0.6f;  EDefs[362].staticFriction=0.6f;  EDefs[362].bounciness=0.0f;  EDefs[362].frictionCombine=PHYS_COMBINE_AVG;  EDefs[362].bounceCombine=PHYS_COMBINE_AVG; 
    /*item_plastique*/             EDefs[363].modelIndex=492; EDefs[363].mass=1.4f; EDefs[363].angularDrag=0.05f;  EDefs[363].gravity=1.0f;  EDefs[363].kinematic=false;  EDefs[363].dynamicFriction=0.6f;  EDefs[363].staticFriction=0.6f;  EDefs[363].bounciness=0.0f;
    /*item_chipset_interfacedemod*/EDefs[364].modelIndex=45;  EDefs[364].collider=COLLIDER_TYPE_BOX;  EDefs[364].colliderCenter=(Vector3){0.003744498f,0.0001704991f,0.03192701f};  EDefs[364].colliderSize=(Vector3){0.459303f,0.3412231f,0.06385402f};  EDefs[364].colliderMeshIndex=U16_MAX;  EDefs[364].mass=0.3f;  EDefs[364].angularDrag=0.05f;  EDefs[364].gravity=1.0f;  EDefs[364].kinematic=false;  EDefs[364].dynamicFriction=0.6f;  EDefs[364].staticFriction=0.6f;  EDefs[364].bounciness=0.0f;  EDefs[364].frictionCombine=PHYS_COMBINE_AVG;  EDefs[364].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[365].path,"item_flask",11); EDefs[365].modelIndex=145;  EDefs[365].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[365].colliderMeshIndex=685;  EDefs[365].texIndex=36;  EDefs[365].specIndex=1242;  EDefs[365].mass=0.22f;  EDefs[365].angularDrag=0.05f;  EDefs[365].gravity=1.0f;  EDefs[365].kinematic=false;  EDefs[365].dynamicFriction=0.6f;  EDefs[365].staticFriction=0.6f;  EDefs[365].bounciness=0.0f;  EDefs[365].frictionCombine=PHYS_COMBINE_AVG;  EDefs[365].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[366].path,"item_chipset_bitflag",21); EDefs[366].modelIndex=45;  EDefs[366].collider=COLLIDER_TYPE_BOX;  EDefs[366].colliderCenter=(Vector3){0.003744498f,0.0001704991f,0.03192701f};  EDefs[366].colliderSize=(Vector3){0.459303f,0.3412231f,0.06385402f};  EDefs[366].colliderMeshIndex=U16_MAX;  EDefs[366].mass=0.3f;  EDefs[366].angularDrag=0.05f;  EDefs[366].gravity=1.0f;  EDefs[366].kinematic=false;  EDefs[366].dynamicFriction=0.6f;  EDefs[366].staticFriction=0.6f;  EDefs[366].bounciness=0.0f;  EDefs[366].frictionCombine=PHYS_COMBINE_AVG;  EDefs[366].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[367].path,"item_ammo_rubber",17); EDefs[367].modelIndex=8; EDefs[367].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[367].colliderMeshIndex=676;  EDefs[367].mass=0.25f;  EDefs[367].angularDrag=0.05f;  EDefs[367].gravity=1.0f;  EDefs[367].kinematic=false;  EDefs[367].dynamicFriction=0.6f;  EDefs[367].staticFriction=0.6f;  EDefs[367].bounciness=0.0f;  EDefs[367].frictionCombine=PHYS_COMBINE_AVG;  EDefs[367].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[368].path,"item_isotopex22",16); EDefs[368].modelIndex=209;  EDefs[368].mass=1.2f;  EDefs[368].angularDrag=0.05f;  EDefs[368].gravity=1.0f;  EDefs[368].kinematic=false;  EDefs[368].dynamicFriction=0.6f;  EDefs[368].staticFriction=0.6f;
    /*item_testtube*/ EDefs[369].modelIndex=622; EDefs[369].texIndex=36; EDefs[369].specIndex=1242; EDefs[369].collider=COLLIDER_TYPE_CONVEXMESH; EDefs[369].colliderMeshIndex=612; EDefs[369].mass=0.21f; EDefs[369].angularDrag=0.05f; EDefs[369].gravity=1.0f; EDefs[369].dynamicFriction=0.6f; EDefs[369].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[370].path,"weapon_grenadefrag_live",24); EDefs[370].modelIndex=182;  EDefs[370].mass=1.0f;   EDefs[370].angularDrag=0.05f;  EDefs[370].gravity=1.0f;  EDefs[370].kinematic=false;  EDefs[370].dynamicFriction=0.6f;  EDefs[370].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[371].path,"item_chipset_isolinear",23); EDefs[371].modelIndex=46;  EDefs[371].collider=COLLIDER_TYPE_BOX;  EDefs[371].colliderCenter=(Vector3){-0.0009825006f,-0.0129465f,0.0148115f};  EDefs[371].colliderSize=(Vector3){0.223635f,0.4175691f,0.02912301f};  EDefs[371].colliderMeshIndex=U16_MAX;  EDefs[371].mass=0.26f;  EDefs[371].angularDrag=0.05f;  EDefs[371].gravity=1.0f;  EDefs[371].kinematic=false;  EDefs[371].dynamicFriction=0.6f;  EDefs[371].staticFriction=0.6f;  EDefs[371].bounciness=0.0f;  EDefs[371].frictionCombine=PHYS_COMBINE_AVG;  EDefs[371].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[372].path,"weapon_grenadeconc_live",24); EDefs[372].modelIndex=165;  EDefs[372].mass=1.0f; EDefs[372].angularDrag=0.05f;  EDefs[372].gravity=1.0f;  EDefs[372].kinematic=false;  EDefs[372].dynamicFriction=0.6f;  EDefs[372].staticFriction=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[373].path,"item_ammo_needle",17); EDefs[373].modelIndex=4;  EDefs[373].texIndex=15;  EDefs[373].collider=COLLIDER_TYPE_BOX;  EDefs[373].colliderCenter=(Vector3){-0.0004654949f,0.0004549972f,0.0244365f};  EDefs[373].colliderSize=(Vector3){0.131339f,0.1442801f,0.04838703f};  EDefs[373].colliderMeshIndex=U16_MAX;  EDefs[373].mass=0.15f;  EDefs[373].angularDrag=0.05f;  EDefs[373].gravity=1.0f;  EDefs[373].kinematic=false;  EDefs[373].dynamicFriction=0.6f;  EDefs[373].staticFriction=0.6f;  EDefs[373].bounciness=0.0f;  EDefs[373].frictionCombine=PHYS_COMBINE_AVG;  EDefs[373].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[374].path,"item_ammo_tranq",16); EDefs[374].modelIndex=4;  EDefs[374].texIndex=27;  EDefs[374].collider=COLLIDER_TYPE_BOX;  EDefs[374].colliderCenter=(Vector3){-0.0004654949f,0.0004549972f,0.0244365f};  EDefs[374].colliderSize=(Vector3){0.131339f,0.1442801f,0.04838703f};  EDefs[374].colliderMeshIndex=U16_MAX;  EDefs[374].mass=0.15f; EDefs[374].angularDrag=0.05f;  EDefs[374].gravity=1.0f;  EDefs[374].kinematic=false;  EDefs[374].dynamicFriction=0.6f;  EDefs[374].staticFriction=0.6f;  EDefs[374].bounciness=0.0f;  EDefs[374].frictionCombine=PHYS_COMBINE_AVG;  EDefs[374].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[375].path,"item_ammo_standard",19); EDefs[375].modelIndex=5;  EDefs[375].collider=COLLIDER_TYPE_BOX;  EDefs[375].colliderCenter=(Vector3){0.0001984993f,0.0f,0.02172501f};  EDefs[375].colliderSize=(Vector3){0.1209471f,0.2176701f,0.04345007f};  EDefs[375].colliderMeshIndex=U16_MAX;  EDefs[375].mass=0.2f; EDefs[375].angularDrag=0.05f;  EDefs[375].gravity=1.0f;  EDefs[375].kinematic=false;  EDefs[375].dynamicFriction=0.6f;  EDefs[375].staticFriction=0.6f;  EDefs[375].bounciness=0.0f;  EDefs[375].frictionCombine=PHYS_COMBINE_AVG;  EDefs[375].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[376].path,"item_ammo_teflon",17); EDefs[376].modelIndex=5;  EDefs[376].collider=COLLIDER_TYPE_BOX;  EDefs[376].colliderCenter=(Vector3){0.0001984993f,0.0f,0.02172501f};  EDefs[376].colliderSize=(Vector3){0.1209471f,0.2176701f,0.04345007f};  EDefs[376].colliderMeshIndex=U16_MAX;  EDefs[376].mass=0.2f; EDefs[376].angularDrag=0.05f;  EDefs[376].gravity=1.0f;  EDefs[376].kinematic=false;  EDefs[376].dynamicFriction=0.6f;  EDefs[376].staticFriction=0.6f;  EDefs[376].bounciness=0.0f;  EDefs[376].frictionCombine=PHYS_COMBINE_AVG;  EDefs[376].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[377].path,"item_ammo_hollow",17); EDefs[377].modelIndex=5;  EDefs[377].collider=COLLIDER_TYPE_BOX;  EDefs[377].colliderCenter=(Vector3){0.0002185023f,0.0f,0.02122951f};  EDefs[377].colliderSize=(Vector3){0.1423431f,0.2127061f,0.04245907f};  EDefs[377].colliderMeshIndex=U16_MAX;  EDefs[377].mass=0.2f; EDefs[377].angularDrag=0.05f;  EDefs[377].gravity=1.0f;  EDefs[377].kinematic=false;  EDefs[377].dynamicFriction=0.6f;  EDefs[377].staticFriction=0.6f;  EDefs[377].bounciness=0.0f;  EDefs[377].frictionCombine=PHYS_COMBINE_AVG;  EDefs[377].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[378].path,"item_ammo_slug",15); EDefs[378].modelIndex=3;  EDefs[378].collider=COLLIDER_TYPE_BOX;  EDefs[378].colliderCenter=(Vector3){0.0002185023f,0.0f,0.02122951f};  EDefs[378].colliderSize=(Vector3){0.1423431f,0.2127061f,0.04245907f};  EDefs[378].colliderMeshIndex=U16_MAX;  EDefs[378].glowIndex=22;  EDefs[378].mass=0.2f; EDefs[378].angularDrag=0.05f;  EDefs[378].gravity=1.0f;  EDefs[378].kinematic=false;  EDefs[378].dynamicFriction=0.6f;  EDefs[378].staticFriction=0.6f;  EDefs[378].bounciness=0.0f;  EDefs[378].frictionCombine=PHYS_COMBINE_AVG;  EDefs[378].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[379].path,"item_ammo_magnesium",20); EDefs[379].modelIndex=3;  EDefs[379].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[379].colliderMeshIndex=673;  EDefs[379].mass=0.35f; EDefs[379].angularDrag=0.05f;  EDefs[379].gravity=1.0f;  EDefs[379].kinematic=false;  EDefs[379].dynamicFriction=0.6f;  EDefs[379].staticFriction=0.6f;  EDefs[379].bounciness=0.0f;  EDefs[379].frictionCombine=PHYS_COMBINE_AVG;  EDefs[379].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[380].path,"item_ammo_penetrator",21); EDefs[380].modelIndex=3;  EDefs[380].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[380].colliderMeshIndex=673;  EDefs[380].mass=0.35f; EDefs[380].angularDrag=0.05f;  EDefs[380].gravity=1.0f;  EDefs[380].kinematic=false;  EDefs[380].dynamicFriction=0.6f;  EDefs[380].staticFriction=0.6f;  EDefs[380].bounciness=0.0f;  EDefs[380].frictionCombine=PHYS_COMBINE_AVG;  EDefs[380].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[381].path,"item_ammo_hornet",17); EDefs[381].modelIndex=1;  EDefs[381].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[381].colliderMeshIndex=673;  EDefs[381].mass=0.35f; EDefs[381].angularDrag=0.05f;  EDefs[381].gravity=1.0f;  EDefs[381].kinematic=false;  EDefs[381].dynamicFriction=0.6f;  EDefs[381].staticFriction=0.6f;  EDefs[381].bounciness=0.0f;  EDefs[381].frictionCombine=PHYS_COMBINE_AVG;  EDefs[381].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[382].path,"item_ammo_splinter",19); EDefs[382].modelIndex=630;  EDefs[382].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[382].colliderMeshIndex=673;  EDefs[382].mass=0.35f; EDefs[382].angularDrag=0.05f;  EDefs[382].gravity=1.0f;  EDefs[382].kinematic=false;  EDefs[382].dynamicFriction=0.6f;  EDefs[382].staticFriction=0.6f;  EDefs[382].bounciness=0.0f;  EDefs[382].frictionCombine=PHYS_COMBINE_AVG;  EDefs[382].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[383].path,"item_ammo_rail",15); EDefs[383].modelIndex=6;  EDefs[383].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[383].colliderMeshIndex=675;  EDefs[383].mass=0.4f; EDefs[383].angularDrag=0.05f;  EDefs[383].gravity=1.0f;  EDefs[383].kinematic=false;  EDefs[383].dynamicFriction=0.6f;  EDefs[383].staticFriction=0.6f;  EDefs[383].bounciness=0.0f;  EDefs[383].frictionCombine=PHYS_COMBINE_AVG;  EDefs[383].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[384].path,"item_ammo_slag",15); EDefs[384].modelIndex=9;  EDefs[384].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[384].colliderMeshIndex=673;  EDefs[384].mass=0.35f; EDefs[384].angularDrag=0.05f;  EDefs[384].gravity=1.0f;  EDefs[384].kinematic=false;  EDefs[384].dynamicFriction=0.6f;  EDefs[384].staticFriction=0.6f;  EDefs[384].bounciness=0.0f;  EDefs[384].frictionCombine=PHYS_COMBINE_AVG;  EDefs[384].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[385].path,"item_ammo_slaglarge",20); EDefs[385].modelIndex=10;  EDefs[385].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[385].colliderMeshIndex=677;  EDefs[385].mass=0.40f; EDefs[385].angularDrag=0.05f;  EDefs[385].gravity=1.0f;  EDefs[385].kinematic=false;  EDefs[385].dynamicFriction=0.6f;  EDefs[385].staticFriction=0.6f;  EDefs[385].bounciness=0.0f;  EDefs[385].frictionCombine=PHYS_COMBINE_AVG;  EDefs[385].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[386].path,"item_ammo_magcart",18); EDefs[386].modelIndex=2;  EDefs[386].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[386].colliderMeshIndex=674;  EDefs[386].mass=0.35f; EDefs[386].angularDrag=0.05f;  EDefs[386].gravity=1.0f;  EDefs[386].kinematic=false;  EDefs[386].dynamicFriction=0.6f;  EDefs[386].staticFriction=0.6f;  EDefs[386].bounciness=0.0f;  EDefs[386].frictionCombine=PHYS_COMBINE_AVG;  EDefs[386].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[387].path,"weapon_grenadeemp_live",23); EDefs[387].modelIndex=168;  EDefs[387].mass=1.0f;  EDefs[387].angularDrag=0.05f;  EDefs[387].gravity=1.0f;  EDefs[387].kinematic=false;  EDefs[387].dynamicFriction=0.6f;  EDefs[387].staticFriction=0.6f;  EDefs[387].bounciness=0.0f;  EDefs[387].frictionCombine=PHYS_COMBINE_AVG;  EDefs[387].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[388].path,"item_access_card_std",21); EDefs[388].modelIndex=0;  EDefs[388].texIndex=79;  EDefs[388].glowIndex=867;  EDefs[388].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[388].colliderMeshIndex=672;  EDefs[388].mass=0.2f; EDefs[388].angularDrag=0.05f;  EDefs[388].kinematic=false;  EDefs[388].dynamicFriction=0.6f;  EDefs[388].staticFriction=0.6f;  EDefs[388].bounciness=0.0f;  EDefs[388].frictionCombine=PHYS_COMBINE_AVG;  EDefs[388].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[389].path,"weapon_grenadeearth_live",25); EDefs[389].modelIndex=181;  EDefs[389].mass=1.0f;  EDefs[389].angularDrag=0.05f;  EDefs[389].gravity=1.0f;  EDefs[389].kinematic=false;  EDefs[389].dynamicFriction=0.6f;  EDefs[389].staticFriction=0.6f; EDefs[389].frictionCombine=PHYS_COMBINE_AVG;  EDefs[389].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[390].path,"item_access_card_group1",24); EDefs[390].modelIndex=0;  EDefs[390].texIndex=7;  EDefs[390].glowIndex=159;  EDefs[390].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[390].colliderMeshIndex=672;  EDefs[390].mass=0.2f; EDefs[390].angularDrag=0.05f; EDefs[390].kinematic=false;  EDefs[390].dynamicFriction=0.6f;  EDefs[390].staticFriction=0.6f;  EDefs[390].bounciness=0.0f;  EDefs[390].frictionCombine=PHYS_COMBINE_AVG;  EDefs[390].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[391].path,"item_access_card_science",25); EDefs[391].modelIndex=0;  EDefs[391].texIndex=2;  EDefs[391].glowIndex=343;  EDefs[391].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[391].colliderMeshIndex=672;  EDefs[391].mass=0.2f; EDefs[391].angularDrag=0.05f; EDefs[391].kinematic=false;  EDefs[391].dynamicFriction=0.6f;  EDefs[391].staticFriction=0.6f;  EDefs[391].bounciness=0.0f;  EDefs[391].frictionCombine=PHYS_COMBINE_AVG;  EDefs[391].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[392].path,"item_access_card_eng",21); EDefs[392].modelIndex=0;  EDefs[392].texIndex=3;  EDefs[392].glowIndex=81;  EDefs[392].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[392].colliderMeshIndex=672;  EDefs[392].mass=0.2f; EDefs[392].angularDrag=0.05f; EDefs[392].kinematic=false;  EDefs[392].dynamicFriction=0.6f;  EDefs[392].staticFriction=0.6f;  EDefs[392].bounciness=0.0f;  EDefs[392].frictionCombine=PHYS_COMBINE_AVG;  EDefs[392].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[393].path,"item_access_card_groupB",24); EDefs[393].modelIndex=0;  EDefs[393].texIndex=7;  EDefs[393].glowIndex=159;  EDefs[393].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[393].colliderMeshIndex=672;  EDefs[393].mass=0.2f; EDefs[393].angularDrag=0.05f;  EDefs[393].kinematic=false;  EDefs[393].dynamicFriction=0.6f;  EDefs[393].staticFriction=0.6f;  EDefs[393].bounciness=0.0f;  EDefs[393].frictionCombine=PHYS_COMBINE_AVG;  EDefs[393].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[394].path,"item_access_card_security",26); EDefs[394].modelIndex=0;  EDefs[394].texIndex=10;  EDefs[394].glowIndex=344;  EDefs[394].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[394].colliderMeshIndex=672;  EDefs[394].mass=0.2f; EDefs[394].angularDrag=0.05f; EDefs[394].kinematic=false;  EDefs[394].dynamicFriction=0.6f;  EDefs[394].staticFriction=0.6f;  EDefs[394].bounciness=0.0f;  EDefs[394].frictionCombine=PHYS_COMBINE_AVG;  EDefs[394].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[395].path,"item_access_card_per5diego",27); EDefs[395].modelIndex=0;  EDefs[395].texIndex=8;  EDefs[395].glowIndex=341;  EDefs[395].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[395].colliderMeshIndex=672;  EDefs[395].mass=0.2f; EDefs[395].angularDrag=0.05f; EDefs[395].kinematic=false;  EDefs[395].dynamicFriction=0.6f;  EDefs[395].staticFriction=0.6f;  EDefs[395].bounciness=0.0f;  EDefs[395].frictionCombine=PHYS_COMBINE_AVG;  EDefs[395].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[396].path,"item_access_card_medi",22); EDefs[396].modelIndex=0;  EDefs[396].texIndex=1;  EDefs[396].glowIndex=161;  EDefs[396].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[396].colliderMeshIndex=672;  EDefs[396].mass=0.2f; EDefs[396].angularDrag=0.05f; EDefs[396].kinematic=false;  EDefs[396].dynamicFriction=0.6f;  EDefs[396].staticFriction=0.6f;  EDefs[396].bounciness=0.0f;  EDefs[396].frictionCombine=PHYS_COMBINE_AVG;  EDefs[396].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[397].path,"item_access_card_group3",24); EDefs[397].modelIndex=0;  EDefs[397].texIndex=7;  EDefs[397].glowIndex=159;  EDefs[397].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[397].colliderMeshIndex=672;  EDefs[397].mass=0.2f; EDefs[397].angularDrag=0.05f; EDefs[397].dynamicFriction=0.6f;  EDefs[397].staticFriction=0.6f; EDefs[397].frictionCombine=PHYS_COMBINE_AVG;  EDefs[397].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[398].path,"item_access_card_purple",24); EDefs[398].modelIndex=0;  EDefs[398].texIndex=5;  EDefs[398].glowIndex=342;  EDefs[398].collider=COLLIDER_TYPE_CONVEXMESH;  EDefs[398].colliderMeshIndex=672;  EDefs[398].mass=0.2f; EDefs[398].angularDrag=0.05f; EDefs[398].dynamicFriction=0.6f;  EDefs[398].staticFriction=0.6f; EDefs[398].frictionCombine=PHYS_COMBINE_AVG;  EDefs[398].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[399].path,"item_head_male",15);   EDefs[399].modelIndex=194; EDefs[399].collider=COLLIDER_TYPE_CONVEXMESH; EDefs[399].colliderMeshIndex=687; EDefs[399].mass=1.29f; EDefs[399].angularDrag=0.05f; EDefs[399].dynamicFriction=0.6f; EDefs[399].staticFriction=0.6f; EDefs[399].frictionCombine=PHYS_COMBINE_AVG;  EDefs[399].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[400].path,"item_head_female",17); EDefs[400].modelIndex=193; EDefs[400].collider=COLLIDER_TYPE_CONVEXMESH; EDefs[400].colliderMeshIndex=686; EDefs[400].mass=1.30f; EDefs[400].angularDrag=0.05f; EDefs[400].dynamicFriction=0.6f; EDefs[400].staticFriction=0.6f; EDefs[400].frictionCombine=PHYS_COMBINE_AVG;  EDefs[400].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[401].path,"item_severedhead",17); EDefs[401].modelIndex=590; EDefs[401].mass=1.28f; EDefs[401].angularDrag=0.05f;  EDefs[401].gravity=1.0f; EDefs[401].dynamicFriction=0.6f;  EDefs[401].staticFriction=0.6f;  EDefs[401].frictionCombine=PHYS_COMBINE_AVG;  EDefs[401].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[402].path,"weapon_grenademine_live",24);  EDefs[402].modelIndex=184; EDefs[402].mass=1.0f; EDefs[402].angularDrag=0.05f; EDefs[402].gravity=1.0f; EDefs[402].dynamicFriction=0.6f;  EDefs[402].staticFriction=0.6f; EDefs[402].frictionCombine=PHYS_COMBINE_AVG; EDefs[402].bounceCombine=PHYS_COMBINE_AVG; 
    /*403 weapon_grenadenitro_live*/   EDefs[403].modelIndex=185; EDefs[403].mass=1.0f; EDefs[403].angularDrag=0.05f; EDefs[403].gravity=1.0f; EDefs[403].dynamicFriction=0.6f;  EDefs[403].staticFriction=0.6f; EDefs[403].frictionCombine=PHYS_COMBINE_AVG; EDefs[403].bounceCombine=PHYS_COMBINE_AVG; 
    /*404 weapon_grenadegas_live*/     EDefs[404].modelIndex=183; EDefs[404].mass=1.0f; EDefs[404].angularDrag=0.05f; EDefs[404].dynamicFriction=0.6f; EDefs[404].staticFriction=0.6f; EDefs[404].frictionCombine=PHYS_COMBINE_AVG; EDefs[404].bounceCombine=PHYS_COMBINE_AVG; 
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
    /*417 item_access_card_perdarcy*/  EDefs[417].modelIndex=0; EDefs[417].texIndex=8; EDefs[417].glowIndex=341; EDefs[417].collider=COLLIDER_TYPE_CONVEXMESH; EDefs[417].colliderMeshIndex=672; EDefs[417].mass=0.2f; EDefs[417].angularDrag=0.05f; EDefs[417].kinematic=false; EDefs[417].dynamicFriction=0.6f; EDefs[417].staticFriction=0.6f; EDefs[417].frictionCombine=PHYS_COMBINE_AVG; EDefs[417].bounceCombine=PHYS_COMBINE_AVG;

    // NPC Definitions
    for (int i=419;i<=447;++i) { EDefs[i].bounceCombine=PHYS_COMBINE_MAX; EDefs[i].collider=COLLIDER_TYPE_CAPSULE; EDefs[i].colliderSize.z=COLLIDER_CAPSULE_DIRECTION_Y_F; EDefs[i].frictionCombine=PHYS_COMBINE_MUL; EDefs[i].staticFriction=1.0f; EDefs[i].dynamicFriction=0.15f; EDefs[i].kinematic=true; EDefs[i].mass=1.0f; EDefs[i].angularDrag=2.2f; }
    /*419 npc_autobomb*/            EDefs[419].modelIndex=299; EDefs[419].texIndex=542; EDefs[419].colliderCenter.y=0.42f; EDefs[419].colliderCenter.z=0.01848752f;                   EDefs[419].colliderSize=(Vector3){0.42f,1.48f,COLLIDER_CAPSULE_DIRECTION_Z_F};          EDefs[419].angularDrag=1.0f; EDefs[419].glowIndex=541;
    /*420 npc_cyborg_assassin*/     EDefs[420].modelIndex=306; EDefs[420].texIndex=545; EDefs[420].numclips= 8; EDefs[420].animationNum=24;                                           EDefs[419].colliderSize.x=0.48f; EDefs[419].colliderSize.y=2.0f;  EDefs[420].mass=1.5f; EDefs[420].angularDrag=1.5f; EDefs[420].glowIndex=544;
    /*421 npc_avian_mutant*/        EDefs[421].modelIndex=328; EDefs[421].texIndex=568; EDefs[421].numclips= 5; EDefs[421].animationNum=35; EDefs[421].colliderCenter.y= 0.0200f;     EDefs[421].colliderSize.x=0.40f; EDefs[421].colliderSize.y=1.60f; EDefs[421].mass=2.0f; EDefs[421].angularDrag=1.0f;
    /*422 npc_exec_bot*/            EDefs[422].modelIndex=316; EDefs[422].texIndex=555; EDefs[422].numclips= 5; EDefs[422].animationNum=29; EDefs[422].colliderCenter.y= 0.0125f;     EDefs[422].colliderSize.x=0.48f; EDefs[422].colliderSize.y=2.025f;EDefs[422].mass=2.2f; EDefs[422].angularDrag=1.5f;
    /*423 npc_cyborg_drone*/        EDefs[423].modelIndex=312; EDefs[423].texIndex=547; EDefs[423].numclips= 7; EDefs[423].animationNum=3;                                            EDefs[423].colliderSize.x=0.36f; EDefs[423].colliderSize.y=2.00f; EDefs[423].mass=1.5f; EDefs[423].angularDrag=2.0f;
    /*424 npc_cortex_reaver*/       EDefs[424].modelIndex=300; EDefs[424].texIndex=543; EDefs[424].numclips=6;  EDefs[424].animationNum=23; EDefs[424].colliderCenter.y=-0.02263292f; EDefs[424].colliderSize.x=0.451f;                                 EDefs[424].mass=5.0f; EDefs[424].angularDrag=3.0f; EDefs[424].collider=COLLIDER_TYPE_SPHERE;
    /*425 npc_cyborg_warrior*/      EDefs[425].modelIndex=315; EDefs[425].texIndex=554; EDefs[425].numclips=7;  EDefs[425].animationNum=28;                                           EDefs[425].colliderSize.x=0.48f; EDefs[425].colliderSize.y=2.00f; EDefs[425].mass=1.5f; EDefs[425].angularDrag=2.0f;
    /*426 npc_cyborg_enforcer*/     EDefs[426].modelIndex=314; EDefs[426].texIndex=550; EDefs[426].numclips=8;  EDefs[426].animationNum=27; EDefs[426].colliderCenter.y=0.05f;        EDefs[426].colliderSize.x=0.40f; EDefs[426].colliderSize.y=2.08f; EDefs[426].mass=1.5f;
    /*427 npc_cyborg_elite*/        EDefs[427].modelIndex=313; EDefs[427].texIndex=548; EDefs[427].numclips=10; EDefs[427].animationNum=26; EDefs[427].colliderCenter.y=0.10f;        EDefs[427].colliderSize.x=0.44f; EDefs[427].colliderSize.y=2.20f; EDefs[427].mass=3.5f;
    /*428 npc_cyborg_diego*/        EDefs[428].modelIndex=309; EDefs[428].texIndex=546; EDefs[428].numclips=6;  EDefs[428].animationNum=25;                                           EDefs[428].colliderSize.x=0.48f; EDefs[428].colliderSize.y=2.12f; EDefs[428].mass=2.0f;
    /*429 npc_sec1_bot*/            EDefs[429].modelIndex=333; EDefs[429].texIndex=573; EDefs[429].numclips=2;  EDefs[429].animationNum=38; EDefs[429].colliderCenter.y=0.05f;        EDefs[429].colliderSize.x=0.64f;                                  EDefs[429].mass=1.5f; EDefs[429].angularDrag=0.8f; EDefs[429].collider=COLLIDER_TYPE_SPHERE;
    /*430 npc_sec2_bot*/            EDefs[430].modelIndex=335; EDefs[430].texIndex=574; EDefs[430].numclips=6;  EDefs[430].animationNum=39; EDefs[430].colliderCenter.y=0.2f;         EDefs[430].colliderSize.x=0.80f; EDefs[430].colliderSize.y=2.40f; EDefs[430].mass=4.51f;
    /*431 npc_maint_bot*/           EDefs[431].modelIndex=325; EDefs[431].texIndex=567; EDefs[431].numclips=4;  EDefs[431].animationNum=34; EDefs[431].colliderCenter.y=-0.3f;        EDefs[431].colliderSize.x=0.48f;                                  EDefs[431].mass=1.5f; EDefs[431].angularDrag=1.5f; EDefs[431].collider=COLLIDER_TYPE_SPHERE;
    /*432 npc_mutant_cyborg*/       EDefs[432].modelIndex=329; EDefs[432].texIndex=569; EDefs[432].numclips=7;  EDefs[432].animationNum=51; EDefs[432].colliderCenter.y=0.12f;        EDefs[432].colliderSize.x=0.65f; EDefs[432].colliderSize.y=2.30f; EDefs[432].mass=3.0f;
    /*433 npc_hopper*/              EDefs[433].modelIndex=322; EDefs[433].texIndex=562; EDefs[433].numclips=8;  EDefs[433].animationNum=32; EDefs[433].colliderCenter.z=1.0f;         EDefs[433].colliderSize.x=0.64f; EDefs[433].colliderSize.y=2.00f; EDefs[433].angularDrag=1000.0f; EDefs[433].dynamicFriction=0.005f; EDefs[433].staticFriction=EDefs[433].bounciness=0.1f; EDefs[433].frictionCombine=PHYS_COMBINE_AVG; EDefs[433].bounceCombine=PHYS_COMBINE_AVG;
    /*434 npc_humanoid_mutant*/     EDefs[434].modelIndex=323; EDefs[434].texIndex=563; EDefs[434].numclips=6;  EDefs[434].animationNum=2;                                            EDefs[434].colliderSize.x=0.38f; EDefs[434].colliderSize.y=2.00f; EDefs[434].mass=1.4f; EDefs[434].angularDrag=2.0f;
    /*435 npc_invisomut*/           EDefs[435].modelIndex=324; EDefs[435].texIndex=565; EDefs[435].numclips=5;  EDefs[435].animationNum=33; EDefs[435].colliderCenter.y=-0.28938290f; EDefs[435].colliderSize=(Vector3){1.5f,1.078766f,0.8f};           EDefs[435].mass=1.3f; EDefs[435].angularDrag=0.8f; EDefs[435].collider=COLLIDER_TYPE_BOX;
    /*436 npc_virus_mutant*/        EDefs[436].modelIndex=330; EDefs[436].texIndex=576; EDefs[436].numclips=6;  EDefs[436].animationNum=41; EDefs[436].colliderCenter.y=-0.05f;       EDefs[436].colliderSize.x=0.40f; EDefs[436].colliderSize.y=1.90f; EDefs[436].mass=1.4f; EDefs[436].angularDrag=2.0f;
    /*437 npc_servbot*/             EDefs[437].modelIndex=5153;EDefs[437].texIndex=575; EDefs[437].numclips=5;  EDefs[437].animationNum=40; EDefs[437].colliderMeshIndex=5153; EDefs[437].mass=2.50f; EDefs[437].angularDrag=1.0f; EDefs[437].collider=COLLIDER_TYPE_MESH;
    /*438 npc_flier_bot*/           EDefs[438].modelIndex=318; EDefs[438].texIndex=558; EDefs[438].numclips=5;  EDefs[438].animationNum=30; EDefs[438].mass=1.75f; EDefs[438].angularDrag=0.8f;
    /*439 npc_zerog_mutant*/        EDefs[439].modelIndex=395; EDefs[439].texIndex=1170;EDefs[439].numclips=3;  EDefs[439].animationNum=42; EDefs[439].mass=1.30f; EDefs[439].angularDrag=1.0f;
    /*440 npc_gorilla_tiger_mutant*/EDefs[440].modelIndex=320; EDefs[440].texIndex=560; EDefs[440].numclips=7;  EDefs[440].animationNum=31; EDefs[440].mass=2.00f;
    /*441 npc_repairbot*/           EDefs[441].modelIndex=331; EDefs[441].texIndex=572; EDefs[441].numclips=4;  EDefs[441].animationNum=37; EDefs[441].mass=1.50f; EDefs[441].angularDrag=2.0f;
    /*442 npc_plant_mutant*/        EDefs[442].modelIndex=330; EDefs[442].texIndex=570; EDefs[442].numclips=6;  EDefs[442].animationNum=36; EDefs[442].mass=0.80f; EDefs[442].angularDrag=1.5f;
    /*443 npc_cyberdog*/            EDefs[443].modelIndex=302; EDefs[443].mass=1.50f; EDefs[443].angularDrag=3.0f;
    /*444 npc_cyberguard*/          EDefs[444].modelIndex=303; EDefs[444].mass=2.00f; EDefs[444].angularDrag=3.0f;
    /*445 npc_cyberram*/            EDefs[445].modelIndex=304; EDefs[445].mass=2.00f; EDefs[445].angularDrag=3.0f;
    /*446 npc_cyber_reaver*/        EDefs[446].modelIndex=305; EDefs[446].mass=2.20f; EDefs[446].angularDrag=3.0f;
    /*447 npc_cybershodan*/         EDefs[447].mass=4.51f; EDefs[447].angularDrag=3.0f; EDefs[447].dynamicFriction=0.6f; EDefs[447].staticFriction=0.6f; EDefs[447].frictionCombine=PHYS_COMBINE_AVG; EDefs[447].bounceCombine=PHYS_COMBINE_AVG;

    // Cyber Item Definitions
    for (int i=448;i<=457;++i) { EDefs[i].collider=COLLIDER_TYPE_SPHERE; EDefs[i].colliderSize=(Vector3){1.5f,1.5f,1.5f}; }
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
    CopyMemoryFromBtoAForNBytes(EDefs[458].path,"prop_phys_barrel_chemical",26); EDefs[458].modelIndex=12;  EDefs[458].texIndex=30;  EDefs[458].mass=1.5f; EDefs[458].angularDrag=0.05f;  EDefs[458].gravity=1.0f;  EDefs[458].kinematic=false;  EDefs[458].dynamicFriction=0.6f;  EDefs[458].staticFriction=0.6f;  EDefs[458].bounciness=0.0f;  EDefs[458].frictionCombine=PHYS_COMBINE_AVG;  EDefs[458].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[459].path,"prop_phys_barrel_radiation",27); EDefs[459].modelIndex=12;  EDefs[459].texIndex=31;  EDefs[459].mass=1.5f;  EDefs[459].angularDrag=0.05f;  EDefs[459].gravity=1.0f;  EDefs[459].kinematic=false;  EDefs[459].dynamicFriction=0.6f;  EDefs[459].staticFriction=0.6f;  EDefs[459].bounciness=0.0f;  EDefs[459].frictionCombine=PHYS_COMBINE_AVG;  EDefs[459].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[460].path,"prop_phys_barrel_toxic",23); EDefs[460].modelIndex=12;  EDefs[460].texIndex=33;  EDefs[460].mass=1.5f; EDefs[460].angularDrag=0.05f;  EDefs[460].gravity=1.0f;  EDefs[460].kinematic=false;  EDefs[460].dynamicFriction=0.6f;  EDefs[460].staticFriction=0.6f;  EDefs[460].bounciness=0.0f;  EDefs[460].frictionCombine=PHYS_COMBINE_AVG;  EDefs[460].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[461].path,"prop_phys_cart",15); EDefs[461].modelIndex=40;  EDefs[461].texIndex=416;  EDefs[461].mass=2.5f; EDefs[461].angularDrag=0.05f;  EDefs[461].gravity=1.0f;  EDefs[461].kinematic=false;  EDefs[461].dynamicFriction=0.6f;  EDefs[461].staticFriction=0.6f;  EDefs[461].bounciness=0.0f;  EDefs[461].frictionCombine=PHYS_COMBINE_AVG;  EDefs[461].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[462].path,"prop_phys_pot",14); EDefs[462].modelIndex=494;  EDefs[462].mass=0.3f; EDefs[462].angularDrag=0.05f;  EDefs[462].gravity=1.0f;  EDefs[462].kinematic=false;  EDefs[462].dynamicFriction=0.6f;  EDefs[462].staticFriction=0.6f;  EDefs[462].bounciness=0.0f;  EDefs[462].frictionCombine=PHYS_COMBINE_AVG;  EDefs[462].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[463].path,"prop_phys_toolcart",19); EDefs[463].modelIndex=624;  EDefs[463].texIndex=865;  EDefs[463].normIndex=864;  EDefs[463].specIndex=866;  EDefs[463].mass=20.0f; EDefs[463].angularDrag=0.2f;  EDefs[463].gravity=1.0f;  EDefs[463].kinematic=false;  EDefs[463].dynamicFriction=0.6f;  EDefs[463].staticFriction=0.6f;  EDefs[463].bounciness=0.0f;  EDefs[463].frictionCombine=PHYS_COMBINE_AVG;  EDefs[463].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[464].path,"se_briefcase",13); EDefs[464].modelIndex=34;  EDefs[464].texIndex=66;  EDefs[464].glowIndex=65; 
    CopyMemoryFromBtoAForNBytes(EDefs[465].path,"se_corpse_blueshirt",20); EDefs[465].texIndex=126;  EDefs[465].specIndex=127;  EDefs[465].modelIndex=51; 
    CopyMemoryFromBtoAForNBytes(EDefs[466].path,"se_corpse_brownshirt",21); EDefs[466].texIndex=128;  EDefs[466].specIndex=129;  EDefs[466].modelIndex=52; 
    CopyMemoryFromBtoAForNBytes(EDefs[467].path,"se_corpse_eaten",16); EDefs[467].texIndex=130;  EDefs[467].specIndex=131;  EDefs[467].modelIndex=53; 
    CopyMemoryFromBtoAForNBytes(EDefs[468].path,"se_corpse_labcoat",18); EDefs[468].texIndex=132;  EDefs[468].specIndex=133;  EDefs[468].modelIndex=55; 
    CopyMemoryFromBtoAForNBytes(EDefs[469].path,"se_corpse_security",19); EDefs[469].texIndex=136;  EDefs[469].specIndex=137;  EDefs[469].modelIndex=56; 
    CopyMemoryFromBtoAForNBytes(EDefs[470].path,"se_corpse_tan",14); EDefs[470].texIndex=138;  EDefs[470].modelIndex=57; 
    CopyMemoryFromBtoAForNBytes(EDefs[471].path,"se_corpse_torso",16); EDefs[471].texIndex=126;  EDefs[471].specIndex=127;  EDefs[471].modelIndex=58; 
    CopyMemoryFromBtoAForNBytes(EDefs[472].path,"se_crate1",10); EDefs[472].texIndex=145;  EDefs[472].modelIndex=60;  EDefs[472].collider=COLLIDER_TYPE_BOX;  EDefs[472].colliderCenter=(Vector3){0.0f,0.0f,0.3420931f};  EDefs[472].colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f};  EDefs[472].colliderMeshIndex=U16_MAX;  EDefs[472].mass=0.75f; EDefs[472].angularDrag=0.05f;  EDefs[472].gravity=1.0f;  EDefs[472].kinematic=false;  EDefs[472].dynamicFriction=0.6f;  EDefs[472].staticFriction=0.6f;  EDefs[472].bounciness=0.0f;  EDefs[472].frictionCombine=PHYS_COMBINE_AVG;  EDefs[472].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[473].path,"se_crate2",10); EDefs[473].texIndex=143;  EDefs[473].modelIndex=60;  EDefs[473].collider=COLLIDER_TYPE_BOX;  EDefs[473].colliderCenter=(Vector3){0.0f,0.0f,0.3420931f};  EDefs[473].colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f};  EDefs[473].colliderMeshIndex=U16_MAX;  EDefs[473].mass=0.75f; EDefs[473].angularDrag=0.05f;  EDefs[473].gravity=1.0f;  EDefs[473].kinematic=false;  EDefs[473].dynamicFriction=0.6f;  EDefs[473].staticFriction=0.6f;  EDefs[473].bounciness=0.0f;  EDefs[473].frictionCombine=PHYS_COMBINE_AVG;  EDefs[473].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[474].path,"se_crate3",10); EDefs[474].texIndex=144;  EDefs[474].modelIndex=60;  EDefs[474].collider=COLLIDER_TYPE_BOX;  EDefs[474].colliderCenter=(Vector3){0.0f,0.0f,0.3420931f};  EDefs[474].colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f};  EDefs[474].colliderMeshIndex=U16_MAX;  EDefs[474].mass=0.75f; EDefs[474].angularDrag=0.05f;  EDefs[474].gravity=1.0f;  EDefs[474].kinematic=false;  EDefs[474].dynamicFriction=0.6f;  EDefs[474].staticFriction=0.6f;  EDefs[474].bounciness=0.0f;  EDefs[474].frictionCombine=PHYS_COMBINE_AVG;  EDefs[474].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[475].path,"se_crate4",10); EDefs[475].texIndex=146;  EDefs[475].modelIndex=60;  EDefs[475].collider=COLLIDER_TYPE_BOX;  EDefs[475].colliderCenter=(Vector3){0.0f,0.0f,0.3420931f};  EDefs[475].colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f};  EDefs[475].colliderMeshIndex=U16_MAX;  EDefs[475].mass=2.25f; EDefs[475].angularDrag=0.05f;  EDefs[475].gravity=1.0f;  EDefs[475].kinematic=false;  EDefs[475].dynamicFriction=0.6f;  EDefs[475].staticFriction=0.6f;  EDefs[475].bounciness=0.0f;  EDefs[475].frictionCombine=PHYS_COMBINE_AVG;  EDefs[475].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[476].path,"se_crate5",10); EDefs[476].modelIndex=60;  EDefs[476].collider=COLLIDER_TYPE_BOX;  EDefs[476].colliderCenter=(Vector3){0.0f,0.0f,0.3420931f};  EDefs[476].colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f};  EDefs[476].colliderMeshIndex=U16_MAX;  EDefs[476].mass=2.25f; EDefs[476].angularDrag=0.05f;  EDefs[476].gravity=1.0f;  EDefs[476].kinematic=false;  EDefs[476].dynamicFriction=0.6f;  EDefs[476].staticFriction=0.6f;  EDefs[476].bounciness=0.0f;  EDefs[476].frictionCombine=PHYS_COMBINE_AVG;  EDefs[476].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[477].path,"sec_camera",11); EDefs[477].modelIndex=589;  EDefs[477].texIndex=73;  EDefs[477].glowIndex=72; 
    CopyMemoryFromBtoAForNBytes(EDefs[478].path,"sec_cpunode",12); EDefs[478].modelIndex=587;  EDefs[478].texIndex=242;  EDefs[478].glowIndex=248; 
    CopyMemoryFromBtoAForNBytes(EDefs[479].path,"sec_cpunode_small",18); EDefs[479].modelIndex=588;  EDefs[479].texIndex=107; 
    CopyMemoryFromBtoAForNBytes(EDefs[480].path,"weapon_cyber_mine",18); EDefs[480].modelIndex=71;  EDefs[480].texIndex=1224; 
    CopyMemoryFromBtoAForNBytes(EDefs[481].path,"proj_enemshot2",15); EDefs[481].modelIndex=MODEL_IDX_MAX;  EDefs[481].mass=0.3f; EDefs[481].angularDrag=0.05f;  EDefs[481].gravity=0.0f;  EDefs[481].kinematic=false;  EDefs[481].dynamicFriction=0.6f;  EDefs[481].staticFriction=0.6f;  EDefs[481].bounciness=0.0f;  EDefs[481].frictionCombine=PHYS_COMBINE_AVG;  EDefs[481].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[482].path,"proj_magpulse_shot",19); EDefs[482].modelIndex=MODEL_IDX_MAX;  EDefs[482].texIndex=807;  EDefs[482].mass=0.3f; EDefs[482].angularDrag=0.05f;  EDefs[482].gravity=0.0f;  EDefs[482].kinematic=false;  EDefs[482].dynamicFriction=0.6f;  EDefs[482].staticFriction=0.6f;  EDefs[482].bounciness=0.0f;  EDefs[482].frictionCombine=PHYS_COMBINE_AVG;  EDefs[482].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[483].path,"proj_stungun_shot",18); EDefs[483].modelIndex=MODEL_IDX_MAX;  EDefs[483].texIndex=835;  EDefs[483].mass=0.3f; EDefs[483].angularDrag=0.05f;  EDefs[483].gravity=0.0f;  EDefs[483].kinematic=false;  EDefs[483].dynamicFriction=0.6f;  EDefs[483].staticFriction=0.6f;  EDefs[483].bounciness=0.0f;  EDefs[483].frictionCombine=PHYS_COMBINE_AVG;  EDefs[483].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[484].path,"proj_rail_shot",15); EDefs[484].modelIndex=652;  EDefs[484].mass=0.3f; EDefs[484].angularDrag=0.05f;  EDefs[484].gravity=0.0f;  EDefs[484].kinematic=false;  EDefs[484].dynamicFriction=0.6f;  EDefs[484].staticFriction=0.6f;  EDefs[484].bounciness=0.0f;  EDefs[484].frictionCombine=PHYS_COMBINE_AVG;  EDefs[484].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[485].path,"proj_plasmarifle_shot",22); EDefs[485].modelIndex=651;  EDefs[485].mass=0.3f;  EDefs[485].angularDrag=0.05f;  EDefs[485].gravity=0.0f;  EDefs[485].kinematic=false;  EDefs[485].dynamicFriction=0.1f;  EDefs[485].staticFriction=0.2f;  EDefs[485].bounciness=0.9f;  EDefs[485].frictionCombine=PHYS_COMBINE_MUL;  EDefs[485].bounceCombine=PHYS_COMBINE_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[486].path,"proj_enemshot6",15); EDefs[486].modelIndex=MODEL_IDX_MAX;  EDefs[486].mass=0.3f; EDefs[486].angularDrag=0.05f;  EDefs[486].gravity=0.0f;  EDefs[486].kinematic=false;  EDefs[486].dynamicFriction=0.6f;  EDefs[486].staticFriction=0.6f; EDefs[486].frictionCombine=PHYS_COMBINE_AVG;  EDefs[486].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[487].path,"proj_enemshot5",15); EDefs[487].modelIndex=MODEL_IDX_MAX;  EDefs[487].mass=0.2f; EDefs[487].angularDrag=0.05f;  EDefs[487].gravity=0.0f;  EDefs[487].kinematic=false;  EDefs[487].dynamicFriction=0.6f;  EDefs[487].staticFriction=0.6f; EDefs[487].frictionCombine=PHYS_COMBINE_AVG;  EDefs[487].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[488].path,"proj_enemshot4",15); EDefs[488].modelIndex=MODEL_IDX_MAX;  EDefs[488].mass=0.3f; EDefs[488].angularDrag=0.05f;  EDefs[488].gravity=0.0f;  EDefs[488].kinematic=false;  EDefs[488].dynamicFriction=0.6f;  EDefs[488].staticFriction=0.6f; EDefs[488].frictionCombine=PHYS_COMBINE_AVG;  EDefs[488].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[489].path,"proj_throwingstar",18); EDefs[489].modelIndex=307;  EDefs[489].mass=0.3f;  EDefs[489].angularDrag=0.05f;  EDefs[489].gravity=0.0f; EDefs[489].dynamicFriction=0.6f;  EDefs[489].staticFriction=0.6f; EDefs[489].frictionCombine=PHYS_COMBINE_AVG;  EDefs[489].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[490].path,"proj_magpulsenpc_shot",22); EDefs[490].modelIndex=645;  EDefs[490].mass=0.3f; EDefs[490].angularDrag=0.05f;  EDefs[490].gravity=0.0f; EDefs[490].dynamicFriction=0.6f;  EDefs[490].staticFriction=0.6f; EDefs[490].frictionCombine=PHYS_COMBINE_AVG;  EDefs[490].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[491].path,"proj_railnpc_shot",18); EDefs[491].modelIndex=MODEL_IDX_MAX;  EDefs[491].mass=0.3f;  EDefs[491].angularDrag=0.05f;  EDefs[491].gravity=0.0f; EDefs[491].dynamicFriction=0.6f;  EDefs[491].staticFriction=0.6f; EDefs[491].frictionCombine=PHYS_COMBINE_AVG;  EDefs[491].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[492].path,"proj_cyberplayer_shot",22); EDefs[492].modelIndex=72;  EDefs[492].mass=0.3f; EDefs[492].angularDrag=0.05f;  EDefs[492].gravity=0.0f;
    CopyMemoryFromBtoAForNBytes(EDefs[493].path,"proj_cyberdog_shot",19); EDefs[493].modelIndex=63;  EDefs[493].mass=0.3f;  EDefs[493].angularDrag=0.05f;  EDefs[493].gravity=0.0f; EDefs[493].dynamicFriction=0.6f;  EDefs[493].staticFriction=0.6f; EDefs[493].frictionCombine=PHYS_COMBINE_AVG;  EDefs[493].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[494].path,"proj_cyberreaver_shot",22); EDefs[494].modelIndex=64;  EDefs[494].mass=0.3f;  EDefs[494].angularDrag=0.05f;  EDefs[494].gravity=0.0f; EDefs[494].dynamicFriction=0.6f;  EDefs[494].staticFriction=0.6f; EDefs[494].frictionCombine=PHYS_COMBINE_AVG;  EDefs[494].bounceCombine=PHYS_COMBINE_AVG; 
    CopyMemoryFromBtoAForNBytes(EDefs[495].path,"proj_cyberice_shot",19); EDefs[495].modelIndex=68;  EDefs[495].mass=0.3f; EDefs[495].angularDrag=0.05f;  EDefs[495].gravity=0.0f; EDefs[495].dynamicFriction=0.6f;  EDefs[495].staticFriction=0.6f; EDefs[495].frictionCombine=PHYS_COMBINE_AVG;  EDefs[495].bounceCombine=PHYS_COMBINE_AVG; 
    for (int i=496;i<515;++i) EDefs[i].SFXIndex = 75;
    CopyMemoryFromBtoAForNBytes(EDefs[496].path,"doorA",6); EDefs[496].modelIndex=719;  EDefs[496].texIndex=185;  EDefs[496].numclips=4;  EDefs[496].animationNum=1; 
    CopyMemoryFromBtoAForNBytes(EDefs[497].path,"doorB",6); EDefs[497].modelIndex=0;  EDefs[497].texIndex=189;  EDefs[497].glowIndex=188;  EDefs[497].numclips=4;  EDefs[497].animationNum=0; 
    CopyMemoryFromBtoAForNBytes(EDefs[498].path,"doorC",6); EDefs[498].modelIndex=0;  EDefs[498].texIndex=184;  EDefs[498].numclips=4;  EDefs[498].animationNum=5; 
    CopyMemoryFromBtoAForNBytes(EDefs[499].path,"doorD",6); EDefs[499].modelIndex=0;  EDefs[499].numclips=4;  EDefs[499].animationNum=4;  EDefs[499].texIndex=196;  EDefs[499].glowIndex=197; 
    CopyMemoryFromBtoAForNBytes(EDefs[500].path,"doorE",6); EDefs[500].modelIndex=0;  EDefs[500].numclips=4;  EDefs[500].animationNum=9;  EDefs[500].texIndex=208;  EDefs[500].glowIndex=207; 
    CopyMemoryFromBtoAForNBytes(EDefs[501].path,"doorF",6); EDefs[501].modelIndex=0;  EDefs[501].numclips=4;  EDefs[501].animationNum=10;  EDefs[501].texIndex=187; 
    CopyMemoryFromBtoAForNBytes(EDefs[502].path,"doorG",6); EDefs[502].modelIndex=0;  EDefs[502].numclips=4;  EDefs[502].animationNum=11;  EDefs[502].texIndex=193;  EDefs[502].glowIndex=194; 
    CopyMemoryFromBtoAForNBytes(EDefs[503].path,"doorH",6); EDefs[503].modelIndex=0;  EDefs[503].numclips=4;  EDefs[503].animationNum=12;  EDefs[503].texIndex=190; 
    CopyMemoryFromBtoAForNBytes(EDefs[504].path,"doorI",6); EDefs[504].modelIndex=0;  EDefs[504].numclips=4;  EDefs[504].animationNum=13;  EDefs[504].texIndex=200;  EDefs[504].glowIndex=199; 
    CopyMemoryFromBtoAForNBytes(EDefs[505].path,"doorJ",6); EDefs[505].modelIndex=0;  EDefs[505].numclips=4;  EDefs[505].animationNum=6;  EDefs[505].texIndex=215; 
    CopyMemoryFromBtoAForNBytes(EDefs[506].path,"doorK",6); EDefs[506].modelIndex=0;  EDefs[506].numclips=4;  EDefs[506].animationNum=7;  EDefs[506].texIndex=214; 
    CopyMemoryFromBtoAForNBytes(EDefs[507].path,"doorL",6); EDefs[507].modelIndex=0;  EDefs[507].numclips=4;  EDefs[507].animationNum=8;  EDefs[507].texIndex=191; 
    CopyMemoryFromBtoAForNBytes(EDefs[508].path,"door_elevator1",15); EDefs[508].modelIndex=0;  EDefs[508].numclips=4;  EDefs[508].animationNum=14;  EDefs[508].texIndex=202; 
    CopyMemoryFromBtoAForNBytes(EDefs[509].path,"door_elevator2",15); EDefs[509].modelIndex=0;  EDefs[509].numclips=4;  EDefs[509].animationNum=15;  EDefs[509].texIndex=203; 
    CopyMemoryFromBtoAForNBytes(EDefs[510].path,"door_elevator3",15); EDefs[510].modelIndex=0;  EDefs[510].numclips=4;  EDefs[510].animationNum=16;  EDefs[510].texIndex=206;  EDefs[510].glowIndex=205; 
    CopyMemoryFromBtoAForNBytes(EDefs[511].path,"door_elevator4",15); EDefs[511].modelIndex=0;  EDefs[511].numclips=4;  EDefs[511].animationNum=17;  EDefs[511].texIndex=203; 
    CopyMemoryFromBtoAForNBytes(EDefs[512].path,"door_secret1",13); EDefs[512].modelIndex=0;  EDefs[512].numclips=4;  EDefs[512].animationNum=19;  EDefs[512].texIndex=210; 
    CopyMemoryFromBtoAForNBytes(EDefs[513].path,"door_secret2",13); EDefs[513].modelIndex=0;  EDefs[513].numclips=4;  EDefs[513].animationNum=18;  EDefs[513].texIndex=209; 
    CopyMemoryFromBtoAForNBytes(EDefs[514].path,"door_secret3",13); EDefs[514].modelIndex=94;  EDefs[514].numclips=4;  EDefs[514].animationNum=20;  EDefs[514].texIndex=211; 
    CopyMemoryFromBtoAForNBytes(EDefs[515].path,"func_forcebridge",17); EDefs[515].modelIndex=78;  EDefs[515].texIndex=38; 
    CopyMemoryFromBtoAForNBytes(EDefs[516].path,"prop_lift2",11); EDefs[516].modelIndex=215;  EDefs[516].texIndex=155;  EDefs[516].glowIndex=154;  EDefs[516].collider=COLLIDER_TYPE_BOX;  EDefs[516].colliderCenter=(Vector3){0.0f,0.0f,0.0f};  EDefs[516].colliderSize=(Vector3){1.0f,1.0f,1.0f};  EDefs[516].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[517].path,"func_wall",10);  EDefs[517].mass=10.0f;  EDefs[517].angularDrag=0.05f;  EDefs[517].gravity=0.0f;  EDefs[517].kinematic=true;  EDefs[517].dynamicFriction=0.6f;  EDefs[517].staticFriction=0.6f;  EDefs[517].bounciness=0.0f;  EDefs[517].frictionCombine=PHYS_COMBINE_AVG;  EDefs[517].bounceCombine=PHYS_COMBINE_AVG;
    CopyMemoryFromBtoAForNBytes(EDefs[518].path,"BulletHoleLarge",16);
    CopyMemoryFromBtoAForNBytes(EDefs[519].path,"BulletHoleScorchLarge",22);
    CopyMemoryFromBtoAForNBytes(EDefs[520].path,"BulletHoleScorchSmall",22);
    CopyMemoryFromBtoAForNBytes(EDefs[521].path,"BulletHoleSmall",16);
    CopyMemoryFromBtoAForNBytes(EDefs[522].path,"BulletHoleTiny",15);
    CopyMemoryFromBtoAForNBytes(EDefs[523].path,"BulletHoleTinySpread",21);
    CopyMemoryFromBtoAForNBytes(EDefs[524].path,"func_door_cyber",16); EDefs[524].modelIndex=178;  EDefs[524].texIndex=1224;  EDefs[524].collider=COLLIDER_TYPE_BOX;  EDefs[524].colliderCenter=(Vector3){0.0f,1.31f,0.0f};  EDefs[524].colliderSize=(Vector3){2.56f,0.06f,2.56f};  EDefs[524].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[525].path,"prop_console01",15); EDefs[525].texIndex=100;  EDefs[525].modelIndex=49; 
    CopyMemoryFromBtoAForNBytes(EDefs[526].path,"prop_console02",15); EDefs[526].texIndex=100;  EDefs[526].modelIndex=50; 
    CopyMemoryFromBtoAForNBytes(EDefs[527].path,"prop_grate1_1",14); EDefs[527].modelIndex=186;  EDefs[527].texIndex=359; 
    CopyMemoryFromBtoAForNBytes(EDefs[528].path,"prop_grate1_2",14); EDefs[528].modelIndex=187;  EDefs[528].texIndex=360; 
    CopyMemoryFromBtoAForNBytes(EDefs[529].path,"prop_grate1_3",14); EDefs[529].modelIndex=188;  EDefs[529].texIndex=361; 
    CopyMemoryFromBtoAForNBytes(EDefs[530].path,"se_cabinet",11); EDefs[530].modelIndex=39;  EDefs[530].texIndex=70; 
    CopyMemoryFromBtoAForNBytes(EDefs[531].path,"se_thermos",11); EDefs[531].texIndex=863;  EDefs[531].modelIndex=623; 
    CopyMemoryFromBtoAForNBytes(EDefs[532].path,"prop_beaker_holder",19); EDefs[532].modelIndex=15;  EDefs[532].texIndex=36; 
    CopyMemoryFromBtoAForNBytes(EDefs[533].path,"prop_bed",9); EDefs[533].modelIndex=16;  EDefs[533].texIndex=246; 
    CopyMemoryFromBtoAForNBytes(EDefs[534].path,"prop_bed_hospital",18); EDefs[534].modelIndex=608;  EDefs[534].texIndex=759; 
    CopyMemoryFromBtoAForNBytes(EDefs[535].path,"prop_bed_neurosurgery",22); EDefs[535].texIndex=18;  EDefs[535].normIndex=29;  EDefs[535].specIndex=1238;  EDefs[535].modelIndex=17; 
    CopyMemoryFromBtoAForNBytes(EDefs[536].path,"prop_bonepile1",15); EDefs[536].modelIndex=19;  EDefs[536].texIndex=815; 
    CopyMemoryFromBtoAForNBytes(EDefs[537].path,"prop_bridgewall1",17); EDefs[537].modelIndex=33; 
    CopyMemoryFromBtoAForNBytes(EDefs[538].path,"prop_broken_clock",18); EDefs[538].modelIndex=38;  EDefs[538].texIndex=1117;  EDefs[538].altTexIndex=1118;  EDefs[538].glowIndex=1115;  EDefs[538].altGlowIndex=1116; 
    CopyMemoryFromBtoAForNBytes(EDefs[539].path,"prop_brokengun",15); EDefs[539].modelIndex=639;  EDefs[539].texIndex=878; 
    CopyMemoryFromBtoAForNBytes(EDefs[540].path,"prop_chair01",13); EDefs[540].modelIndex=41;  EDefs[540].texIndex=195; 
    CopyMemoryFromBtoAForNBytes(EDefs[541].path,"prop_chair02",13); EDefs[541].modelIndex=42;  EDefs[541].texIndex=195; 
    CopyMemoryFromBtoAForNBytes(EDefs[542].path,"prop_chair03",13); EDefs[542].modelIndex=43;  EDefs[542].texIndex=195; 
    CopyMemoryFromBtoAForNBytes(EDefs[543].path,"prop_chair04",13); EDefs[543].modelIndex=41;  EDefs[543].texIndex=195; 
    CopyMemoryFromBtoAForNBytes(EDefs[544].path,"prop_chair05",13); EDefs[544].modelIndex=42;  EDefs[544].texIndex=195; 
    CopyMemoryFromBtoAForNBytes(EDefs[545].path,"prop_chandelier",16); EDefs[545].modelIndex=496;  EDefs[545].texIndex=644; 
    CopyMemoryFromBtoAForNBytes(EDefs[546].path,"prop_charge_station",20); EDefs[546].modelIndex=44;  EDefs[546].texIndex=77;  EDefs[546].glowIndex=76; 
    CopyMemoryFromBtoAForNBytes(EDefs[547].path,"prop_clothes",13); EDefs[547].modelIndex=47;  EDefs[547].texIndex=97; 
    CopyMemoryFromBtoAForNBytes(EDefs[548].path,"prop_computer",14); EDefs[548].modelIndex=48; 
    CopyMemoryFromBtoAForNBytes(EDefs[549].path,"prop_couch",11); EDefs[549].modelIndex=59; 
    CopyMemoryFromBtoAForNBytes(EDefs[550].path,"prop_couch2",12); EDefs[550].modelIndex=59; 
    CopyMemoryFromBtoAForNBytes(EDefs[551].path,"prop_cpuscreen",15); EDefs[551].modelIndex=178;  EDefs[551].texIndex=768; 
    CopyMemoryFromBtoAForNBytes(EDefs[552].path,"prop_cyber_datafrag",20); EDefs[552].modelIndex=78; 
    CopyMemoryFromBtoAForNBytes(EDefs[553].path,"prop_cyber_decoy",17); EDefs[553].modelIndex=78; 
    CopyMemoryFromBtoAForNBytes(EDefs[554].path,"prop_cyber_exit",16); EDefs[554].modelIndex=78; 
    CopyMemoryFromBtoAForNBytes(EDefs[555].path,"prop_cyber_switch",18); EDefs[555].modelIndex=0; 
    CopyMemoryFromBtoAForNBytes(EDefs[556].path,"prop_cyberport",15); EDefs[556].modelIndex=62;  EDefs[556].texIndex=117;  EDefs[556].glowIndex=116; 
    CopyMemoryFromBtoAForNBytes(EDefs[557].path,"prop_desk01",12); EDefs[557].modelIndex=74;  EDefs[557].texIndex=125; 
    CopyMemoryFromBtoAForNBytes(EDefs[558].path,"prop_desk02",12); EDefs[558].modelIndex=75;  EDefs[558].texIndex=124; 
    CopyMemoryFromBtoAForNBytes(EDefs[559].path,"prop_dexmissile",16); EDefs[559].modelIndex=76;  EDefs[559].texIndex=164;  EDefs[559].glowIndex=162; 
    CopyMemoryFromBtoAForNBytes(EDefs[560].path,"prop_foliage_fernpoison",24); EDefs[560].modelIndex=160;  EDefs[560].texIndex=331; 
    CopyMemoryFromBtoAForNBytes(EDefs[561].path,"prop_foliage_bush",18); EDefs[561].modelIndex=495;  EDefs[561].texIndex=643;  EDefs[561].glowIndex=642; 
    CopyMemoryFromBtoAForNBytes(EDefs[562].path,"prop_foliage_fern",18); EDefs[562].modelIndex=160;  EDefs[562].texIndex=333;  EDefs[562].glowIndex=330; 
    CopyMemoryFromBtoAForNBytes(EDefs[563].path,"prop_foliage_fernblueflower",28); EDefs[563].modelIndex=159;  EDefs[563].texIndex=333;  EDefs[563].glowIndex=330; 
    CopyMemoryFromBtoAForNBytes(EDefs[564].path,"prop_foliage_pinetreem",23); EDefs[564].modelIndex=489;  EDefs[564].texIndex=594; 
    CopyMemoryFromBtoAForNBytes(EDefs[565].path,"prop_foliage_poisonbush1",25); EDefs[565].modelIndex=493; EDefs[565].texIndex=638;
    CopyMemoryFromBtoAForNBytes(EDefs[566].path,"prop_gear_large",16); EDefs[566].modelIndex=166; EDefs[566].texIndex=335;
    CopyMemoryFromBtoAForNBytes(EDefs[567].path,"prop_gear_small",16); EDefs[567].modelIndex=167; EDefs[567].texIndex=336;
    CopyMemoryFromBtoAForNBytes(EDefs[568].path,"prop_grass1",12); EDefs[568].texIndex=329;
    CopyMemoryFromBtoAForNBytes(EDefs[569].path,"prop_grass2",12); EDefs[569].texIndex=329;
    CopyMemoryFromBtoAForNBytes(EDefs[570].path,"prop_grass3",12); EDefs[570].texIndex=329;
    CopyMemoryFromBtoAForNBytes(EDefs[571].path,"prop_grass4",12); EDefs[571].texIndex=329;
    CopyMemoryFromBtoAForNBytes(EDefs[572].path,"prop_grass5",12); EDefs[572].texIndex=329;
    CopyMemoryFromBtoAForNBytes(EDefs[573].path,"prop_grate4",12); EDefs[573].modelIndex=161;  EDefs[573].texIndex=329; 
    CopyMemoryFromBtoAForNBytes(EDefs[574].path,"prop_healingbed",16); EDefs[574].modelIndex=195;  EDefs[574].texIndex=1139; 
    CopyMemoryFromBtoAForNBytes(EDefs[575].path,"prop_lamp",10); EDefs[575].modelIndex=212;  EDefs[575].texIndex=423; 
    CopyMemoryFromBtoAForNBytes(EDefs[576].path,"prop_light_emergsignal",23); EDefs[576].modelIndex=216;  EDefs[576].texIndex=426;  EDefs[576].altTexIndex=424;  EDefs[576].glowIndex=0;  EDefs[576].altGlowIndex=424; 
    CopyMemoryFromBtoAForNBytes(EDefs[577].path,"prop_microscope",16); EDefs[577].modelIndex=298;  EDefs[577].texIndex=645;  EDefs[577].specIndex=1241; 
    CopyMemoryFromBtoAForNBytes(EDefs[578].path,"prop_pipe",10); EDefs[578].modelIndex=490;  EDefs[578].texIndex=595; 
    CopyMemoryFromBtoAForNBytes(EDefs[579].path,"prop_puddle",12); EDefs[579].modelIndex=157;  EDefs[579].texIndex=648; 
    CopyMemoryFromBtoAForNBytes(EDefs[580].path,"prop_puddle_grease",19); EDefs[580].modelIndex=157;  EDefs[580].texIndex=650; 
    CopyMemoryFromBtoAForNBytes(EDefs[581].path,"prop_puddle_oil",16); EDefs[581].modelIndex=157;  EDefs[581].texIndex=652; 
    CopyMemoryFromBtoAForNBytes(EDefs[582].path,"prop_shelves",13); EDefs[582].modelIndex=591;  EDefs[582].texIndex=94; 
    CopyMemoryFromBtoAForNBytes(EDefs[583].path,"prop_skeleton",14); EDefs[583].modelIndex=592;  EDefs[583].texIndex=815; 
    CopyMemoryFromBtoAForNBytes(EDefs[584].path,"prop_sleeping_cables",21); EDefs[584].modelIndex=595;  EDefs[584].texIndex=71; 
    CopyMemoryFromBtoAForNBytes(EDefs[585].path,"prop_sparkingwire",18); EDefs[585].modelIndex=0;  EDefs[585].numclips=1;  EDefs[585].animationNum=46;  EDefs[585].texIndex=71; 
    CopyMemoryFromBtoAForNBytes(EDefs[586].path,"prop_table",11); EDefs[586].modelIndex=619;  EDefs[586].texIndex=92; 
    CopyMemoryFromBtoAForNBytes(EDefs[587].path,"prop_tv_on_a_post",18); EDefs[587].modelIndex=625;  EDefs[587].texIndex=1228; 
    CopyMemoryFromBtoAForNBytes(EDefs[588].path,"prop_vendingmachines1",22); EDefs[588].modelIndex=627;  EDefs[588].texIndex=870; 
    CopyMemoryFromBtoAForNBytes(EDefs[589].path,"prop_vendingmachines2",22); EDefs[589].modelIndex=614;  EDefs[589].texIndex=871; 
    CopyMemoryFromBtoAForNBytes(EDefs[590].path,"prop_weapon_rack",17); EDefs[590].modelIndex=641;  EDefs[590].texIndex=113; 
    CopyMemoryFromBtoAForNBytes(EDefs[591].path,"prop_xray",10); EDefs[591].modelIndex=660;  EDefs[591].texIndex=153; 
    CopyMemoryFromBtoAForNBytes(EDefs[592].path,"text_decal",11); EDefs[592].modelIndex=77; 
    CopyMemoryFromBtoAForNBytes(EDefs[593].path,"text_decalStopDSS1",19); EDefs[593].modelIndex=77; 
    CopyMemoryFromBtoAForNBytes(EDefs[594].path,"trigger_counter",16);
    CopyMemoryFromBtoAForNBytes(EDefs[595].path,"trigger_cyberpush",18);
    CopyMemoryFromBtoAForNBytes(EDefs[596].path,"trigger_gravitylift",20);
    CopyMemoryFromBtoAForNBytes(EDefs[597].path,"trigger_ladder",15);
    CopyMemoryFromBtoAForNBytes(EDefs[598].path,"trigger_multiple",17);
    CopyMemoryFromBtoAForNBytes(EDefs[599].path,"trigger_music",14);
    CopyMemoryFromBtoAForNBytes(EDefs[600].path,"trigger_once",13);
    CopyMemoryFromBtoAForNBytes(EDefs[601].path,"trigger_radiation",18);
    CopyMemoryFromBtoAForNBytes(EDefs[602].path,"us_isotopepanel",16); EDefs[602].modelIndex=0;  EDefs[602].texIndex=616;  EDefs[602].numclips=5;  EDefs[602].animationNum=44; 
    CopyMemoryFromBtoAForNBytes(EDefs[603].path,"us_paperlog",12); EDefs[603].modelIndex=486;  EDefs[603].texIndex=580; 
    CopyMemoryFromBtoAForNBytes(EDefs[604].path,"us_puz_elevatorkeypad",22); EDefs[604].modelIndex=615;  EDefs[604].texIndex=247; 
    CopyMemoryFromBtoAForNBytes(EDefs[605].path,"us_puz_elevatorkeypad2",23); EDefs[605].modelIndex=618;  EDefs[605].texIndex=250; 
    CopyMemoryFromBtoAForNBytes(EDefs[606].path,"us_puz_elevatorkeypad3",23); EDefs[606].modelIndex=615;  EDefs[606].texIndex=247; 
    CopyMemoryFromBtoAForNBytes(EDefs[607].path,"us_puz_elevatorkeypad4",23); EDefs[607].modelIndex=210;  EDefs[607].texIndex=249; 
    CopyMemoryFromBtoAForNBytes(EDefs[608].path,"us_puz_keypad",14); EDefs[608].modelIndex=211;  EDefs[608].texIndex=414; 
    CopyMemoryFromBtoAForNBytes(EDefs[609].path,"us_puz_panel_blue_grid",23); EDefs[609].modelIndex=0;  EDefs[609].texIndex=604;  EDefs[609].numclips=3;  EDefs[609].animationNum=43; 
    CopyMemoryFromBtoAForNBytes(EDefs[610].path,"us_puz_panel_brown_grid",24); EDefs[610].modelIndex=0;  EDefs[610].texIndex=604;  EDefs[610].numclips=3;  EDefs[610].animationNum=43; 
    CopyMemoryFromBtoAForNBytes(EDefs[611].path,"us_puz_panel_gray_grid",23); EDefs[611].modelIndex=0;  EDefs[611].texIndex=634;  EDefs[611].numclips=3;  EDefs[611].animationNum=43; 
    CopyMemoryFromBtoAForNBytes(EDefs[612].path,"us_puz_panel_red_grid",22); EDefs[612].modelIndex=0;  EDefs[612].texIndex=625;  EDefs[612].numclips=3;  EDefs[612].animationNum=43; 
    CopyMemoryFromBtoAForNBytes(EDefs[613].path,"us_puz_panel_teal_grid",23); EDefs[613].modelIndex=0;  EDefs[613].texIndex=601;  EDefs[613].numclips=3;  EDefs[613].animationNum=43; 
    CopyMemoryFromBtoAForNBytes(EDefs[614].path,"us_relaypanel",14); EDefs[614].modelIndex=0;  EDefs[614].texIndex=617;  EDefs[614].numclips=4;  EDefs[614].animationNum=45; 
    CopyMemoryFromBtoAForNBytes(EDefs[615].path,"us_retinalscanner",18); EDefs[615].modelIndex=79;  EDefs[615].texIndex=46; 
    CopyMemoryFromBtoAForNBytes(EDefs[616].path,"prop_vending1_1",16); EDefs[616].modelIndex=627;  EDefs[616].texIndex=870; 
    CopyMemoryFromBtoAForNBytes(EDefs[617].path,"prop_vending1_2",16); EDefs[617].modelIndex=628;  EDefs[617].texIndex=870; 
    CopyMemoryFromBtoAForNBytes(EDefs[618].path,"prop_vending1_3",16); EDefs[618].modelIndex=629;  EDefs[618].texIndex=870; 
    CopyMemoryFromBtoAForNBytes(EDefs[619].path,"prop_vending2_1",16); EDefs[619].modelIndex=614;  EDefs[619].texIndex=871; 
    CopyMemoryFromBtoAForNBytes(EDefs[620].path,"prop_vending2_2",16); EDefs[620].modelIndex=621;  EDefs[620].texIndex=871; 
    CopyMemoryFromBtoAForNBytes(EDefs[621].path,"ambient_airhiss",16); EDefs[621].volume=0.05f;
    CopyMemoryFromBtoAForNBytes(EDefs[622].path,"ambient_clicker",16); EDefs[622].volume=0.20f;
    CopyMemoryFromBtoAForNBytes(EDefs[623].path,"ambient_compressor",19); EDefs[623].volume=0.4f;
    CopyMemoryFromBtoAForNBytes(EDefs[624].path,"ambient_dishwasher",19); EDefs[624].volume=0.2f;
    CopyMemoryFromBtoAForNBytes(EDefs[625].path,"ambient_drip_amb",17); EDefs[625].volume=0.5f;
    CopyMemoryFromBtoAForNBytes(EDefs[626].path,"ambient_fan",12); EDefs[626].volume=0.3f;
    CopyMemoryFromBtoAForNBytes(EDefs[627].path,"ambient_generator_gas",22); EDefs[627].volume=0.3f;
    CopyMemoryFromBtoAForNBytes(EDefs[628].path,"ambient_gurgle",15); EDefs[628].volume=0.3f;
    CopyMemoryFromBtoAForNBytes(EDefs[629].path,"ambient_icemaker",17); EDefs[629].volume=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[630].path,"ambient_intake",15); EDefs[630].volume=0.2f;
    CopyMemoryFromBtoAForNBytes(EDefs[631].path,"ambient_lathe",14); EDefs[631].volume=0.4f;
    CopyMemoryFromBtoAForNBytes(EDefs[632].path,"ambient_lev3loop1",18); EDefs[632].volume=0.1f;
    CopyMemoryFromBtoAForNBytes(EDefs[633].path,"ambient_lev3loop2",18); EDefs[633].volume=0.1f;
    CopyMemoryFromBtoAForNBytes(EDefs[634].path,"ambient_lev3loop3",18); EDefs[634].volume=0.1f;
    CopyMemoryFromBtoAForNBytes(EDefs[635].path,"ambient_lev3loop4",18); EDefs[635].volume=0.1f;
    CopyMemoryFromBtoAForNBytes(EDefs[636].path,"ambient_liquid_bubble",22); EDefs[636].volume=1.0f;
    CopyMemoryFromBtoAForNBytes(EDefs[637].path,"ambient_liquid_lava2",21); EDefs[637].volume=0.4f;
    CopyMemoryFromBtoAForNBytes(EDefs[638].path,"ambient_looping",16); EDefs[638].volume=0.4f;
    CopyMemoryFromBtoAForNBytes(EDefs[639].path,"ambient_machgear_loop",22); EDefs[639].volume=0.4f;
    CopyMemoryFromBtoAForNBytes(EDefs[640].path,"ambient_machine_ambience",25); EDefs[640].volume=0.8f;
    CopyMemoryFromBtoAForNBytes(EDefs[641].path,"ambient_machine_go",19); EDefs[641].volume=0.6f;
    CopyMemoryFromBtoAForNBytes(EDefs[642].path,"ambient_machine_humamb7",24); EDefs[642].volume=1.0f;
    CopyMemoryFromBtoAForNBytes(EDefs[643].path,"ambient_machine_humlonoise",27); EDefs[643].volume=0.4f;
    CopyMemoryFromBtoAForNBytes(EDefs[644].path,"ambient_machine_loop1",22); EDefs[644].volume=0.4f;
    CopyMemoryFromBtoAForNBytes(EDefs[645].path,"ambient_machine_loop2",22); EDefs[645].volume=0.4f;
    CopyMemoryFromBtoAForNBytes(EDefs[646].path,"ambient_machinea1",18); EDefs[646].volume=0.4f;
    CopyMemoryFromBtoAForNBytes(EDefs[647].path,"ambient_machinevat_loop",24); EDefs[647].volume=0.8f;
    CopyMemoryFromBtoAForNBytes(EDefs[648].path,"ambient_mist",13); EDefs[648].volume=0.02f;
    CopyMemoryFromBtoAForNBytes(EDefs[649].path,"ambient_pipewater_loop",23); EDefs[649].volume=0.65f;
    CopyMemoryFromBtoAForNBytes(EDefs[650].path,"ambient_powerloom",18); EDefs[650].volume=0.3f;
    CopyMemoryFromBtoAForNBytes(EDefs[651].path,"ambient_pump",13); EDefs[651].volume=0.2f;
    CopyMemoryFromBtoAForNBytes(EDefs[652].path,"ambient_pump2",14); EDefs[652].volume=0.05f;
    CopyMemoryFromBtoAForNBytes(EDefs[653].path,"ambient_rain",13); EDefs[653].volume=0.55f;
    CopyMemoryFromBtoAForNBytes(EDefs[654].path,"ambient_steam_loop",19); EDefs[654].volume=0.1f;
    CopyMemoryFromBtoAForNBytes(EDefs[655].path,"ambient_washing_machine",24); EDefs[655].volume=0.5f;
    CopyMemoryFromBtoAForNBytes(EDefs[656].path,"decal_blood_die",16); EDefs[656].modelIndex=77;  EDefs[656].texIndex=237;  EDefs[656].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[657].path,"decal_blood_resist",19); EDefs[657].modelIndex=77;  EDefs[657].texIndex=240;  EDefs[657].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[658].path,"decal_blood_stayaway",21); EDefs[658].modelIndex=77;  EDefs[658].texIndex=235;  EDefs[658].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[659].path,"decal_blood_words2",19); EDefs[659].modelIndex=77;  EDefs[659].texIndex=236;  EDefs[659].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[660].path,"decal_bloodfonta",17); EDefs[660].modelIndex=178;  EDefs[660].texIndex=118;  EDefs[660].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[661].path,"decal_bloodfonte",17); EDefs[661].modelIndex=178;  EDefs[661].texIndex=121;  EDefs[661].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[662].path,"decal_bloodfontg",17); EDefs[662].modelIndex=178;  EDefs[662].texIndex=122;  EDefs[662].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[663].path,"decal_bloodfonth",17); EDefs[663].modelIndex=178;  EDefs[663].texIndex=89;  EDefs[663].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[664].path,"decal_bloodfontr",17); EDefs[664].modelIndex=178;  EDefs[664].texIndex=139;  EDefs[664].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[665].path,"decal_bloodfonty",17); EDefs[665].modelIndex=178;  EDefs[665].texIndex=140;  EDefs[665].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[666].path,"decal_bloodsplat2",18); EDefs[666].modelIndex=157;  EDefs[666].texIndex=130;  EDefs[666].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[667].path,"decal_logo_antenna",19); EDefs[667].modelIndex=77;  EDefs[667].texIndex=182;  EDefs[667].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[668].path,"decal_logo_armory",18); EDefs[668].modelIndex=77;  EDefs[668].texIndex=178;  EDefs[668].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[669].path,"decal_logo_biohazard",21); EDefs[669].modelIndex=77;  EDefs[669].texIndex=180;  EDefs[669].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[670].path,"decal_logo_bridge",18); EDefs[670].modelIndex=77;  EDefs[670].texIndex=181;  EDefs[670].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[671].path,"decal_logo_cyborg",18); EDefs[671].modelIndex=77;  EDefs[671].texIndex=176;  EDefs[671].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[672].path,"decal_logo_gears",17); EDefs[672].modelIndex=77;  EDefs[672].texIndex=174;  EDefs[672].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[673].path,"decal_logo_medical",19); EDefs[673].modelIndex=77;  EDefs[673].texIndex=165;  EDefs[673].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[674].path,"decal_logo_radhazard",21); EDefs[674].modelIndex=77;  EDefs[674].texIndex=177;  EDefs[674].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[675].path,"decal_logo_research",20); EDefs[675].modelIndex=77;  EDefs[675].texIndex=175;  EDefs[675].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[676].path,"decal_logo_security",20); EDefs[676].modelIndex=77;  EDefs[676].texIndex=167;  EDefs[676].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[677].path,"decal_painting1",16); EDefs[677].modelIndex=77;  EDefs[677].texIndex=218;  EDefs[677].glowIndex=216;  EDefs[677].normIndex=217;  EDefs[677].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[678].path,"decal_painting2",16); EDefs[678].modelIndex=77;  EDefs[678].texIndex=220;  EDefs[678].glowIndex=219;  EDefs[678].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[679].path,"decal_painting3",16); EDefs[679].modelIndex=77;  EDefs[679].texIndex=222;  EDefs[679].glowIndex=221;  EDefs[679].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[680].path,"decal_posterbetterfuture",25); EDefs[680].modelIndex=77;  EDefs[680].texIndex=226;  EDefs[680].normIndex=225;  EDefs[680].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[681].path,"decal_postergenetics",21); EDefs[681].modelIndex=77;  EDefs[681].texIndex=224;  EDefs[681].normIndex=223;  EDefs[681].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[682].path,"decal_scorch1",14); EDefs[682].modelIndex=77;  EDefs[682].texIndex=227;  EDefs[682].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[683].path,"decal_scorch2",14); EDefs[683].modelIndex=77;  EDefs[683].texIndex=228;  EDefs[683].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[684].path,"decal_scorch3",14); EDefs[684].modelIndex=77;  EDefs[684].texIndex=229;  EDefs[684].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[685].path,"decal_scorch4",14); EDefs[685].modelIndex=77;  EDefs[685].texIndex=230;  EDefs[685].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[686].path,"decal_scorchtiny",17); EDefs[686].modelIndex=77;  EDefs[686].texIndex=232;  EDefs[686].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[687].path,"decal_blood_splat",18); EDefs[687].modelIndex=77;  EDefs[687].texIndex=234;  EDefs[687].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[688].path,"func_switch1",13); EDefs[688].modelIndex=609;  EDefs[688].texIndex=837;  EDefs[688].collider=COLLIDER_TYPE_BOX;  EDefs[688].colliderCenter=(Vector3){0.0f,0.0f,0.0f};  EDefs[688].colliderSize=(Vector3){0.32f,0.04f,0.32f}; EDefs[688].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[689].path,"func_switch2",13); EDefs[689].modelIndex=610;  EDefs[689].texIndex=839;  EDefs[689].mainSwitchMaterial=839;  EDefs[689].altTexIndex=841;  EDefs[689].glowIndex=0;  EDefs[689].altGlowIndex=840;  EDefs[689].changeTexOnActive=true; EDefs[689].blinkTexOnActive=true;  EDefs[689].collider=COLLIDER_TYPE_BOX;  EDefs[689].colliderCenter=(Vector3){-0.0243553f,0.0f,0.000004883f};  EDefs[689].colliderSize=(Vector3){0.0476318f,0.64f,0.64f};  EDefs[689].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[690].path,"func_switch3",13); EDefs[690].modelIndex=611;  EDefs[690].texIndex=842;  EDefs[690].altTexIndex=844;  EDefs[690].glowIndex=0;  EDefs[690].altGlowIndex=843;  EDefs[690].changeTexOnActive=true;  EDefs[690].collider=COLLIDER_TYPE_BOX;  EDefs[690].colliderCenter=(Vector3){-0.02285008f,0.000053061f,-0.000056993f};  EDefs[690].colliderSize=(Vector3){0.02f,0.32f,0.32f};  EDefs[690].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[691].path,"func_switch4",13); EDefs[691].modelIndex=612;  EDefs[691].texIndex=846;  EDefs[691].collider=COLLIDER_TYPE_BOX;  EDefs[691].colliderCenter=(Vector3){0.06f,0.0f,0.0f};  EDefs[691].colliderSize=(Vector3){0.2f,0.64f,0.64f};  EDefs[691].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[692].path,"func_switch5",13); EDefs[692].modelIndex=614;  EDefs[692].texIndex=848;  EDefs[692].collider=COLLIDER_TYPE_BOX;  EDefs[692].colliderCenter=(Vector3){0.0f,0.0f,0.0f};  EDefs[692].colliderSize=(Vector3){0.64f,0.64f,0.08f};  EDefs[692].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[693].path,"func_switch5broken",19); EDefs[693].modelIndex=613;  EDefs[693].texIndex=847;  EDefs[693].collider=COLLIDER_TYPE_BOX;  EDefs[693].colliderCenter=(Vector3){0.0f,0.0f,0.0f};  EDefs[693].colliderSize=(Vector3){0.64f,0.64f,0.08f};  EDefs[693].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[694].path,"func_switch7",13); EDefs[694].modelIndex=612;  EDefs[694].texIndex=854;  EDefs[694].collider=COLLIDER_TYPE_BOX;  EDefs[694].colliderCenter=(Vector3){1.523325f,0.0f,0.0f};  EDefs[694].colliderSize=(Vector3){0.2008026f,0.64f,0.64f};  EDefs[694].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[695].path,"func_switch8",13); EDefs[695].modelIndex=616;  EDefs[695].texIndex=856;  EDefs[695].altTexIndex=858;  EDefs[695].glowIndex=855;  EDefs[695].altGlowIndex=857;  EDefs[695].changeTexOnActive=true;  EDefs[695].collider=COLLIDER_TYPE_BOX;  EDefs[695].colliderCenter=(Vector3){-0.04f,0.0f,0.0001220703f};  EDefs[695].colliderSize=(Vector3){0.08f,0.64f,0.64f};  EDefs[695].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[696].path,"func_switchbroken1",19); EDefs[696].modelIndex=617;  EDefs[696].texIndex=618; 
    CopyMemoryFromBtoAForNBytes(EDefs[697].path,"clip_npc",9); EDefs[697].collider=COLLIDER_TYPE_BOX;  EDefs[697].colliderCenter=(Vector3){1.005016f,0.0f,0.0f};  EDefs[697].colliderSize=(Vector3){2.010033f,16.0f,16.0f};  EDefs[697].colliderMeshIndex=U16_MAX;
    CopyMemoryFromBtoAForNBytes(EDefs[698].path,"clip_objects",13); EDefs[698].collider=COLLIDER_TYPE_BOX;  EDefs[698].colliderCenter=(Vector3){0.0f,0.0f,0.0f};  EDefs[698].colliderSize=(Vector3){2.56f,2.56f,2.56f};  EDefs[698].colliderMeshIndex=U16_MAX;
    CopyMemoryFromBtoAForNBytes(EDefs[699].path,"logic_relay",12);
    CopyMemoryFromBtoAForNBytes(EDefs[700].path,"logic_branch",13);
    CopyMemoryFromBtoAForNBytes(EDefs[701].path,"logic_timer",12);
    CopyMemoryFromBtoAForNBytes(EDefs[702].path,"logic_spawner",14);
    CopyMemoryFromBtoAForNBytes(EDefs[703].path,"info_teleport_destination",26);
    CopyMemoryFromBtoAForNBytes(EDefs[704].path,"prop_debris_panel",18);
    CopyMemoryFromBtoAForNBytes(EDefs[705].path,"info_cyborgconversion",22);
    CopyMemoryFromBtoAForNBytes(EDefs[706].path,"info_elev_destination",22);
    CopyMemoryFromBtoAForNBytes(EDefs[707].path,"info_email",11);
    CopyMemoryFromBtoAForNBytes(EDefs[708].path,"info_gameend",13);
    CopyMemoryFromBtoAForNBytes(EDefs[709].path,"info_message",13);
    CopyMemoryFromBtoAForNBytes(EDefs[710].path,"info_mission",13);
    CopyMemoryFromBtoAForNBytes(EDefs[711].path,"info_note",10);
    CopyMemoryFromBtoAForNBytes(EDefs[712].path,"info_playsound",15);
    CopyMemoryFromBtoAForNBytes(EDefs[713].path,"info_ressurection_point",24);
    CopyMemoryFromBtoAForNBytes(EDefs[714].path,"info_screenshake",17);
    CopyMemoryFromBtoAForNBytes(EDefs[715].path,"info_spawnpoint",16);
    CopyMemoryFromBtoAForNBytes(EDefs[716].path,"fx_reverbzone",14);
    CopyMemoryFromBtoAForNBytes(EDefs[717].path,"ef_cyber_ice",13); EDefs[717].collider=COLLIDER_TYPE_SPHERE;  EDefs[717].colliderCenter=(Vector3){0.0f,0.004354001f,-0.014725f};  EDefs[717].colliderSize=(Vector3){1.0f,0.0f,0.0f};  EDefs[717].colliderMeshIndex=U16_MAX; 
    CopyMemoryFromBtoAForNBytes(EDefs[718].path,"ef_fragexplosion",17);
    CopyMemoryFromBtoAForNBytes(EDefs[719].path,"ef_line_sparqbeam",18);
    CopyMemoryFromBtoAForNBytes(EDefs[720].path,"ef_mist",8);
    CopyMemoryFromBtoAForNBytes(EDefs[721].path,"ef_particle_bloodspurtsmall",28);
    CopyMemoryFromBtoAForNBytes(EDefs[722].path,"ef_particle_bloodspurtsmallgreen",33);
    CopyMemoryFromBtoAForNBytes(EDefs[723].path,"ef_particle_bloodspurtsmallyellow",34);
    CopyMemoryFromBtoAForNBytes(EDefs[724].path,"ef_particle_bloodspurttiny",27);
    CopyMemoryFromBtoAForNBytes(EDefs[725].path,"ef_particle_camerahit",22);
    CopyMemoryFromBtoAForNBytes(EDefs[726].path,"ef_particle_darthit",20);
    CopyMemoryFromBtoAForNBytes(EDefs[727].path,"ef_particle_sec2muzburst",25);
    CopyMemoryFromBtoAForNBytes(EDefs[728].path,"ef_particle_sec2rotmuzburst",28);
    CopyMemoryFromBtoAForNBytes(EDefs[729].path,"ef_particle_sparksmall",23);
    CopyMemoryFromBtoAForNBytes(EDefs[730].path,"ef_particle_sparksmallblue",27);
    CopyMemoryFromBtoAForNBytes(EDefs[731].path,"ef_particle_sparqhit",21);
    CopyMemoryFromBtoAForNBytes(EDefs[732].path,"ef_sparkspits",14);
    CopyMemoryFromBtoAForNBytes(EDefs[733].path,"ef_spraydrips",14);
    CopyMemoryFromBtoAForNBytes(EDefs[734].path,"ef_steam",9);
    CopyMemoryFromBtoAForNBytes(EDefs[735].path,"env_sparksmall",15);
    CopyMemoryFromBtoAForNBytes(EDefs[736].path,"TargetIDInstance",17);
    CopyMemoryFromBtoAForNBytes(EDefs[737].path,"prop_papers01",14); EDefs[737].modelIndex=484;  EDefs[737].texIndex=580; 
    CopyMemoryFromBtoAForNBytes(EDefs[738].path,"prop_papers02",14); EDefs[738].modelIndex=485;  EDefs[738].texIndex=580; 
    CopyMemoryFromBtoAForNBytes(EDefs[739].path,"ef_particle_blasterhit",23);
    CopyMemoryFromBtoAForNBytes(EDefs[740].path,"ef_particle_ionhit",19);
    /*741 us_puz_panel_blue_wire*/       EDefs[741].modelIndex=0;  EDefs[741].texIndex=604;  EDefs[741].numclips=3;  EDefs[741].animationNum=43; 
    /*742 us_puz_panel_brown_wire*/      EDefs[742].modelIndex=0;  EDefs[742].texIndex=631;  EDefs[742].numclips=3;  EDefs[742].animationNum=43; 
    /*743 us_puz_panel_gray_wire*/       EDefs[743].modelIndex=0;  EDefs[743].texIndex=634;  EDefs[743].numclips=3;  EDefs[743].animationNum=43; 
    /*744 us_puz_panel_red_wire*/        EDefs[744].modelIndex=0;  EDefs[744].texIndex=625;  EDefs[744].numclips=3;  EDefs[744].animationNum=43; 
    /*745 us_puz_panel_teal_wire*/       EDefs[745].modelIndex=0;  EDefs[745].texIndex=601;  EDefs[745].numclips=3;  EDefs[745].animationNum=43; 
    /*746 weapon_grenadeenergmine_live*/ EDefs[746].modelIndex=169;  EDefs[746].texIndex=852; 
    /*747 decal_logo_storage*/           EDefs[747].modelIndex=77;  EDefs[747].texIndex=169;  EDefs[747].shadows=false; 
    CopyMemoryFromBtoAForNBytes(EDefs[748].path,"748 light_animated",15);
    CopyMemoryFromBtoAForNBytes(EDefs[749].path,"749 generic_transform",18);
    CopyMemoryFromBtoAForNBytes(EDefs[750].path,"750 chunk_crate_impenetrable2",26); EDefs[750].modelIndex=61;  EDefs[750].texIndex=147; 
    CopyMemoryFromBtoAForNBytes(EDefs[751].path,"751 chunk_crate_impenetrable3",26); EDefs[751].modelIndex=61;  EDefs[751].texIndex=148; 
    CopyMemoryFromBtoAForNBytes(EDefs[752].path,"752 chunk_crate_impenetrable4",26); EDefs[752].modelIndex=61;  EDefs[752].texIndex=149; 
    CopyMemoryFromBtoAForNBytes(EDefs[753].path,"753 npc_sec3_bot",13); EDefs[753].modelIndex=681;  EDefs[753].texIndex=553; 
    CopyMemoryFromBtoAForNBytes(EDefs[754].path,"754 prop_shieldgenerator",21); EDefs[754].modelIndex=143;  EDefs[754].texIndex=316; 

    CopyMemoryFromBtoAForNBytes(EDefs[756].path,"756 ef_particle_leafburst",22);
    CopyMemoryFromBtoAForNBytes(EDefs[757].path,"757 ef_particle_mutationburst",26);
    CopyMemoryFromBtoAForNBytes(EDefs[758].path,"758 ef_particle_graytationburst",28);

    /*767 player*/
    for (i32 i = 0; i < MAX_ENTITIES; i++) {
        if (EDefs[i].index == U16_MAX) continue;
        
        if (!EDefs[i].layer) EDefs[i].layer = Layer_Default;
        flag_set(&EDefs[i].entflags,ENTFLAG_ACTIVE,true); // Individual value setting to allow mods to set custom starting flags themselves. (or here too if they want, tis your oyster).
        flag_set(&EDefs[i].entflags,ENTFLAG_RIGIDBODY,ConstIndexIsDynamicObject(EDefs[i].index));
        if (EDefs[i].cardchunk) {
            EDefs[i].lodIndex = GEOMETRY_LOD_CARD_MODEL_IDX;
            EDefs[i].collider = COLLIDER_TYPE_BOX;
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
    e->position = pos;
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
    flag_set(&e->entflags,ENTFLAG_RIGIDBODY,EDefs[entIdx].entflags & ENTFLAG_RIGIDBODY);
    flag_set(&e->entflags,ENTFLAG_NO_SHADOWS, EDefs[entIdx].entflags & ENTFLAG_NO_SHADOWS);
    e->collider = EDefs[entIdx].collider; e->colliderCenter = EDefs[entIdx].colliderCenter; e->colliderSize = EDefs[entIdx].colliderSize;
    e->mass = EDefs[entIdx].mass > 0.0f ? EDefs[entIdx].mass : 1.0f; e->angularDrag = EDefs[entIdx].angularDrag > 0.0f ? EDefs[entIdx].angularDrag : 0.05f;    
    Eng_Global->instances[i].lockedMessageLingdex = EDefs[entIdx].lockedMessageLingdex;
    Eng_Global->dirtyInstances[i] = true;
    Eng_Global->loadedInstances++;
    return i;
}

void DeleteInstance(u16 i) {
    if (i <= PLAYER2 || i >= Eng_Global->loadedInstances) return; // Don't delete null ent, player 1, nor player 2 or already empty slots.
    
    u16 endInstance = vmax(vmin(INSTANCE_COUNT - 1, Eng_Global->loadedInstances - 1),START_INDEX_LEVEL_INSTANCES);
//     for (;i<endInstance;++i) Eng_Global->instances[i] = Eng_Global->instances[i + 1]; // Shift the entire list down, overwriting the entity we're deleting at starting i
    for (;i<endInstance;++i) CopyMemoryFromBtoAForNBytes(&Eng_Global->instances[i],&Eng_Global->instances[i+1],sizeof(Entity));
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

    for (u16 idx = START_INDEX_LEVEL_INSTANCES; idx < INSTANCE_COUNT; idx++) { InitializeEntity(&Eng_Global->instances[idx]); Eng_Global->dirtyInstances[idx] = true; }
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
        CopyMemoryFromBtoAForNBytes(firstKeyCheck,line,10); firstKeyCheck[10] = '\0'; lineNum++;
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
                else if (StringsEqual(trimmed_key,"go.activeSelf"))   { activeStateRead = true; flag_set(&inst->entflags, ENTFLAG_ACTIVE, parse_bool(trimmed_value,initialLine,lineNum)); }
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
                else if (StringsEqual(trimmed_key,"locked"))          flag_set(&inst->entflags, ENTFLAG_LOCKED, parse_bool(trimmed_value,initialLine,lineNum));
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
                else if (StringsEqual(trimmed_key,"accessCardUsedByPlayer")) inst->accessCardUsedByPlayer = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"timeBeforeLasersOn")) inst->timeBeforeLasersOn = parse_float(trimmed_value, initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"toggleLasers"))    inst->toggleLasers = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"targettingOnlyUnlocks")) inst->targettingOnlyUnlocks = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"changeLayerOnOpenClose")) inst->changeLayerOnOpenClose = parse_bool(trimmed_value,initialLine,lineNum);
                else if (StringsEqual(trimmed_key,"useFinished"))     inst->useFinished = parse_float(trimmed_value, initialLine, lineNum) + Eng_Global->pauseRelativeTime;
                else if (StringsEqual(trimmed_key,"waitBeforeClose")) inst->waitBeforeClose = parse_float(trimmed_value,initialLine, lineNum) + Eng_Global->pauseRelativeTime;
                else if (StringsEqual(trimmed_key,"lasersFinished"))  inst->lasersFinished = parse_float(trimmed_value,initialLine, lineNum) + Eng_Global->pauseRelativeTime;
                else if (StringsEqual(trimmed_key,"changeMatOnActive")) inst->changeTexOnActive = parse_bool(trimmed_value,initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"blinkWhenActive")) inst->blinkTexOnActive = parse_bool(trimmed_value,initialLine, lineNum);
                else if (StringsEqual(trimmed_key,"doorOpen"))        flag_setu64(&inst->ioflags,TARG_IOFLAGS_DOOROPEN,parse_bool(trimmed_value, initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorOpenIfUnlocked")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_DOOROPENIFUNLOCKED, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorClose"))       flag_setu64(&inst->ioflags,TARG_IOFLAGS_DOORCLOSE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorLock"))        flag_setu64(&inst->ioflags,TARG_IOFLAGS_DOORLOCK,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorUnlock"))      flag_setu64(&inst->ioflags,TARG_IOFLAGS_DOORUNLOCK,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"switchTrigger"))   flag_setu64(&inst->ioflags,TARG_IOFLAGS_SWITCHTRIGGER,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"tripTrigger"))     flag_setu64(&inst->ioflags,TARG_IOFLAGS_TRIPTRIGGER,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"forceBridgeActivate")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_FBRIDGE_ACTIVATE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"forceBridgeDeactivate")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_FBRIDGE_DEACTIVATE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"forceBridgeToggle")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_FBRIDGE_TOGGLE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"gravityLiftToggle")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_GRAVLIFT_TOGGLE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"textureChangeToggle")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_TEXTURE_CHG_TOGGLE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"lightOn"))         flag_setu64(&inst->ioflags,TARG_IOFLAGS_LIGHT_ON,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"lightOff"))        flag_setu64(&inst->ioflags,TARG_IOFLAGS_LIGHT_OFF,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"lightToggle"))     flag_setu64(&inst->ioflags,TARG_IOFLAGS_LIGHT_TOGGLE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"funcwallMove"))    flag_setu64(&inst->ioflags,TARG_IOFLAGS_FUNCWALL_MOVE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"missionBitOn"))    flag_setu64(&inst->ioflags,TARG_IOFLAGS_MISSION_BIT_ON,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"missionBitOff"))   flag_setu64(&inst->ioflags,TARG_IOFLAGS_MISSION_BIT_OFF,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"missionBitToggle")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_MISSION_BIT_TOGGLE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"sendEmail"))       flag_setu64(&inst->ioflags,TARG_IOFLAGS_SEND_EMAIL,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"switchLockToggle")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_SWITCH_LOCK_TOGGLE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"spawnerActivate")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_SPAWNER_ACTIVATE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"spawnerActivateAlerted")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_SPAWNER_ACTALERTED, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"cyborgConversionToggle")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_CYBORG_CONV_TOGGLE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"GOSetActive"))     flag_setu64(&inst->ioflags,TARG_IOFLAGS_INST_ACTIVATE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"GOSetDeactive"))   flag_setu64(&inst->ioflags,TARG_IOFLAGS_INST_DEACTIVATE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"GOToggleActive"))  flag_setu64(&inst->ioflags,TARG_IOFLAGS_INST_TOGGLE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"disableThisGOOnAwake")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_DISABLE_ON_AWAKE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"playSoundOnce"))   flag_setu64(&inst->ioflags,TARG_IOFLAGS_PLAY_SOUND_ONCE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"stopSound"))       flag_setu64(&inst->ioflags,TARG_IOFLAGS_STOP_SOUND, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"sendSprintMessage")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_SEND_CENTERPRINT, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"radiationTreatment")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_RADIATION_TREATMNT, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"startFlashingMaterials")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_START_FLASHING_TEX, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"stopFlashingMaterials")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_STOP_FLASHING_TEX, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"unlockElevatorPad")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_UNLOCK_ELEVATORPAD, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"unlockKeycodePad")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_UNLOCK_KEYPAD, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"unlockPuzzlePad")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_UNLOCK_PUZPAD, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"screenShake"))     flag_setu64(&inst->ioflags,TARG_IOFLAGS_SCREENSHAKE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"awakeSleepingEnemy")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_AWAKE_SLEEPING_NPC, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"branchFlip"))      flag_setu64(&inst->ioflags,TARG_IOFLAGS_BRANCH_FLIP, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"branchFlipOnly"))  flag_setu64(&inst->ioflags,TARG_IOFLAGS_BRANCH_FLIPONLY, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorAccessCardOverrideToggle")) flag_setu64(&inst->ioflags, TARG_IOFLAGS_TOG_DORACESOVERIDE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"unlockSwitch"))    flag_setu64(&inst->ioflags,TARG_IOFLAGS_UNLOCK_SWITCH, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"lockElevatorPad")) flag_setu64(&inst->ioflags,TARG_IOFLAGS_LOCK_ELEVATORPAD, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorToggle"))      flag_setu64(&inst->ioflags,TARG_IOFLAGS_DOOR_TOGGLE, parse_bool(trimmed_value,initialLine,lineNum));
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
        if (!isLight && !activeStateRead) flag_set(&entsFromFile[entCount].entflags, ENTFLAG_ACTIVE, true); // Default active if not specified
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
        par->accessCardUsedByPlayer= src->accessCardUsedByPlayer;
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
        } else if (entIdx == 279) { // chunk_screen
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
