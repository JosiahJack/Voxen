// entity.c - Entity Definitions and Save Load System for levels and savegames
#include "common.h"
#include "lib.h" // LibC Replacements and Helpers
#define LINE_LEN_MAX 81920
Entity EDefs[MAX_ENTITIES];
V3 EDefscolliderCenter[MAX_ENTITIES]; // Offset relative to .position's global worldspace xyz location
V3 EDefscolliderSize[MAX_ENTITIES]; // x,y,z for Box, x for Sphere radius, else x, y, z for Capsule radius, height, and direction (0.0f = X-Axis, 1.0f = Y-Axis, 2.0f = Z-Axis respectively, default 1.0f)
ColliderType/*u8*/ EDefscollider[MAX_ENTITIES];
u32 EDefslayer[MAX_ENTITIES];
float EDefsmass[MAX_ENTITIES];
float EDefsdynamicFriction[MAX_ENTITIES];
float EDefsstaticFriction[MAX_ENTITIES];
float EDefsbounciness[MAX_ENTITIES];
float EDefsangularDrag[MAX_ENTITIES];
float EDefsgravity[MAX_ENTITIES];
#define GEOMETRY_LOD_CARD_MODEL_IDX 178
INLINE i32 parse_numberi32(const char* str, const char* line, u32 lineNum) { if(str == 0 || *str == '\0'){DualLogError("Invalid from line[%d]: %s\n",lineNum+1,line); return 0;} while(cEmpty((char)*str)){str++;} bool negative=false; if(*str == '+'){str++;}else if(*str == '-'){negative=true; str++;} i64 result=0; while(*str >= '0' && *str <= '9'){result=result*10L + (*str-'0'); str++;} return (i32)(negative ? -result : result); }
INLINE i16 parse_numberi16(const char* str, const char* line, u32 lineNum) { i32 retval = parse_numberi32(str, line, lineNum); if (retval < -32768 || retval > 32767) { DualLogError("Value %d out of range for i16 from line[%d]: %s\n", retval, lineNum+1, line); return 0; } return (i16)retval; }
INLINE i8 parse_numberi8(const char* str, const char* line, u32 lineNum) { i32 retval = parse_numberi32(str, line, lineNum); if (retval < -128 || retval > 127) { DualLogError("Value %d out of range for i8 from line[%d]: %s\n", retval, lineNum+1, line); return 0; } return (i8)retval; }
float parse_float(const char* str, const char* line, u32 lineNum) {
    if (str == 0 || *str == '\0') { DualLogError("Invalid blank string from line[%d]: %s\n", lineNum+1, line); return 0.0f; }
    while (cEmpty(*str)) str++;
    bool negative = false;
    if (*str == '-') { negative = true; str++; }
    else if (*str == '+') { str++; }
    double value = 0.0;
    bool has_digit = false;
    while (*str >= '0' && *str <= '9') { value = value * 10.0 + (*str - '0'); str++; has_digit = true; } // Integer part
    if (*str == '.') { // Decimal part
        str++;
        double frac = 0.0;
        double place = 0.1;
        while (*str >= '0' && *str <= '9') { frac += (*str - '0') * place; place *= 0.1; str++; has_digit = true; }
        value += frac;
    }
    if (!has_digit) return 0.0f;
    if (negative) value = -value;
    return (float)value;
}

void ModEDefsInitAfterLoad() { // Global conditions for all entities.  No sense inflating the table data in entity.c
    mset(EDefs,0,sizeof(Entity)); 
    for (int i=0;i<768;++i) { EDefs[i].index = i; EDefs[i].modelIndex = MAX_MDLS; EDefs[i].lodIndex = MAX_MDLS; }
    
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
    for (i32 i = 0; i < MAX_ENTITIES; i++) {        
        if (!EDefslayer[i]) EDefslayer[i] = L_Default;
        flag_set(&EDefs[i].entflags,EF_ACTIVE,true); // Individual value setting to allow mods to set custom starting flags themselves. (or here too if they want, tis your oyster).
        flag_set(&EDefs[i].entflags,EF_RIGIDBODY,IdxIsDynamicObject(EDefs[i].index));
        if (EDefs[i].cardchunk) {
            EDefs[i].lodIndex = GEOMETRY_LOD_CARD_MODEL_IDX;
            EDefscollider[i] = COLTYPE_BOX;
            EDefscolliderCenter[i].y = 1.32f;
            EDefscolliderSize[i] = (V3){2.56f,0.08f,2.56f};
        }
        
        EDefs[i].currentFrameFinished = World.pauseRelativeTime + 0.1;
        if (IdxIsButtonSwitch(EDefs[i].index)) { EDefs[i].lockedMessageLingdex = 193; EDefs[i].tickTime = 1.5; } // ButtonSwitch
    } // Handle generics up front such that all below can override it.
    // Note that designated initializer method, e.g. { .modelIndex = 178, .texIndex = 0 } method will cause compiler to add the = 0 assignment for every field ballooning binary size to 11mb!  So we do this.  Straightforward and simple:
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
    /*123 chunk_blockerflightbay*/     EDefs[123].modelIndex=178; EDefs[123].normIndex=160; EDefs[123].texIndex=1230; EDefs[123].specIndex=1242; EDefscollider[123]=COLTYPE_BOX; EDefscolliderCenter[123].y=1.44f; EDefscolliderSize[123]=(V3){2.56f,0.32f,2.56f}; EDefs[123].colMeshIndex=U16_MAX;
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
    /*149 chunk_med1_1*/               EDefs[149].modelIndex=249; EDefs[149].texIndex=486; EDefs[149].specIndex=1256; EDefs[149].normIndex=1255; EDefscollider[149]=COLTYPE_MSH;
    /*150 chunk_med1_1_half_top*/      EDefs[150].modelIndex=250; EDefs[150].texIndex=486; EDefs[150].specIndex=1256; EDefs[150].normIndex=1255; EDefscollider[150]=COLTYPE_MSH;
    /*151 chunk_med1_1_slice128high*/  EDefs[151].modelIndex=251; EDefs[151].texIndex=486; EDefs[151].specIndex=1256; EDefs[151].normIndex=1255; EDefscollider[151]=COLTYPE_MSH;
    /*152 chunk_med1_1_slice192RH*/    EDefs[152].modelIndex=252; EDefs[152].texIndex=486; EDefs[152].specIndex=1256; EDefs[152].normIndex=1255; EDefscollider[152]=COLTYPE_MSH;
    /*153 chunk_med1_1_slice256*/      EDefs[153].modelIndex=253; EDefs[153].texIndex=486; EDefs[153].specIndex=1256; EDefs[153].normIndex=1255; EDefscollider[153]=COLTYPE_MSH;
    /*154 chunk_med1_1d*/              EDefs[154].modelIndex=248; EDefs[154].texIndex=485; EDefs[154].glowIndex=484; EDefs[154].specIndex=1236; EDefs[154].normIndex=1255; EDefscollider[154]=COLTYPE_MSH;
    /*155 chunk_med1_2*/               EDefs[155].modelIndex=255; EDefs[155].texIndex=489; EDefs[155].glowIndex=488; EDefs[155].specIndex=1256;
    /*156 chunk_med1_2d*/              EDefs[156].modelIndex=254; EDefs[156].texIndex=487; EDefs[156].specIndex=1256;
    /*157 chunk_med1_3*/               EDefs[157].modelIndex=257; EDefs[157].texIndex=493; EDefs[157].glowIndex=492; EDefs[157].specIndex=1256;
    /*158 chunk_med1_3d*/              EDefs[158].modelIndex=256; EDefs[158].texIndex=491; EDefs[158].glowIndex=490; EDefs[158].specIndex=1256;
    /*159 chunk_med1_4*/               EDefs[159].modelIndex=258; EDefs[159].texIndex=494; EDefs[159].specIndex=1256;
    /*160 chunk_med1_5*/               EDefs[160].modelIndex=669; EDefs[160].texIndex=495; EDefs[160].specIndex=1256;
    /*161 chunk_med1_6*/               EDefs[161].modelIndex=259; EDefs[161].texIndex=496; EDefs[161].normIndex=509; EDefs[161].specIndex=1256;
    /*162 chunk_med1_7*/               EDefs[162].modelIndex=262; EDefs[162].texIndex=499; EDefs[162].specIndex=1268; EDefs[162].normIndex=498; EDefscollider[162]=COLTYPE_MSH;
    /*163 chunk_med1_7_slice14_64*/    EDefs[163].modelIndex=263; EDefs[163].texIndex=499; EDefs[163].specIndex=1268; EDefs[163].normIndex=1254; EDefscollider[163]=COLTYPE_MSH;
    /*164 chunk_med1_7_slice45_320lh*/ EDefs[164].modelIndex=264; EDefs[164].texIndex=499; EDefs[164].specIndex=1268; EDefs[164].normIndex=1254; EDefscollider[164]=COLTYPE_MSH;
    /*165 chunk_med1_7_slice45_320rh*/ EDefs[165].modelIndex=265; EDefs[165].texIndex=499; EDefs[165].specIndex=1268; EDefs[165].normIndex=1254; EDefscollider[165]=COLTYPE_MSH;
    /*166 chunk_med1_7_slice96high*/   EDefs[166].modelIndex=266; EDefs[166].texIndex=499; EDefs[166].specIndex=1268; EDefs[166].normIndex=1254; EDefscollider[166]=COLTYPE_MSH;
    /*167 chunk_med1_7d*/              EDefs[167].modelIndex=260; EDefs[167].texIndex=497; EDefs[167].specIndex=1269; EDefs[167].normIndex=1270; EDefscollider[167]=COLTYPE_MSH;
    /*168 chunk_med1_7d_slice128*/     EDefs[168].modelIndex=261; EDefs[168].texIndex=497; EDefs[168].specIndex=1269; EDefs[168].normIndex=1270; EDefscollider[168]=COLTYPE_MSH;
    /*169 chunk_med1_8*/               EDefs[169].modelIndex=268; EDefs[169].texIndex=503; EDefs[169].normIndex=502; EDefs[169].specIndex=1242; EDefscollider[169]=COLTYPE_MSH;
    /*170 chunk_med1_8d*/              EDefs[170].modelIndex=267; EDefs[170].texIndex=501; EDefs[170].normIndex=163; EDefs[170].specIndex=1242; EDefscollider[170]=COLTYPE_MSH;
    /*171 chunk_med1_9*/               EDefs[171].modelIndex=278; EDefs[171].texIndex=507; EDefs[171].normIndex=506; EDefs[171].specIndex=1267; EDefscollider[171]=COLTYPE_MSH;
    /*172 unused*/
    /*173 unused*/
    /*174 chunk_med1_9d*/              EDefs[174].modelIndex=269; EDefs[174].texIndex=505; EDefs[174].normIndex=504; EDefs[174].specIndex=1267; EDefscollider[174]=COLTYPE_MSH;
    /*175 unused*/
    /*176 chunk_med1_9d_ofs112_90*/    EDefs[176].modelIndex=270; EDefs[176].texIndex=505; EDefs[176].normIndex=504; EDefs[176].specIndex=1267; EDefscollider[176]=COLTYPE_MSH;
    /*177 chunk_med1_9d_ofs144_90*/    EDefs[177].modelIndex=272; EDefs[177].texIndex=505; EDefs[177].normIndex=504; EDefs[177].specIndex=1267; EDefscollider[177]=COLTYPE_MSH;
    /*178 chunk_med2_1*/               EDefs[178].modelIndex=280; EDefs[178].texIndex=513; EDefs[178].specIndex=1254; EDefs[178].glowIndex=511; EDefs[178].normIndex=512;
    /*179 chunk_med2_1_slice32RH*/     EDefs[179].modelIndex=281; EDefs[179].texIndex=513; EDefs[179].normIndex=512; EDefs[179].specIndex=1254;
    /*180 chunk_med2_1d*/              EDefs[180].modelIndex=279; EDefs[180].glowIndex=508; EDefs[180].texIndex=510; EDefs[180].specIndex=1254;
    /*181 chunk_med2_2*/               EDefs[181].modelIndex=283; EDefs[181].texIndex=517; EDefs[181].glowIndex=516; EDefs[181].specIndex=1242;
    /*182 chunk_med2_2_half_bottom*/   EDefs[182].modelIndex=284; EDefs[182].texIndex=517; EDefs[182].glowIndex=516; EDefs[182].specIndex=1242;
    /*183 chunk_med2_2d*/              EDefs[183].modelIndex=282; EDefs[183].texIndex=515; EDefs[183].glowIndex=516; EDefs[183].specIndex=1242;
    /*184 chunk_med2_3*/               EDefs[184].modelIndex=286; EDefs[184].texIndex=521; EDefs[184].glowIndex=520; EDefs[184].specIndex=1242;
    /*185 chunk_med2_3d*/              EDefs[185].modelIndex=285; EDefs[185].texIndex=519; EDefs[185].glowIndex=518; EDefs[185].specIndex=1242;
    /*186 chunk_med2_4*/               EDefs[186].modelIndex=287; EDefs[186].texIndex=523; EDefs[186].glowIndex=522; EDefs[186].specIndex=1242;
    /*187 chunk_med2_5*/               EDefs[187].modelIndex=288; EDefs[187].texIndex=527; EDefs[187].glowIndex=526; EDefs[187].specIndex=539; EDefscollider[187]=COLTYPE_BOX; EDefscolliderCenter[187].y=1.44f; EDefscolliderSize[187]=(V3){2.56f,0.32f,2.56f}; EDefs[187].colMeshIndex=U16_MAX;
    /*188 chunk_med2_6*/               EDefs[188].modelIndex=289; EDefs[188].texIndex=528; EDefs[188].specIndex=1271;                          EDefscollider[188]=COLTYPE_MSH; EDefs[188].colMeshIndex=289;
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
    for (int i=307;i<=404;++i) { EDefsangularDrag[i]=0.05f; EDefsdynamicFriction[i]=0.5f; EDefsstaticFriction[i]=0.6f; EDefsmass[i]=1.0f; } // Item
    /*307 item_paper_wad*/             EDefs[307].modelIndex=487; EDefs[307].texIndex=1250; EDefscollider[307]= COLTYPE_SPH; EDefscolliderCenter[307]=(V3){-0.001254f,-0.001190498f,0.006335999f}; EDefscolliderSize[307].x=0.0451f; EDefsmass[307]=0.06f;
    /*308 item_warecasing*/            EDefs[308].modelIndex=637; EDefs[308].texIndex=1251; EDefsmass[308]=0.8f;
    /*309 item_beaker*/                EDefs[309].modelIndex=14;  EDefscollider[309]=COLTYPE_CVX; EDefs[309].colMeshIndex=682; EDefs[309].texIndex=36; EDefs[309].specIndex=1242;  EDefsmass[309]=0.28f; EDefsdynamicFriction[309]=0.1f; EDefsstaticFriction[309]=0.2f;
    /*310 item_beverage*/              EDefs[310].modelIndex=18;  EDefscollider[310]=COLTYPE_CVX; EDefs[310].colMeshIndex=683; EDefs[310].texIndex=37; EDefsmass[310]=0.12f;
    /*311 item_skull*/                 EDefs[311].modelIndex=593; EDefsmass[311]=0.451f;
    /*312 item_arm*/                   EDefs[312].modelIndex=7;   EDefs[312].texIndex=28; EDefscollider[312]=COLTYPE_CVX; EDefs[312].colMeshIndex=678;
    /*313 item_audiolog*/              EDefs[313].modelIndex=11;  EDefscollider[313]=COLTYPE_CVX; EDefs[313].colMeshIndex=679; EDefs[313].texIndex=52; EDefs[313].glowIndex=80;  EDefsmass[313]=0.2f;
    /*314 weapon_grenadefrag*/         EDefs[314].modelIndex=182;
    /*315 weapon_grenadeconc*/         EDefs[315].modelIndex=165;
    /*316 weapon_grenadeemp*/          EDefs[316].modelIndex=168;
    /*317 weapon_grenadeearth*/        EDefs[317].modelIndex=181;
    /*318 weapon_grenademine*/         EDefs[318].modelIndex=184;
    /*319 weapon_grenadenitro*/        EDefs[319].modelIndex=185;
    /*320 weapon_grenadegas*/          EDefs[320].modelIndex=183;
    /*321 item_patch_berserk*/         EDefs[321].modelIndex=488; EDefs[321].texIndex=590; EDefscollider[321]=COLTYPE_CVX; EDefs[321].colMeshIndex=491; EDefsmass[321]=0.12f;
    /*322 item_patch_detox*/           EDefs[322].modelIndex=488; EDefs[322].texIndex=591; EDefscollider[322]=COLTYPE_CVX; EDefs[322].colMeshIndex=491; EDefsmass[322]=0.12f;
    /*323 item_patch_genius*/          EDefs[323].modelIndex=488; EDefs[323].texIndex=592; EDefscollider[323]=COLTYPE_CVX; EDefs[323].colMeshIndex=491; EDefsmass[323]=0.12f;
    /*324 item_patch_medi*/            EDefs[324].modelIndex=488; EDefs[324].texIndex=600; EDefscollider[324]=COLTYPE_CVX; EDefs[324].colMeshIndex=491; EDefsmass[324]=0.12f;
    /*325 item_patch_reflex*/          EDefs[325].modelIndex=488; EDefs[325].texIndex=641; EDefscollider[325]=COLTYPE_CVX; EDefs[325].colMeshIndex=491; EDefsmass[325]=0.12f;
    /*326 item_patch_sight*/           EDefs[326].modelIndex=488; EDefs[326].texIndex=646; EDefscollider[326]=COLTYPE_CVX; EDefs[326].colMeshIndex=491; EDefsmass[326]=0.12f;
    /*327 item_patch_staminup*/        EDefs[327].modelIndex=488; EDefs[327].texIndex=647; EDefscollider[327]=COLTYPE_CVX; EDefs[327].colMeshIndex=491; EDefsmass[327]=0.12f;
    /*328 item_hw_system*/             EDefs[328].modelIndex=207; EDefs[328].texIndex=405; EDefs[328].glowIndex=404; EDefsmass[328]=0.17f;
    /*329 item_hw_navunit*/            EDefs[329].modelIndex=204; EDefs[329].texIndex=1258;EDefs[329].glowIndex=1259; EDefscollider[329]=COLTYPE_CVX; EDefs[329].colMeshIndex=696; EDefsmass[329]=0.1f;
    /*330 item_hw_ereader*/            EDefs[330].modelIndex=200; EDefscollider[330]=COLTYPE_CVX; EDefs[330].colMeshIndex=692; EDefsmass[330]=0.12f;
    /*331 item_hw_sensaround*/         EDefs[331].modelIndex=205; EDefscollider[331]=COLTYPE_CVX; EDefs[331].colMeshIndex=697; EDefsmass[331]=0.12f;
    /*332 item_hw_targetid*/           EDefs[332].modelIndex=208; EDefsmass[332]=0.08f;
    /*333 item_hw_shield*/             EDefs[333].modelIndex=206; EDefsmass[333]=0.14f;
    /*334 item_hw_bio*/                EDefs[334].modelIndex=197; EDefscollider[334]=COLTYPE_CVX; EDefs[334].colMeshIndex=689; EDefsmass[334]=0.1f;
    /*335 item_hw_lantern*/            EDefs[335].modelIndex=203; EDefscollider[335]=COLTYPE_CVX; EDefs[335].colMeshIndex=695; EDefsmass[335]=0.11f;
    /*336 item_hw_envirosuit*/         EDefs[336].modelIndex=199; EDefscollider[336]=COLTYPE_CVX; EDefs[336].colMeshIndex=691; EDefsmass[336]=0.451f;
    /*337 item_hw_booster*/            EDefs[337].modelIndex=198; EDefscollider[337]=COLTYPE_CVX; EDefs[337].colMeshIndex=690; EDefsmass[337]=0.16f;
    /*338 item_hw_jumpjets*/           EDefs[338].modelIndex=202; EDefscollider[338]=COLTYPE_CVX; EDefs[338].colMeshIndex=694; EDefsmass[338]=0.32f;
    /*339 item_hw_infrared*/           EDefs[339].modelIndex=201; EDefscollider[339]=COLTYPE_CVX; EDefs[339].colMeshIndex=693; EDefsmass[339]=0.1f;
    /*340 item_fireextinguisher*/      EDefs[340].modelIndex=144; EDefscollider[340]=COLTYPE_CVX; EDefs[340].colMeshIndex=684; EDefsmass[340]=1.3f;
    /*341 item_access_card_admin*/     EDefs[341].modelIndex=0;   EDefs[341].texIndex=9; EDefs[341].glowIndex=82; EDefscollider[341]=COLTYPE_CVX; EDefs[341].colMeshIndex=672; EDefsmass[341]=0.2f;
    /*342 item_workerhelmet*/          EDefs[342].modelIndex=648; EDefsmass[342]=1.2f;
    /*343 weapon_mk3*/                 EDefs[343].modelIndex=646; EDefsmass[343]=0.75f;
    /*344 weapon_blaster*/             EDefs[344].modelIndex=638; EDefsmass[344]=0.5f;
    /*345 weapon_dartgun*/             EDefs[345].modelIndex=640; EDefs[345].texIndex=876; EDefsmass[345]=0.3f;
    /*346 weapon_flechette*/           EDefs[346].modelIndex=642; EDefsmass[346]=0.4f;
    /*347 weapon_ionrifle*/            EDefs[347].modelIndex=643; EDefsmass[347]=0.8f;
    /*348 weapon_rapier*/              EDefs[348].modelIndex=653; EDefsmass[348]=0.3f;
    /*349 weapon_pipe*/                EDefs[349].modelIndex=649; EDefs[349].texIndex=887;  EDefsmass[349]=0.85f;
    /*350 weapon_magnum*/              EDefs[350].modelIndex=644; EDefsmass[350]=0.6f;
    /*351 weapon_magpulse*/            EDefs[351].modelIndex=645; EDefsmass[351]=0.65f;
    /*352 weapon_pistol*/              EDefs[352].modelIndex=650; EDefs[352].texIndex=878;  EDefsmass[352]=0.3f;
    /*353 weapon_plasma*/              EDefs[353].modelIndex=651; EDefsmass[353]=1.2f;
    /*354 weapon_railgun*/             EDefs[354].modelIndex=652;
    /*355 weapon_riotgun*/             EDefs[355].modelIndex=654; EDefsmass[355]=0.55f;
    /*356 weapon_skorpion*/            EDefs[356].modelIndex=655; EDefsmass[356]=1.3f;
    /*357 weapon_sparqbeam*/           EDefs[357].modelIndex=656; EDefsmass[357]=0.3f;
    /*358 weapon_stungun*/             EDefs[358].modelIndex=657; EDefsmass[358]=0.3f;
    /*359 item_battery*/               EDefs[359].modelIndex=13;  EDefscollider[359]=COLTYPE_CVX;  EDefs[359].colMeshIndex=680;  EDefsmass[359]=0.3f;
    /*360 item_battery_icad*/          EDefs[360].modelIndex=13;  EDefscollider[360]=COLTYPE_CVX;  EDefs[360].colMeshIndex=680;  EDefsmass[360]=0.35f;
    /*361 item_logic_probe*/           EDefs[361].modelIndex=217; EDefs[361].texIndex=427;  EDefsmass[361]=0.15f;
    /*362 item_healthkit*/             EDefs[362].modelIndex=196; EDefscollider[362]=COLTYPE_CVX;  EDefs[362].colMeshIndex=688;  EDefsmass[362]=0.25f;
    /*363 item_plastique*/             EDefs[363].modelIndex=492; EDefsmass[363]=1.4f;
    /*364 item_chipset_interfacedemod*/EDefs[364].modelIndex=45;  EDefscollider[364]=COLTYPE_BOX;  EDefscolliderCenter[364]=(V3){0.003744498f,0.0001704991f,0.03192701f};  EDefscolliderSize[364]=(V3){0.459303f,0.3412231f,0.06385402f};  EDefs[364].colMeshIndex=U16_MAX;  EDefsmass[364]=0.3f;
    /*365 item_flask*/                 EDefs[365].modelIndex=145; EDefscollider[365]=COLTYPE_CVX;  EDefs[365].colMeshIndex=685;  EDefs[365].texIndex=36;  EDefs[365].specIndex=1242;  EDefsmass[365]=0.22f;
    /*366 item_chipset_bitflag*/       EDefs[366].modelIndex=45;  EDefscollider[366]=COLTYPE_BOX;  EDefscolliderCenter[366]=(V3){0.003744498f,0.0001704991f,0.03192701f};  EDefscolliderSize[366]=(V3){0.459303f,0.3412231f,0.06385402f};  EDefs[366].colMeshIndex=U16_MAX;  EDefsmass[366]=0.3f;
    /*367 item_ammo_rubber*/           EDefs[367].modelIndex=8;   EDefscollider[367]=COLTYPE_CVX;  EDefs[367].colMeshIndex=676;  EDefsmass[367]=0.25f;
    /*368 item_isotopex22*/            EDefs[368].modelIndex=209; EDefsmass[368]=1.2f;
    /*369 item_testtube*/              EDefs[369].modelIndex=622; EDefs[369].texIndex=36; EDefs[369].specIndex=1242; EDefscollider[369]=COLTYPE_CVX; EDefs[369].colMeshIndex=612; EDefsmass[369]=0.21f;
    /*370 weapon_grenadefrag_live*/    EDefs[370].modelIndex=182;
    /*371 item_chipset_isolinear*/     EDefs[371].modelIndex=46;  EDefscollider[371]=COLTYPE_BOX;  EDefscolliderCenter[371]=(V3){-0.0009825006f,-0.0129465f,0.0148115f};  EDefscolliderSize[371]=(V3){0.223635f,0.4175691f,0.02912301f};  EDefs[371].colMeshIndex=U16_MAX;  EDefsmass[371]=0.26f;
    /*372 weapon_grenadeconc_live*/    EDefs[372].modelIndex=165;
    /*373 item_ammo_needle*/           EDefs[373].modelIndex=4;   EDefs[373].texIndex=15; EDefscollider[373]=COLTYPE_BOX; EDefscolliderCenter[373]=(V3){-0.0004654949f,0.0004549972f,0.0244365f}; EDefscolliderSize[373]=(V3){0.131339f,0.1442801f,0.04838703f}; EDefs[373].colMeshIndex=U16_MAX; EDefsmass[373]=0.15f;
    /*374 item_ammo_tranq*/            EDefs[374].modelIndex=4;   EDefs[374].texIndex=27; EDefscollider[374]=COLTYPE_BOX; EDefscolliderCenter[374]=(V3){-0.0004654949f,0.0004549972f,0.0244365f}; EDefscolliderSize[374]=(V3){0.131339f,0.1442801f,0.04838703f}; EDefs[374].colMeshIndex=U16_MAX; EDefsmass[374]=0.15f;
    /*375 item_ammo_standard*/         EDefs[375].modelIndex=5;   EDefscollider[375]=COLTYPE_BOX; EDefscolliderCenter[375]=(V3){0.0001984993f,0.0f,0.02172501f};  EDefscolliderSize[375]=(V3){0.1209471f,0.2176701f,0.04345007f};  EDefs[375].colMeshIndex=U16_MAX;  EDefsmass[375]=0.2f;
    /*376 item_ammo_teflon*/           EDefs[376].modelIndex=5;   EDefscollider[376]=COLTYPE_BOX; EDefscolliderCenter[376]=(V3){0.0001984993f,0.0f,0.02172501f};  EDefscolliderSize[376]=(V3){0.1209471f,0.2176701f,0.04345007f};  EDefs[376].colMeshIndex=U16_MAX;  EDefsmass[376]=0.2f;
    /*377 item_ammo_hollow*/           EDefs[377].modelIndex=5;   EDefscollider[377]=COLTYPE_BOX; EDefscolliderCenter[377]=(V3){0.0002185023f,0.0f,0.02122951f};  EDefscolliderSize[377]=(V3){0.1423431f,0.2127061f,0.04245907f};  EDefs[377].colMeshIndex=U16_MAX;  EDefsmass[377]=0.2f;
    /*378 item_ammo_slug*/             EDefs[378].modelIndex=3;   EDefscollider[378]=COLTYPE_BOX; EDefscolliderCenter[378]=(V3){0.0002185023f,0.0f,0.02122951f};  EDefscolliderSize[378]=(V3){0.1423431f,0.2127061f,0.04245907f};  EDefs[378].colMeshIndex=U16_MAX;  EDefs[378].glowIndex=22;  EDefsmass[378]=0.2f;
    /*379 item_ammo_magnesium*/        EDefs[379].modelIndex=3;   EDefscollider[379]=COLTYPE_CVX; EDefs[379].colMeshIndex=673; EDefsmass[379]=0.35f;
    /*380 item_ammo_penetrator*/       EDefs[380].modelIndex=3;   EDefscollider[380]=COLTYPE_CVX; EDefs[380].colMeshIndex=673; EDefsmass[380]=0.35f;
    /*381 item_ammo_hornet*/           EDefs[381].modelIndex=1;   EDefscollider[381]=COLTYPE_CVX; EDefs[381].colMeshIndex=673; EDefsmass[381]=0.35f;
    /*382 item_ammo_splinter*/         EDefs[382].modelIndex=630; EDefscollider[382]=COLTYPE_CVX; EDefs[382].colMeshIndex=673; EDefsmass[382]=0.35f;
    /*383 item_ammo_rail*/             EDefs[383].modelIndex=6;   EDefscollider[383]=COLTYPE_CVX; EDefs[383].colMeshIndex=675; EDefsmass[383]=0.40f;
    /*384 item_ammo_slag*/             EDefs[384].modelIndex=9;   EDefscollider[384]=COLTYPE_CVX; EDefs[384].colMeshIndex=673; EDefsmass[384]=0.35f;
    /*385 item_ammo_slaglarge*/        EDefs[385].modelIndex=10;  EDefscollider[385]=COLTYPE_CVX; EDefs[385].colMeshIndex=677; EDefsmass[385]=0.40f;
    /*386 item_ammo_magcart*/          EDefs[386].modelIndex=2;   EDefscollider[386]=COLTYPE_CVX; EDefs[386].colMeshIndex=674; EDefsmass[386]=0.35f;
    /*387 weapon_grenadeemp_live*/     EDefs[387].modelIndex=168;
    /*388 item_access_card_std*/       EDefs[388].modelIndex=0;   EDefs[388].texIndex=79;  EDefs[388].glowIndex=867;  EDefscollider[388]=COLTYPE_CVX;  EDefs[388].colMeshIndex=672;  EDefsmass[388]=0.2f;
    /*389 weapon_grenadeearth_live*/   EDefs[389].modelIndex=181;
    /*390 item_access_card_group1*/    EDefs[390].modelIndex=0;   EDefs[390].texIndex=7;  EDefs[390].glowIndex=159;  EDefscollider[390]=COLTYPE_CVX;  EDefs[390].colMeshIndex=672;  EDefsmass[390]=0.2f;
    /*391 item_access_card_science*/   EDefs[391].modelIndex=0;   EDefs[391].texIndex=2;  EDefs[391].glowIndex=343;  EDefscollider[391]=COLTYPE_CVX;  EDefs[391].colMeshIndex=672;  EDefsmass[391]=0.2f;
    /*392 item_access_card_eng*/       EDefs[392].modelIndex=0;   EDefs[392].texIndex=3;  EDefs[392].glowIndex=81;  EDefscollider[392]=COLTYPE_CVX;  EDefs[392].colMeshIndex=672;  EDefsmass[392]=0.2f;
    /*393 item_access_card_groupB*/    EDefs[393].modelIndex=0;   EDefs[393].texIndex=7;  EDefs[393].glowIndex=159;  EDefscollider[393]=COLTYPE_CVX;  EDefs[393].colMeshIndex=672;  EDefsmass[393]=0.2f;
    /*394 item_access_card_security*/  EDefs[394].modelIndex=0;   EDefs[394].texIndex=10;  EDefs[394].glowIndex=344;  EDefscollider[394]=COLTYPE_CVX;  EDefs[394].colMeshIndex=672;  EDefsmass[394]=0.2f;
    /*395 item_access_card_per5diego*/ EDefs[395].modelIndex=0;   EDefs[395].texIndex=8;  EDefs[395].glowIndex=341;  EDefscollider[395]=COLTYPE_CVX;  EDefs[395].colMeshIndex=672;  EDefsmass[395]=0.2f;
    /*396 item_access_card_medi*/      EDefs[396].modelIndex=0;   EDefs[396].texIndex=1;  EDefs[396].glowIndex=161;  EDefscollider[396]=COLTYPE_CVX;  EDefs[396].colMeshIndex=672;  EDefsmass[396]=0.2f;
    /*397 item_access_card_group3*/    EDefs[397].modelIndex=0;   EDefs[397].texIndex=7;  EDefs[397].glowIndex=159;  EDefscollider[397]=COLTYPE_CVX;  EDefs[397].colMeshIndex=672;  EDefsmass[397]=0.2f;
    /*398 item_access_card_purple*/    EDefs[398].modelIndex=0;   EDefs[398].texIndex=5;  EDefs[398].glowIndex=342;  EDefscollider[398]=COLTYPE_CVX;  EDefs[398].colMeshIndex=672;  EDefsmass[398]=0.2f;
    /*399 item_head_male*/             EDefs[399].modelIndex=194; EDefscollider[399]=COLTYPE_CVX; EDefs[399].colMeshIndex=687; EDefsmass[399]=1.29f;
    /*400 item_head_female*/           EDefs[400].modelIndex=193; EDefscollider[400]=COLTYPE_CVX; EDefs[400].colMeshIndex=686; EDefsmass[400]=1.30f;
    /*401 item_severedhead*/           EDefs[401].modelIndex=590; EDefsmass[401]=1.28f;
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
    /*417 item_access_card_perdarcy*/  EDefs[417].modelIndex=0; EDefs[417].texIndex=8; EDefs[417].glowIndex=341; EDefscollider[417]=COLTYPE_CVX; EDefs[417].colMeshIndex=672; EDefsmass[417]=0.2f; EDefsangularDrag[417]=0.05f; EDefs[417].kinematic=false; EDefsdynamicFriction[417]=EDefsstaticFriction[417]=0.6f;
    for (int i=419;i<=447;++i) { EDefscollider[i]=COLTYPE_CAP; EDefscolliderSize[i].z=COLCAP_DIR_Y_F; EDefsstaticFriction[i]=1.0f; EDefsdynamicFriction[i]=0.15f; EDefs[i].kinematic=true; EDefsmass[i]=1.0f; EDefsangularDrag[i]=2.2f; } // NPCs
    /*419 npc_autobomb*/            EDefs[419].modelIndex=299; EDefs[419].texIndex=542; EDefscolliderCenter[419].y=0.42f; EDefscolliderCenter[419].z=0.01848752f;                   EDefscolliderSize[419]=(V3){0.42f,1.48f,COLCAP_DIR_Z_F};          EDefsangularDrag[419]=1.0f; EDefs[419].glowIndex=541;
    /*420 npc_cyborg_assassin*/     EDefs[420].modelIndex=306; EDefs[420].texIndex=545; EDefs[420].numclips= 8; EDefs[420].animationNum=24;                                           EDefscolliderSize[419].x=0.48f; EDefscolliderSize[419].y=2.0f;  EDefsmass[420]=1.5f; EDefsangularDrag[420]=1.5f; EDefs[420].glowIndex=544;
    /*421 npc_avian_mutant*/        EDefs[421].modelIndex=328; EDefs[421].texIndex=568; EDefs[421].numclips= 5; EDefs[421].animationNum=35; EDefscolliderCenter[421].y= 0.0200f;     EDefscolliderSize[421].x=0.40f; EDefscolliderSize[421].y=1.60f; EDefsmass[421]=2.0f; EDefsangularDrag[421]=1.0f;
    /*422 npc_exec_bot*/            EDefs[422].modelIndex=316; EDefs[422].texIndex=555; EDefs[422].numclips= 5; EDefs[422].animationNum=29; EDefscolliderCenter[422].y= 0.0125f;     EDefscolliderSize[422].x=0.48f; EDefscolliderSize[422].y=2.025f;EDefsmass[422]=2.2f; EDefsangularDrag[422]=1.5f;
    /*423 npc_cyborg_drone*/        EDefs[423].modelIndex=312; EDefs[423].texIndex=547; EDefs[423].numclips= 7; EDefs[423].animationNum=3;                                            EDefscolliderSize[423].x=0.36f; EDefscolliderSize[423].y=2.00f; EDefsmass[423]=1.5f; EDefsangularDrag[423]=2.0f;
    /*424 npc_cortex_reaver*/       EDefs[424].modelIndex=300; EDefs[424].texIndex=543; EDefs[424].numclips=6;  EDefs[424].animationNum=23; EDefscolliderCenter[424].y=-0.02263292f; EDefscolliderSize[424].x=0.451f;                                EDefsmass[424]=5.0f; EDefsangularDrag[424]=3.0f; EDefscollider[424]=COLTYPE_SPH;
    /*425 npc_cyborg_warrior*/      EDefs[425].modelIndex=315; EDefs[425].texIndex=554; EDefs[425].numclips=7;  EDefs[425].animationNum=28;                                           EDefscolliderSize[425].x=0.48f; EDefscolliderSize[425].y=2.00f; EDefsmass[425]=1.5f; EDefsangularDrag[425]=2.0f;
    /*426 npc_cyborg_enforcer*/     EDefs[426].modelIndex=314; EDefs[426].texIndex=550; EDefs[426].numclips=8;  EDefs[426].animationNum=27; EDefscolliderCenter[426].y=0.05f;        EDefscolliderSize[426].x=0.40f; EDefscolliderSize[426].y=2.08f; EDefsmass[426]=1.5f;
    /*427 npc_cyborg_elite*/        EDefs[427].modelIndex=313; EDefs[427].texIndex=548; EDefs[427].numclips=10; EDefs[427].animationNum=26; EDefscolliderCenter[427].y=0.10f;        EDefscolliderSize[427].x=0.44f; EDefscolliderSize[427].y=2.20f; EDefsmass[427]=3.5f;
    /*428 npc_cyborg_diego*/        EDefs[428].modelIndex=309; EDefs[428].texIndex=546; EDefs[428].numclips=6;  EDefs[428].animationNum=25;                                           EDefscolliderSize[428].x=0.48f; EDefscolliderSize[428].y=2.12f; EDefsmass[428]=2.0f;
    /*429 npc_sec1_bot*/            EDefs[429].modelIndex=333; EDefs[429].texIndex=573; EDefs[429].numclips=2;  EDefs[429].animationNum=38; EDefscolliderCenter[429].y=0.05f;        EDefscolliderSize[429].x=0.64f;                                 EDefsmass[429]=1.5f; EDefsangularDrag[429]=0.8f; EDefscollider[429]=COLTYPE_SPH;
    /*430 npc_sec2_bot*/            EDefs[430].modelIndex=335; EDefs[430].texIndex=574; EDefs[430].numclips=6;  EDefs[430].animationNum=39; EDefscolliderCenter[430].y=0.2f;         EDefscolliderSize[430].x=0.80f; EDefscolliderSize[430].y=2.40f; EDefsmass[430]=4.51f;
    /*431 npc_maint_bot*/           EDefs[431].modelIndex=325; EDefs[431].texIndex=567; EDefs[431].numclips=4;  EDefs[431].animationNum=34; EDefscolliderCenter[431].y=-0.3f;        EDefscolliderSize[431].x=0.48f;                                 EDefsmass[431]=1.5f; EDefsangularDrag[431]=1.5f; EDefscollider[431]=COLTYPE_SPH;
    /*432 npc_mutant_cyborg*/       EDefs[432].modelIndex=329; EDefs[432].texIndex=569; EDefs[432].numclips=7;  EDefs[432].animationNum=51; EDefscolliderCenter[432].y=0.12f;        EDefscolliderSize[432].x=0.65f; EDefscolliderSize[432].y=2.30f; EDefsmass[432]=3.0f; // Josiah's assumption is that the Mutant Cyborg is the "toaster oven" to inspire the first ever ECS that LGS made on Thief as they experimented more with their entity management, per Mahk interview with Casey Muratori: https://www.youtube.com/watch?v=73Do0OScoOU
    /*433 npc_hopper*/              EDefs[433].modelIndex=322; EDefs[433].texIndex=562; EDefs[433].numclips=8;  EDefs[433].animationNum=32; EDefscolliderCenter[433].z=1.0f;         EDefscolliderSize[433].x=0.64f; EDefscolliderSize[433].y=2.00f; EDefsangularDrag[433]=1000.0f; EDefsdynamicFriction[433]=0.005f; EDefsstaticFriction[433]=0.1f;
    /*434 npc_humanoid_mutant*/     EDefs[434].modelIndex=323; EDefs[434].texIndex=563; EDefs[434].numclips=6;  EDefs[434].animationNum=2;                                            EDefscolliderSize[434].x=0.38f; EDefscolliderSize[434].y=2.00f; EDefsmass[434]=1.4f; EDefsangularDrag[434]=2.0f;
    /*435 npc_invisomut*/           EDefs[435].modelIndex=324; EDefs[435].texIndex=565; EDefs[435].numclips=5;  EDefs[435].animationNum=33; EDefscolliderCenter[435].y=-0.28938290f; EDefscolliderSize[435]=(V3){1.5f,1.078766f,0.8f};           EDefsmass[435]=1.3f; EDefsangularDrag[435]=0.8f; EDefscollider[435]=COLTYPE_BOX;
    /*436 npc_virus_mutant*/        EDefs[436].modelIndex=330; EDefs[436].texIndex=576; EDefs[436].numclips=6;  EDefs[436].animationNum=41; EDefscolliderCenter[436].y=-0.05f;       EDefscolliderSize[436].x=0.40f; EDefscolliderSize[436].y=1.90f; EDefsmass[436]=1.4f; EDefsangularDrag[436]=2.0f;
    /*437 npc_servbot*/             EDefs[437].modelIndex=5153;EDefs[437].texIndex=575; EDefs[437].numclips=5;  EDefs[437].animationNum=40; EDefs[437].colMeshIndex=54; EDefsmass[437]=2.50f; EDefsangularDrag[437]=1.0f; EDefscollider[437]=COLTYPE_CVX;
    /*438 npc_flier_bot*/           EDefs[438].modelIndex=318; EDefs[438].texIndex=558; EDefs[438].numclips=5;  EDefs[438].animationNum=30; EDefsmass[438]=1.75f; EDefsangularDrag[438]=0.8f;
    /*439 npc_zerog_mutant*/        EDefs[439].modelIndex=395; EDefs[439].texIndex=1170;EDefs[439].numclips=3;  EDefs[439].animationNum=42; EDefsmass[439]=1.30f; EDefsangularDrag[439]=1.0f;
    /*440 npc_gorilla_tiger_mutant*/EDefs[440].modelIndex=320; EDefs[440].texIndex=560; EDefs[440].numclips=7;  EDefs[440].animationNum=31; EDefsmass[440]=2.00f;
    /*441 npc_repairbot*/           EDefs[441].modelIndex=331; EDefs[441].texIndex=572; EDefs[441].numclips=4;  EDefs[441].animationNum=37; EDefsmass[441]=1.50f; EDefsangularDrag[441]=2.0f;
    /*442 npc_plant_mutant*/        EDefs[442].modelIndex=330; EDefs[442].texIndex=570; EDefs[442].numclips=6;  EDefs[442].animationNum=36; EDefsmass[442]=0.80f; EDefsangularDrag[442]=1.5f;
    /*443 npc_cyberdog*/            EDefs[443].modelIndex=302; EDefsmass[443]=1.50f; EDefsangularDrag[443]=3.0f;
    /*444 npc_cyberguard*/          EDefs[444].modelIndex=303; EDefsmass[444]=2.00f; EDefsangularDrag[444]=3.0f;
    /*445 npc_cyberram*/            EDefs[445].modelIndex=304; EDefsmass[445]=2.00f; EDefsangularDrag[445]=3.0f;
    /*446 npc_cyber_reaver*/        EDefs[446].modelIndex=305; EDefsmass[446]=2.20f; EDefsangularDrag[446]=3.0f;
    /*447 npc_cybershodan*/         EDefsmass[447]=4.51f; EDefsangularDrag[447]=3.0f; EDefsdynamicFriction[447]=0.6f; EDefsstaticFriction[447]=0.6f;
    for (int i=448;i<=457;++i) { EDefscollider[i]=COLTYPE_SPH; EDefscolliderSize[i]=(V3){1.5f,1.5f,1.5f}; } // Cyber Item Definitions
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
    for (int i=458;i<=463;++i) { EDefsangularDrag[i]=0.05f; EDefsdynamicFriction[i]=0.5f; EDefsstaticFriction[i]=0.6f; EDefsmass[i]=1.5f; } // Physical Generic Objects
    /*458 prop_phys_barrel_chemical*/ EDefs[458].modelIndex=12;  EDefs[458].texIndex=30;
    /*459 prop_phys_barrel_radiation*/EDefs[459].modelIndex=12;  EDefs[459].texIndex=31;
    /*460 prop_phys_barrel_toxic*/    EDefs[460].modelIndex=12;  EDefs[460].texIndex=33;
    /*461 prop_phys_cart*/            EDefs[461].modelIndex=40;  EDefs[461].texIndex=416; EDefsmass[461]=2.5f;
    /*462 prop_phys_pot*/             EDefs[462].modelIndex=494; EDefsmass[462]=0.3f;
    /*463 prop_phys_toolcart*/        EDefs[463].modelIndex=624; EDefs[463].texIndex=865; EDefs[463].normIndex=864;  EDefs[463].specIndex=866;  EDefsmass[463]=20.0f; EDefsangularDrag[463]=0.2f;
    /*464 se_briefcase*/        EDefs[464].modelIndex=34;  EDefs[464].texIndex=66;  EDefs[464].glowIndex=65; 
    /*465 se_corpse_blueshirt*/                                   EDefs[465].texIndex=126;  EDefs[465].specIndex=127; EDefs[465].modelIndex=51; 
    /*466 se_corpse_brownshirt*/EDefs[466].texIndex=128;  EDefs[466].specIndex=129;  EDefs[466].modelIndex=52; 
    /*467 se_corpse_eaten*/     EDefs[467].texIndex=130;  EDefs[467].specIndex=131;  EDefs[467].modelIndex=53; 
    /*468 se_corpse_labcoat*/   EDefs[468].texIndex=132;  EDefs[468].specIndex=133;  EDefs[468].modelIndex=55; 
    /*469 se_corpse_security*/  EDefs[469].texIndex=136;  EDefs[469].specIndex=137;  EDefs[469].modelIndex=56; 
    /*470 se_corpse_tan*/       EDefs[470].texIndex=138;  EDefs[470].modelIndex=57; 
    /*471 se_corpse_torso*/     EDefs[471].texIndex=126;  EDefs[471].specIndex=127;  EDefs[471].modelIndex=58; 
    /*472 se_crate1*/           EDefs[472].texIndex=145;  EDefs[472].modelIndex=60;  EDefscollider[472]=COLTYPE_BOX; EDefscolliderSize[472]=(V3){0.684186f,0.6841861f,0.6841861f};  EDefs[472].colMeshIndex=U16_MAX;  EDefsmass[472]=0.75f; EDefsgravity[472]=1.0f; EDefsdynamicFriction[472]=0.6f; EDefsstaticFriction[472]=0.6f;
    /*473 se_crate2*/           EDefs[473].texIndex=143;  EDefs[473].modelIndex=60;  EDefscollider[473]=COLTYPE_BOX; EDefscolliderSize[473]=(V3){0.684186f,0.6841861f,0.6841861f};  EDefs[473].colMeshIndex=U16_MAX;  EDefsmass[473]=0.75f; EDefsgravity[473]=1.0f; EDefsdynamicFriction[473]=0.6f; EDefsstaticFriction[473]=0.6f;
    /*474 se_crate3*/           EDefs[474].texIndex=144;  EDefs[474].modelIndex=60;  EDefscollider[474]=COLTYPE_BOX; EDefscolliderSize[474]=(V3){0.684186f,0.6841861f,0.6841861f};  EDefs[474].colMeshIndex=U16_MAX;  EDefsmass[474]=0.75f; EDefsgravity[474]=1.0f; EDefsdynamicFriction[474]=0.6f; EDefsstaticFriction[474]=0.6f;
    /*475 se_crate4*/           EDefs[475].texIndex=146;  EDefs[475].modelIndex=60;  EDefscollider[475]=COLTYPE_BOX; EDefscolliderSize[475]=(V3){0.684186f,0.6841861f,0.6841861f};  EDefs[475].colMeshIndex=U16_MAX;  EDefsmass[475]=2.25f; EDefsgravity[475]=1.0f; EDefsdynamicFriction[475]=0.6f; EDefsstaticFriction[475]=0.6f;
    /*476 se_crate5*/           EDefs[476].modelIndex=60;  EDefscollider[476]=COLTYPE_BOX; EDefscolliderSize[476]=(V3){0.684186f,0.6841861f,0.6841861f};  EDefs[476].colMeshIndex=U16_MAX;  EDefsmass[476]=2.25f; EDefsdynamicFriction[476]=0.6f; EDefsstaticFriction[476]=0.6f;
    /*477 sec_camera*/       EDefs[477].modelIndex=589;  EDefs[477].texIndex=73;  EDefs[477].glowIndex=72; EDefscollider[477]=COLTYPE_MSH;
    /*478 sec_cpunode*/      EDefs[478].modelIndex=587;  EDefs[478].texIndex=242;  EDefs[478].glowIndex=248; EDefscollider[478]=COLTYPE_MSH;
    /*479 sec_cpunode_small*/EDefs[479].modelIndex=588;  EDefs[479].texIndex=107; EDefscollider[479]=COLTYPE_MSH;
    /*480 weapon_cyber_mine*/ EDefs[480].modelIndex=71;  EDefs[480].texIndex=1224;
    for (int i=481;i<=895;++i) { EDefsangularDrag[i]=0.05f; EDefsdynamicFriction[i]=0.5f; EDefsstaticFriction[i]=0.6f; EDefsmass[i]=0.3f; } // Physical Generic Objects
    /*481 proj_enemshot2*/       EDefs[481].modelIndex=MAX_MDLS;
    /*482 proj_magpulse_shot*/   EDefs[482].modelIndex=MAX_MDLS; EDefs[482].texIndex=807;
    /*483 proj_stungun_shot*/    EDefs[483].modelIndex=MAX_MDLS; EDefs[483].texIndex=835;
    /*484 proj_rail_shot*/       EDefs[484].modelIndex=652; 
    /*485 proj_plasmarifle_shot*/EDefs[485].modelIndex=651; EDefsbounciness[485]=0.9f;
    /*486 proj_enemshot6*/       EDefs[486].modelIndex=MAX_MDLS;
    /*487 proj_enemshot5*/       EDefs[487].modelIndex=MAX_MDLS; EDefsmass[487]=0.2f;
    /*488 proj_enemshot4*/       EDefs[488].modelIndex=MAX_MDLS;
    /*489 proj_throwingstar*/    EDefs[489].modelIndex=307;
    /*490 proj_magpulsenpc_shot*/EDefs[490].modelIndex=645;
    /*491 proj_railnpc_shot*/    EDefs[491].modelIndex=MAX_MDLS;
    /*492 proj_cyberplayer_shot*/EDefs[492].modelIndex=72;
    /*493 proj_cyberdog_shot*/   EDefs[493].modelIndex=63;
    /*494 proj_cyberreaver_shot*/EDefs[494].modelIndex=64;
    /*495 proj_cyberice_shot*/   EDefs[495].modelIndex=68;
    for (int i=496;i<515;++i) { EDefs[i].SFXIndex = 75; EDefscollider[i]=COLTYPE_MSH;  } // Doors
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
    /*515 func_forcebridge*/ EDefs[515].modelIndex=78;  EDefs[515].texIndex=38; EDefscollider[515]=COLTYPE_BOX;
    /*516 prop_lift2*/       EDefs[516].modelIndex=215;  EDefs[516].texIndex=155;  EDefs[516].glowIndex=154;  EDefscollider[516]=COLTYPE_BOX;  EDefscolliderCenter[516]=(V3){0.0f,0.0f,0.0f};  EDefscolliderSize[516]=(V3){1.0f,1.0f,1.0f};  EDefs[516].colMeshIndex=U16_MAX; 
    /*517 func_wall*/        EDefsmass[517]=10.0f;  EDefsangularDrag[517]=0.05f;  EDefsgravity[517]=0.0f;  EDefs[517].kinematic=true;  EDefsdynamicFriction[517]=0.6f;  EDefsstaticFriction[517]=0.6f;
    /*518 BulletHoleLarge*/
    /*519 BulletHoleScorchLarge*/
    /*520 BulletHoleScorchSmall*/
    /*521 BulletHoleSmall*/
    /*522 BulletHoleTiny*/
    /*523 BulletHoleTinySpread*/
    /*524 func_door_cyber*/      EDefs[524].modelIndex=178; EDefs[524].texIndex=1224; EDefscollider[524]=COLTYPE_BOX; EDefscolliderCenter[524]=(V3){0.0f,1.31f,0.0f}; EDefscolliderSize[524]=(V3){2.56f,0.06f,2.56f}; EDefs[524].colMeshIndex=U16_MAX; 
    /*525 prop_console01*/       EDefs[525].texIndex=100; EDefs[525].modelIndex=49; EDefscollider[525]=COLTYPE_MSH;
    /*526 prop_console02*/       EDefs[526].texIndex=100; EDefs[526].modelIndex=50; EDefscollider[526]=COLTYPE_MSH;
    /*527 prop_grate1_1*/        EDefs[527].modelIndex=186; EDefs[527].texIndex=359; EDefscollider[527]=COLTYPE_MSH;
    /*528 prop_grate1_2*/        EDefs[528].modelIndex=187; EDefs[528].texIndex=360; EDefscollider[528]=COLTYPE_MSH;
    /*529 prop_grate1_3*/        EDefs[529].modelIndex=188; EDefs[529].texIndex=361; EDefscollider[529]=COLTYPE_MSH;
    /*530 se_cabinet*/           EDefs[530].modelIndex=39; EDefs[530].texIndex=70; EDefscollider[530]=COLTYPE_MSH;
    /*531 se_thermos*/           EDefs[531].texIndex=863; EDefs[531].modelIndex=623; EDefscollider[531]=COLTYPE_MSH;
    /*532 prop_beaker_holder*/   EDefs[532].modelIndex=15; EDefs[532].texIndex=36; EDefscollider[532]=COLTYPE_MSH;
    /*533 prop_bed*/             EDefs[533].modelIndex=16; EDefs[533].texIndex=246; EDefscollider[533]=COLTYPE_MSH;
    /*534 prop_bed_hospital*/    EDefs[534].modelIndex=608; EDefs[534].texIndex=759; EDefscollider[534]=COLTYPE_MSH;
    /*535 prop_bed_neurosurgery*/EDefs[535].texIndex=18; EDefs[535].normIndex=29; EDefs[535].specIndex=1238; EDefs[535].modelIndex=17; EDefscollider[535]=COLTYPE_MSH;
    /*536 prop_bonepile1*/       EDefs[536].modelIndex=19; EDefs[536].texIndex=815; EDefscollider[536]=COLTYPE_MSH;
    /*537 prop_bridgewall1*/     EDefs[537].modelIndex=33; 
    /*538 prop_broken_clock*/    EDefs[538].modelIndex=38; EDefs[538].texIndex=1117; EDefs[538].altTexIndex=1118; EDefs[538].glowIndex=1115; EDefs[538].altGlowIndex=1116; 
    /*539 prop_brokengun*/       EDefs[539].modelIndex=639; EDefs[539].texIndex=878; 
    /*540 prop_chair01*/         EDefs[540].modelIndex=41; EDefs[540].texIndex=195; EDefscollider[540]=COLTYPE_MSH;
    /*541 prop_chair02*/         EDefs[541].modelIndex=42; EDefs[541].texIndex=195; EDefscollider[541]=COLTYPE_MSH;
    /*542 prop_chair03*/         EDefs[542].modelIndex=43; EDefs[542].texIndex=195; EDefscollider[542]=COLTYPE_MSH;
    /*543 prop_chair04*/         EDefs[543].modelIndex=41; EDefs[543].texIndex=195; EDefscollider[543]=COLTYPE_MSH;
    /*544 prop_chair05*/         EDefs[544].modelIndex=42; EDefs[544].texIndex=195; EDefscollider[544]=COLTYPE_MSH;
    /*545 prop_chandelier*/      EDefs[545].modelIndex=496; EDefs[545].texIndex=644;
    /*546 prop_charge_station*/  EDefs[546].modelIndex=44; EDefs[546].texIndex=77; EDefs[546].glowIndex=76; EDefscollider[546]=COLTYPE_MSH;
    /*547 prop_clothes*/         EDefs[547].modelIndex=47; EDefs[547].texIndex=97; EDefscollider[547]=COLTYPE_MSH;
    /*548 prop_computer*/        EDefs[548].modelIndex=48; EDefs[548].texIndex=195; EDefscollider[548]=COLTYPE_MSH;
    /*549 prop_couch*/           EDefs[549].modelIndex=59; EDefs[549].texIndex=195; EDefscollider[549]=COLTYPE_MSH;
    /*550 prop_couch2*/          EDefs[550].modelIndex=59; EDefs[550].texIndex=195; EDefscollider[550]=COLTYPE_MSH;
    /*551 prop_cpuscreen*/       EDefs[551].modelIndex=178; EDefs[551].texIndex=768; EDefscollider[551]=COLTYPE_MSH;
    /*552 prop_cyber_datafrag*/  EDefs[552].modelIndex=78; 
    /*553 prop_cyber_decoy*/     EDefs[553].modelIndex=78; 
    /*554 prop_cyber_exit*/      EDefs[554].modelIndex=78; 
    /*555 prop_cyber_switch*/    EDefs[555].modelIndex=0; 
    /*556 prop_cyberport*/       EDefs[556].modelIndex=62; EDefs[556].texIndex=117; EDefs[556].glowIndex=116; EDefscollider[556]=COLTYPE_MSH;
    /*557 prop_desk01*/          EDefs[557].modelIndex=74; EDefs[557].texIndex=125; EDefscollider[557]=COLTYPE_MSH;
    /*558 prop_desk02*/          EDefs[558].modelIndex=75; EDefs[558].texIndex=124; EDefscollider[558]=COLTYPE_MSH;
    /*559 prop_dexmissile*/            EDefs[559].modelIndex=76; EDefs[559].texIndex=164; EDefs[559].glowIndex=162; 
    /*560 prop_foliage_fernpoison*/    EDefs[560].modelIndex=160; EDefs[560].texIndex=331;
    /*561 prop_foliage_bush*/          EDefs[561].modelIndex=495; EDefs[561].texIndex=643; EDefs[561].glowIndex=642; 
    /*562 prop_foliage_fern*/          EDefs[562].modelIndex=160; EDefs[562].texIndex=333; EDefs[562].glowIndex=330; 
    /*563 prop_foliage_fernblueflower*/EDefs[563].modelIndex=159; EDefs[563].texIndex=333; EDefs[563].glowIndex=330; 
    /*564 prop_foliage_pinetreem*/     EDefs[564].modelIndex=489; EDefs[564].texIndex=594;
    /*565 prop_foliage_poisonbush1*/   EDefs[565].modelIndex=493; EDefs[565].texIndex=638;
    /*566 prop_gear_large*/            EDefs[566].modelIndex=166; EDefs[566].texIndex=335; EDefscollider[566]=COLTYPE_MSH;
    /*567 prop_gear_small*/            EDefs[567].modelIndex=167; EDefs[567].texIndex=336; EDefscollider[567]=COLTYPE_MSH;
    /*568 prop_grass1*/                EDefs[568].texIndex=329;
    /*569 prop_grass2*/                EDefs[569].texIndex=329;
    /*570 prop_grass3*/                EDefs[570].texIndex=329;
    /*571 prop_grass4*/                EDefs[571].texIndex=329;
    /*572 prop_grass5*/                EDefs[572].texIndex=329;
    /*573 prop_grate4*/                EDefs[573].modelIndex=161; EDefs[573].texIndex=329; EDefscollider[573]=COLTYPE_MSH;
    /*574 prop_healingbed*/ EDefs[574].modelIndex=195; EDefs[574].texIndex=1139; EDefscollider[574]=COLTYPE_MSH;
    /*575 prop_lamp*/ EDefs[575].modelIndex=212; EDefs[575].texIndex=423; EDefscollider[575]=COLTYPE_MSH;
    /*576 prop_light_emergsignal*/ EDefs[576].modelIndex=216; EDefs[576].texIndex=426; EDefs[576].altTexIndex=424; EDefs[576].glowIndex=0; EDefs[576].altGlowIndex=424; EDefscollider[576]=COLTYPE_MSH;
    /*577 prop_microscope*/ EDefs[577].modelIndex=298; EDefs[577].texIndex=645; EDefs[577].specIndex=1241; EDefscollider[577]=COLTYPE_MSH;
    /*578 prop_pipe*/ EDefs[578].modelIndex=490; EDefs[578].texIndex=595; EDefscollider[578]=COLTYPE_MSH;
    /*579 prop_puddle*/ EDefs[579].modelIndex=157; EDefs[579].texIndex=648;
    /*580 prop_puddle_grease*/ EDefs[580].modelIndex=157; EDefs[580].texIndex=650;
    /*581 prop_puddle_oil*/ EDefs[581].modelIndex=157; EDefs[581].texIndex=652;
    /*582 prop_shelves*/ EDefs[582].modelIndex=591; EDefs[582].texIndex=94; EDefscollider[582]=COLTYPE_MSH;
    /*583 prop_skeleton*/ EDefs[583].modelIndex=592; EDefs[583].texIndex=815; EDefscollider[583]=COLTYPE_MSH;
    /*584 prop_sleeping_cables*/ EDefs[584].modelIndex=595; EDefs[584].texIndex=71;
    /*585 prop_sparkingwire*/ EDefs[585].modelIndex=0; EDefs[585].numclips=1; EDefs[585].animationNum=46; EDefs[585].texIndex=71;
    /*586 prop_table*/ EDefs[586].modelIndex=619; EDefs[586].texIndex=92; EDefscollider[586]=COLTYPE_MSH;
    /*587 prop_tv_on_a_post*/ EDefs[587].modelIndex=625; EDefs[587].texIndex=1228; EDefscollider[587]=COLTYPE_MSH;
    /*588 prop_vendingmachines1*/ EDefs[588].modelIndex=627; EDefs[588].texIndex=870; EDefscollider[588]=COLTYPE_MSH;
    /*589 prop_vendingmachines2*/ EDefs[589].modelIndex=614; EDefs[589].texIndex=871; EDefscollider[589]=COLTYPE_MSH;
    /*590 prop_weapon_rack*/ EDefs[590].modelIndex=641; EDefs[590].texIndex=113; EDefscollider[590]=COLTYPE_MSH;
    /*591 prop_xray*/ EDefs[591].modelIndex=660; EDefs[591].texIndex=153; EDefscollider[591]=COLTYPE_MSH;
    /*592 text_decal*/ EDefs[592].modelIndex=77;
    /*593 text_decalStopDSS1*/ EDefs[593].modelIndex=77; 
    /*594 trigger_counter*/
    /*595 trigger_cyberpush*/
    /*596 trigger_gravitylift*/
    /*597 trigger_ladder*/
    /*598 trigger_multiple*/
    /*599 trigger_music*/
    /*600 trigger_once*/
    /*601 trigger_radiation*/
    /*602 us_isotopepanel*/ EDefs[602].modelIndex=0;  EDefs[602].texIndex=616;  EDefs[602].numclips=5;  EDefs[602].animationNum=44; 
    /*603 us_paperlog*/ EDefs[603].modelIndex=486;  EDefs[603].texIndex=580; 
    /*604 us_puz_elevatorkeypad*/ EDefs[604].modelIndex=615;  EDefs[604].texIndex=247; 
    /*605 us_puz_elevatorkeypad2*/ EDefs[605].modelIndex=618;  EDefs[605].texIndex=250; 
    /*606 us_puz_elevatorkeypad3*/ EDefs[606].modelIndex=615;  EDefs[606].texIndex=247; 
    /*607 us_puz_elevatorkeypad4*/ EDefs[607].modelIndex=210;  EDefs[607].texIndex=249; 
    /*608 us_puz_keypad*/ EDefs[608].modelIndex=211;  EDefs[608].texIndex=414; 
    /*609 us_puz_panel_blue_grid*/ EDefs[609].modelIndex=0;  EDefs[609].texIndex=604;  EDefs[609].numclips=3;  EDefs[609].animationNum=43; 
    /*610 us_puz_panel_brown_grid*/ EDefs[610].modelIndex=0;  EDefs[610].texIndex=604;  EDefs[610].numclips=3;  EDefs[610].animationNum=43; 
    /*611 us_puz_panel_gray_grid*/ EDefs[611].modelIndex=0;  EDefs[611].texIndex=634;  EDefs[611].numclips=3;  EDefs[611].animationNum=43; 
    /*612 us_puz_panel_red_grid*/ EDefs[612].modelIndex=0;  EDefs[612].texIndex=625;  EDefs[612].numclips=3;  EDefs[612].animationNum=43; 
    /*613 us_puz_panel_teal_grid*/ EDefs[613].modelIndex=0;  EDefs[613].texIndex=601;  EDefs[613].numclips=3;  EDefs[613].animationNum=43; 
    /*614 us_relaypanel*/ EDefs[614].modelIndex=0;  EDefs[614].texIndex=617;  EDefs[614].numclips=4;  EDefs[614].animationNum=45; 
    /*615 us_retinalscanner*/ EDefs[615].modelIndex=79;  EDefs[615].texIndex=46; 
    /*616 prop_vending1_1*/ EDefs[616].modelIndex=627;  EDefs[616].texIndex=870; 
    /*617 prop_vending1_2*/ EDefs[617].modelIndex=628;  EDefs[617].texIndex=870; 
    /*618 prop_vending1_3*/ EDefs[618].modelIndex=629;  EDefs[618].texIndex=870; 
    /*619 prop_vending2_1*/ EDefs[619].modelIndex=614;  EDefs[619].texIndex=871; 
    /*620 prop_vending2_2*/ EDefs[620].modelIndex=621;  EDefs[620].texIndex=871; 
    /*621 ambient_airhiss*/ EDefs[621].volume=0.05f;
    /*622 ambient_clicker*/ EDefs[622].volume=0.20f;
    /*623 ambient_compressor*/ EDefs[623].volume=0.4f;
    /*624 ambient_dishwasher*/ EDefs[624].volume=0.2f;
    /*625 ambient_drip_amb*/   EDefs[625].volume=0.5f;
    /*626 ambient_fan*/        EDefs[626].volume=0.3f;
    /*627 ambient_generator_gas*/ EDefs[627].volume=0.3f;
    /*628 ambient_gurgle*/    EDefs[628].volume=0.3f;
    /*629 ambient_icemaker*/  EDefs[629].volume=0.6f;
    /*630 ambient_intake*/    EDefs[630].volume=0.2f;
    /*631 ambient_lathe*/     EDefs[631].volume=0.4f;
    /*632 ambient_lev3loop1*/ EDefs[632].volume=0.1f;
    /*633 ambient_lev3loop2*/ EDefs[633].volume=0.1f;
    /*634 ambient_lev3loop3*/ EDefs[634].volume=0.1f;
    /*635 ambient_lev3loop4*/ EDefs[635].volume=0.1f;
    /*636 ambient_liquid_bubble*/     EDefs[636].volume=1.0f;
    /*637 ambient_liquid_lava2*/      EDefs[637].volume=0.4f;
    /*638 ambient_looping*/           EDefs[638].volume=0.4f;
    /*639 ambient_machgear_loop*/     EDefs[639].volume=0.4f;
    /*640 ambient_machine_ambience*/  EDefs[640].volume=0.8f;
    /*641 ambient_machine_go*/        EDefs[641].volume=0.6f;
    /*642 ambient_machine_humamb7*/   EDefs[642].volume=1.0f;
    /*643 ambient_machine_humlonoise*/EDefs[643].volume=0.4f;
    /*644 ambient_machine_loop1*/     EDefs[644].volume=0.4f;
    /*645 ambient_machine_loop2*/     EDefs[645].volume=0.4f;
    /*646 ambient_machinea1*/         EDefs[646].volume=0.4f;
    /*647 ambient_machinevat_loop*/   EDefs[647].volume=0.8f;
    /*648 ambient_mist*/              EDefs[648].volume=0.02f;
    /*649 ambient_pipewater_loop*/    EDefs[649].volume=0.65f;
    /*650 ambient_powerloom*/         EDefs[650].volume=0.3f;
    /*651 ambient_pump*/              EDefs[651].volume=0.2f;
    /*652 ambient_pump2*/             EDefs[652].volume=0.05f;
    /*653 ambient_rain*/              EDefs[653].volume=0.55f;
    /*654 ambient_steam_loop*/        EDefs[654].volume=0.1f;
    /*655 ambient_washing_machine*/   EDefs[655].volume=0.5f;
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
    /*688 func_switch1*/         EDefs[688].modelIndex=609;  EDefs[688].texIndex=837;  EDefscollider[688]=COLTYPE_BOX;  EDefscolliderCenter[688]=(V3){0.0f,0.0f,0.0f};  EDefscolliderSize[688]=(V3){0.32f,0.04f,0.32f}; EDefs[688].colMeshIndex=U16_MAX; 
    /*689 func_switch2*/         EDefs[689].modelIndex=610;  EDefs[689].texIndex=839;  EDefs[689].mainSwitchMaterial=839;  EDefs[689].altTexIndex=841;  EDefs[689].glowIndex=0;  EDefs[689].altGlowIndex=840;  EDefs[689].changeTexOnActive=true; EDefs[689].blinkTexOnActive=true;  EDefscollider[689]=COLTYPE_BOX;  EDefscolliderCenter[689]=(V3){-0.0243553f,0.0f,0.000004883f};  EDefscolliderSize[689]=(V3){0.0476318f,0.64f,0.64f};  EDefs[689].colMeshIndex=U16_MAX; 
    /*690 func_switch3*/         EDefs[690].modelIndex=611;  EDefs[690].texIndex=842;  EDefs[690].altTexIndex=844;  EDefs[690].glowIndex=0;  EDefs[690].altGlowIndex=843;  EDefs[690].changeTexOnActive=true;  EDefscollider[690]=COLTYPE_BOX;  EDefscolliderCenter[690]=(V3){-0.02285008f,0.000053061f,-0.000056993f};  EDefscolliderSize[690]=(V3){0.02f,0.32f,0.32f};  EDefs[690].colMeshIndex=U16_MAX; 
    /*691 func_switch4*/         EDefs[691].modelIndex=612;  EDefs[691].texIndex=846;  EDefscollider[691]=COLTYPE_BOX;  EDefscolliderCenter[691]=(V3){0.06f,0.0f,0.0f};  EDefscolliderSize[691]=(V3){0.2f,0.64f,0.64f};  EDefs[691].colMeshIndex=U16_MAX; 
    /*692 func_switch5*/         EDefs[692].modelIndex=614;  EDefs[692].texIndex=848;  EDefscollider[692]=COLTYPE_BOX;  EDefscolliderCenter[692]=(V3){0.0f,0.0f,0.0f};  EDefscolliderSize[692]=(V3){0.64f,0.64f,0.08f};  EDefs[692].colMeshIndex=U16_MAX; 
    /*693 func_switch5broken*/   EDefs[693].modelIndex=613;  EDefs[693].texIndex=847;  EDefscollider[693]=COLTYPE_BOX;  EDefscolliderCenter[693]=(V3){0.0f,0.0f,0.0f};  EDefscolliderSize[693]=(V3){0.64f,0.64f,0.08f};  EDefs[693].colMeshIndex=U16_MAX; 
    /*694 func_switch7*/         EDefs[694].modelIndex=612;  EDefs[694].texIndex=854;  EDefscollider[694]=COLTYPE_BOX;  EDefscolliderCenter[694]=(V3){1.523325f,0.0f,0.0f};  EDefscolliderSize[694]=(V3){0.2008026f,0.64f,0.64f};  EDefs[694].colMeshIndex=U16_MAX; 
    /*695 func_switch8*/         EDefs[695].modelIndex=616;  EDefs[695].texIndex=856;  EDefs[695].altTexIndex=858;  EDefs[695].glowIndex=855;  EDefs[695].altGlowIndex=857;  EDefs[695].changeTexOnActive=true;  EDefscollider[695]=COLTYPE_BOX;  EDefscolliderCenter[695]=(V3){-0.04f,0.0f,0.0001220703f};  EDefscolliderSize[695]=(V3){0.08f,0.64f,0.64f};  EDefs[695].colMeshIndex=U16_MAX; 
    /*696 func_switchbroken1*/   EDefs[696].modelIndex=617;  EDefs[696].texIndex=618; 
    /*697 clip_npc*/             EDefscollider[697]=COLTYPE_BOX;  EDefscolliderCenter[697]=(V3){1.005016f,0.0f,0.0f};  EDefscolliderSize[697]=(V3){2.010033f,16.0f,16.0f};  EDefs[697].colMeshIndex=U16_MAX;
    /*698 clip_objects*/         EDefscollider[698]=COLTYPE_BOX;  EDefscolliderCenter[698]=(V3){0.0f,0.0f,0.0f};  EDefscolliderSize[698]=(V3){2.56f,2.56f,2.56f};  EDefs[698].colMeshIndex=U16_MAX;
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
    /*717 ef_cyber_ice*/ EDefscollider[717]=COLTYPE_SPH;  EDefscolliderCenter[717]=(V3){0.0f,0.004354001f,-0.014725f};  EDefscolliderSize[717]=(V3){1.0f,0.0f,0.0f};  EDefs[717].colMeshIndex=U16_MAX; 
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
}
// Lights
i32 AddLight(Light* lit, LightAnimation* lanim) {
    i32 i = World.loadedLights; World.loadedLights++; World.levelLoadedLights[World.currentLevel]++;
    if (World.loadedLights >= LIGHT_COUNT) { DualLogError("Too many lights %u added in level %d!\n",i,World.curLev); OS_Exit(1); }
    mcpy(&World.lights[i],lit,sizeof(Light)); mcpy(&World.lanims[i],lanim,sizeof(LightAnimation));
    World.lightsNewPosition[i] = lit->pos; flag_set(&World.lights[i].lflags,LDIRTY,true);
    return i;
}

bool alreadyReadLightOnOnce[LIGHT_COUNT] = {0};
void LoadFieldIntoLight(char* k, char* v, char* il, u32 ln, Light* lit, LightAnimation* lam, u16 lIdx) {
    char* br = StringFindFirstCharWithin(k,'[');
    if (br) {
        int i = parse_numberu32(br + 1,il,ln);
        if (i >= 0 && i < 32) { if(k[12] == 's'){lam->intervalSteps[i] = parse_float(v,il,ln);}else{lam->stepIsLerping[i] = parse_float(v,il,ln);} } /*"intervalSteps[" index 12 is 's', "intervalStepisLerping[" index 12 is 'i'*/
        return;
    }
    static const struct { const char* key; u16 offset; u8 type; } map[] = {
        {"currentStep",    __builtin_offsetof(LightAnimation,currentStep),1},{"lerpValue",      __builtin_offsetof(LightAnimation,lerpValue),0},{"intervalSteps.Length",__builtin_offsetof(LightAnimation,numIntervalSteps),1},{"intervalStepisLerping.Length",__builtin_offsetof(LightAnimation, numLerpSteps),1},
        {"localPosition.x",__builtin_offsetof(Light,pos.x),0},               {"localPosition.y",__builtin_offsetof(Light,pos.y),0},             {"localPosition.z",     __builtin_offsetof(Light,pos.z),0},                    {"localRotation.x",             __builtin_offsetof(Light,spotDir.x),0},
        {"localRotation.y",__builtin_offsetof(Light,spotDir.y),0},           {"localRotation.z",__builtin_offsetof(Light,spotDir.z),0},         {"localRotation.w",     __builtin_offsetof(Light,spotDir.w),0},                {"range",                       __builtin_offsetof(Light,range),0},
        {"spotAngle",      __builtin_offsetof(Light,spotAng),0},             {"minIntensity",   __builtin_offsetof(Light,minIntensity),0},      {"maxIntensity",        __builtin_offsetof(Light,maxIntensity),0},             {"color.r",__builtin_offsetof(Light,col.r),0},{"color.g",__builtin_offsetof(Light,col.g),0},{"color.b",__builtin_offsetof(Light,col.b),0}
    };
    for (int i = 0; i < (int)(sizeof(map)/sizeof(map[0])); i++) {
        if (sEqual(k, map[i].key)) { // Types: 0 = float, 1 = u8.  Check key prefix to decide if pointing at 'lit' or 'lam'
            void* dest = (k[0] == 'l' && k[1] == 'o') ? (void*)lit : (void*)lam;
            if (k[0] == 'r' || k[0] == 's' || k[0] == 'm' || k[0] == 'c') {
                if (k[1] != 'u') dest = (void*)lit; // range, spot, max, color (not currentStep)
            }
            char* ptr = (char*)dest + map[i].offset;
            if (map[i].type == 0) *(float*)ptr = parse_float(v,il,ln);
            else                  *(u8*)ptr = parse_numberu8(v,il,ln);
            return;
        }
    }
    if (sEqual(k,"intensity")) lit->intensity = lit->maxIntensity = parse_float(v,il,ln) * 0.35f;
    else if (sEqual(k,"type")) flag_set(&lit->lflags, (v[0] == 'S') ? LSPOT : LDIR, true);
    else if (sEqual(k,"lightOn") && !alreadyReadLightOnOnce[lIdx]) { alreadyReadLightOnOnce[lIdx] = true; flag_set(&lit->lflags,LIGHTON,parse_bool(v,il,ln)); }
    else if (sEqual(k,"lerpOn")) flag_set(&lit->lflags,LERPON,parse_bool(v,il,ln));
}

u16 headmountedLanternLight;
V3 lanternPos;
#define CHGD(a,b) (vabs((a) - (b)) > 0.0001f)
void UpdateLight(u16 i, V3 pos, Color3 col, float range, float intensity, float max, float min, float spotAng, Quaternion spotDir, bool on, bool shad) {
    bool changed = ((!!(World.lights[i].lflags & SHADON) - shad) || (!!(World.lights[i].lflags & LIGHTON) -  on) || CHGD(World.lights[i].range,range) || CHGD(World.lights[i].pos.x,pos.x) || CHGD(World.lights[i].pos.y,pos.y) || CHGD(World.lights[i].pos.z,pos.z));
    World.lights[i].intensity=intensity; World.lights[i].minIntensity=min; World.lights[i].maxIntensity=max; World.lights[i].spotAng=spotAng; World.lights[i].spotDir=spotDir; World.lights[i].col=col; World.lights[i].pos=World.lightsNewPosition[i]=pos; World.lights[i].range=range;
    flag_set(&World.lights[i].lflags,19,(World.lights[i].lflags&LDIRTY)|changed<<4|on|shad<<1);
}
#undef CHGD
// Level Loading and Entity Management System
void InitializeEntity(Entity* e) { mset(e,0,sizeof(Entity)); u16 idx=(u16)(e - World.instances); e->index=U16_MAX; e->entflags=EF_ACTIVE; e->kinematic=true; World.layer[idx]=L_Default; e->camView=255; e->tickTime = 0.35f; World.angularDrag[idx] = 0.05f; e->modelIndex=e->lodIndex=e->colMeshIndex=MAX_MDLS; e->texIndex=e->glowIndex=e->specIndex=e->normIndex = MAX_TXRS; World.scale[idx].x=World.scale[idx].y=World.scale[idx].z=World.mass[idx]=e->volume=World.rotation[idx].w=1.0f; World.dynamicFriction[idx] = World.staticFriction[idx] = 0.6f; }
void InitializeAIAfterLoad(u16 i);
void DeleteInstance(u16 i) { if (i <= PLAYER1 || i >= World.instCount) return; flag_set(&World.instances[i].entflags,EF_ACTIVE,false); } // Don't delete null ent, player 1, nor player 2 or already empty slots.
u16 AddInstance(u16 entIdx, V3 pos) {
    if (entIdx >= MAX_ENTITIES) { DualLogError("\nEntity index when loading non-light entity was %d, exceeds max defined entity count of %d, skipped\n",entIdx,MAX_ENTITIES); return 0; }
    if (World.instCount >= INSTANCE_COUNT) { DualLogError("\nToo many instances while adding entity %u, max instance count is %u, skipped\n", entIdx, INSTANCE_COUNT); return 0; }
    u16 i = World.instCount;
    InitializeEntity(&World.instances[i]);
    World.instances[i].index = entIdx;
    SetPosition(i,pos,true); // Marks dirty internally, using true to force as if twere teleported.
    if (IdxIsNPC(entIdx)) InitializeAIAfterLoad(i);
    World.instances[i].cardchunk = EDefs[entIdx].cardchunk;
    World.instances[i].modelIndex = EDefs[entIdx].modelIndex;
    World.instances[i].colMeshIndex = EDefs[entIdx].colMeshIndex;
    World.instances[i].numclips = EDefs[entIdx].numclips;
    World.instances[i].animationNum = EDefs[entIdx].animationNum;
    World.instances[i].texIndex = EDefs[entIdx].texIndex;
    World.instances[i].glowIndex = EDefs[entIdx].glowIndex >= MAX_TXRS ? 0 : EDefs[entIdx].glowIndex;
    World.instances[i].specIndex = EDefs[entIdx].specIndex >= MAX_TXRS ? 0 : EDefs[entIdx].specIndex;
    World.instances[i].normIndex = EDefs[entIdx].normIndex >= MAX_TXRS ? 0 : EDefs[entIdx].normIndex;
    World.instances[i].lodIndex = EDefs[entIdx].lodIndex;
    World.instances[i].kinematic = EDefs[entIdx].kinematic;
    flag_set(&World.instances[i].entflags,EF_RIGIDBODY,EDefs[entIdx].entflags & EF_RIGIDBODY);
    flag_set(&World.instances[i].entflags,EF_NO_SHADOWS,EDefs[entIdx].entflags & EF_NO_SHADOWS);
    World.collider[i] = EDefscollider[entIdx];
    World.colliderCenter[i] = EDefscolliderCenter[entIdx];
    World.colliderSize[i] = EDefscolliderSize[entIdx];
    World.mass[i] = EDefsmass[entIdx] > 0.0f ? EDefsmass[entIdx] : 1.0f;
    World.angularDrag[i] = EDefsangularDrag[entIdx] > 0.0f ? EDefsangularDrag[entIdx] : 0.05f;
    World.gravity[i] = EDefsgravity[entIdx] > 0.0f ? EDefsgravity[entIdx] : 1.0f;
    World.instances[i].lockedMessageLingdex = EDefs[entIdx].lockedMessageLingdex;
    World.instCount++;
    World.levelInstCount[World.currentLevel] = World.instCount;
    return i;
}

extern u16 headmountedLanternLight;
Entity entsFromFile[INSTANCE_COUNT]; V3 positionFromFile[INSTANCE_COUNT]; V3 scaleFromFile[INSTANCE_COUNT]; Quaternion rotationFromFile[INSTANCE_COUNT]; Light lightsFromFile[LIGHT_COUNT]; LightAnimation lanimsFromFile[LIGHT_COUNT];
void GenBTexture(u32 *id, i32 internalFormat, i32 width, i32 height, u32 format, u32 type, i32 filt);
void AddCamView(V3 p, Quaternion r, u8 fv, u16 w, u16 h, float nr, float fr) { camViews[camViewCount] = (CamView){p,r,fv,w,h,nr,fr,World.pauseRelativeTime + (camViewCount * 0.05f) + 0.5f,false};/*Staggered for perf*/ GenBTexture(&camViewTextures[camViewCount],GL_RGBA8,w,h,GL_RGBA,GL_UNSIGNED_BYTE,0x2600/*GL_NEAREST*/); camViewCount++; }
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
    World.collider         = World.levelCollider[lev];
    World.rotation         = World.levelRotation[lev];
    World.layer            = World.levelLayer[lev];
    World.mass             = World.levelMass[lev];
    World.radius           = World.levelRadius[lev];
    World.gravity          = World.levelGravity[lev];
    World.inertiaTensor    = World.levelInertiaTensor[lev];
    World.invInertiaTensor = World.levelInvInertiaTensor[lev];
    World.angularDrag      = World.levelAngularDrag[lev];
    World.dynamicFriction  = World.levelDynamicFriction[lev];
    World.staticFriction   = World.levelStaticFriction[lev];
    World.bounciness       = World.levelBounciness[lev];
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
    World.levelAngularDrag[dstLevel][s]          = World.levelAngularDrag[srcLevel][s];
    World.levelDynamicFriction[dstLevel][s]      = World.levelDynamicFriction[srcLevel][s];
    World.levelStaticFriction[dstLevel][s]       = World.levelStaticFriction[srcLevel][s];
    World.levelBounciness[dstLevel][s]           = World.levelBounciness[srcLevel][s];
    World.levelInvTnsrValid[dstLevel][s]         = World.levelInvTnsrValid[srcLevel][s];
    World.levelColliding[dstLevel][s]            = World.levelColliding[srcLevel][s];
}

char lineSpace[LINE_LEN_MAX];
void AddDoorPortal(u16 entIdx, u16 parent);
void TextureSequenceInit(u16 self, char* trimmed_value);
void LoadLevelMod(u8 lev) {
    u8 curlevel = vclamp(lev, 0, 13);
    World.curLev = curlevel;
    World.levelCurrentlyLoading = true;
    World.instCount = 3;
    if (curlevel == 1) {
        AddCamView((V3){-19.2301f,-42.6604f,-49.7453f},(Quaternion){0.2375f,0.0008f,-0.0002f,0.9713f},75u,256u,256u,2.21f,11.5f);
        AddCamView((V3){7.664583f,-44.88017f,-14.26742f},(Quaternion){0.0f,0.9999f,0.0129f,0.0f},60u,256u,256u,2.192f,20.6f);
    } // TODO other level camviews
    mset(lineSpace,0,LINE_LEN_MAX * sizeof(char));
    u32 lineNum = 0; i32 entCount = -1, lightsIdx = -1;
    char* line;
    while (MmapGetLine(lineSpace, LINE_LEN_MAX)) {
        lineNum++;
        line = lineSpace;
        char* firstColon = StringFindFirstCharWithin(line, ':');
        int firstKeyLen = firstColon ? (int)(firstColon - line) : 0;
        bool isLight = !(firstKeyLen == 10 && sCompUpToLen(line, "constIndex", 10) == 0);
        Entity* inst = NULL;
        Light*   lit  = NULL;
        LightAnimation* lanim = NULL;
        if (isLight) {
            lightsIdx++;
            if (lightsIdx >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n", lightsIdx, curlevel); continue; }
            lit   = &lightsFromFile[lightsIdx];
            lanim = &lanimsFromFile[lightsIdx];
            // Zero this slot only (replaces the full-array mset + full-array lflags init)
            mset(lit, 0, sizeof(Light));
            mset(lanim, 0, sizeof(LightAnimation));
            lit->lflags = LIGHT_AND_SHADOW_ON; // default per-light
        } else {
            entCount++;
            if (entCount >= INSTANCE_COUNT) { DualLogError("Too many instances %u in level%d.txt!\n", entCount, curlevel); continue; }
            inst = &entsFromFile[entCount];
            // Zero this entity slot only
            mset(inst, 0, sizeof(Entity));
            mset(&positionFromFile[entCount], 0, sizeof(V3));
            mset(&scaleFromFile[entCount],    0, sizeof(V3));
            mset(&rotationFromFile[entCount], 0, sizeof(Quaternion));
        }

        bool activeStateRead = false;
        while (line[0] != '\0') {
            char* pipe = StringFindFirstCharWithin(line, '|');
            char* kvString = line;
            if (pipe) { *pipe = '\0'; line = pipe + 1; }
            else       { line += slen(line); }
            if (kvString[0] == '\0') continue;
            char* colon = StringFindFirstCharWithin(kvString, ':');
            if (!colon || colon[1] == '\0') continue;
            *colon = '\0';
            char* key = kvString;
            char* value = colon + 1;
            int keyLen = (int)(colon - key); // length is free, no slen()
            if (isLight) { LoadFieldIntoLight(key,value,lineSpace,lineNum,lit,lanim,lightsIdx);
            } else {
                // Use KEY_EQ for length-aware compares against literals (no slen on key).
                // key/value are used directly instead of trimmed_key/trimmed_value.
                if (KEY_EQ("constIndex"))           inst->index = parse_numberu16(value, lineSpace, lineNum);
                else if (KEY_EQ("localPosition.x")) positionFromFile[entCount].x = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("localPosition.y")) positionFromFile[entCount].y = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("localPosition.z")) positionFromFile[entCount].z = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("localRotation.x")) rotationFromFile[entCount].x = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("localRotation.y")) rotationFromFile[entCount].y = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("localRotation.z")) rotationFromFile[entCount].z = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("localRotation.w")) rotationFromFile[entCount].w = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("localScale.x"))    scaleFromFile[entCount].x = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("localScale.y"))    scaleFromFile[entCount].y = parse_float(value, lineSpace, lineNum);
                else if (KEY_EQ("localScale.z"))    scaleFromFile[entCount].z = parse_float(value, lineSpace, lineNum);
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
                else if (KEY_EQ("alternateOn"))     inst->alternateOn = parse_bool(value, lineSpace, lineNum);
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
                else if (KEY_EQ("changeMatOnActive")) inst->changeTexOnActive = parse_bool(value, lineSpace, lineNum);
                else if (KEY_EQ("blinkWhenActive")) inst->blinkTexOnActive = parse_bool(value, lineSpace, lineNum);
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
                else if (KEY_EQ("frameDelay"))      inst->tickTime = (double)parse_float(value, lineSpace, lineNum);
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
        u16 parent = AddInstance(entIdx,positionFromFile[e]);
        Entity* par = &World.instances[parent];
        par->lastPosition          = positionFromFile[e];
        World.rotation[parent]    = rotationFromFile[e];
        World.scale[parent]       = scaleFromFile[e];
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
        scpy_to_a_from_b(par->target, src->target, TARGET_STRING_LENGTH);
        scpy_to_a_from_b(par->targetname, src->targetname, TARGET_STRING_LENGTH);
        scpy_to_a_from_b(par->texAnimResourceFolder, src->texAnimResourceFolder, TARGET_STRING_LENGTH);
        if (IdxIsPortalBlockingDoor(entIdx)) AddDoorPortal(entIdx,parent); // Only at load, not in AddInstance
        if (entIdx == 525) { // prop_console01
            V3 ofs1 = GetLocalTransformedPos(par,(V3){5.81f,2.29f,38.05f-38.3552f});
            V3 ofs2 = GetLocalTransformedPos(par,(V3){-10.1f,0.9f,18.21f-38.3552f});
            Light lit1 = (Light){.pos=ofs1,.col=(Color3){0.3531f,0.4837f,0.6509f},.range=1.85f,.intensity=0.7f,.maxIntensity=0.7f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
            Light lit2 = (Light){.pos=ofs2,.col=(Color3){0.3561f,0.3561f,0.8970f},.range=2.0f,.intensity=1.12f,.maxIntensity=1.12f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
            LightAnimation lam={0};
            par->texAnimLight  = AddLight(&lit1,&lam);
            par->texAnimLight2 = AddLight(&lit2,&lam);
        } else if (entIdx == 309 || entIdx == 365 || entIdx == 369) { World.position[parent].y += 0.12f; } // item_beaker || item_flask || item_testtube: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 310) { World.position[parent].y += 0.0975f; } // item_beverage: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx >= 472 && entIdx <= 476) { World.position[parent].y += 0.342f; } // se_crate1, se_crate2, se_crate3, se_crate4, se_crate5: Move up to account for CG mod (origin moved vs Unity version)
        else if (entIdx == 279) { // chunk_screen
            V3 ofs1 = GetLocalTransformedPos(par,(V3){0.0f,-0.08f,0.0f});
            Light lit1 = (Light){.pos=ofs1,.col=(Color3){0.909803922f,0.929411765f,1.0f},.range=3.2f,.intensity=1.575f,.maxIntensity=1.575f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
            LightAnimation lam={0};
            par->texAnimLight = AddLight(&lit1,&lam);
        } else if (par->index == 574) { // prop_healingbed
            V3 ofs1 = GetLocalTransformedPos(par,(V3){0.5292511f,0.065f,0.915f});
            V3 ofs2 = GetLocalTransformedPos(par,(V3){-0.5317497f,0.065f,1.039f});
            Light lit1 = (Light){.pos=ofs1,.col=(Color3){0.0f,0.925490196f,0.082352941f},.range=3.0f,.intensity=0.72f,.maxIntensity=0.72f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
            Light lit2 = (Light){.pos=ofs2,.col=(Color3){0.0f,0.925490196f,0.082352941f},.range=3.0f,.intensity=0.72f,.maxIntensity=0.72f,.minIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY,.lflags=LIGHT_AND_SHADOW_ON};
            LightAnimation lam={0};
            par->texAnimLight  = AddLight(&lit1,&lam);
            par->texAnimLight2 = AddLight(&lit2,&lam);
            par->textureAnimating = true; par->texAnimClip = 12; par->texFrame = 0;
            scpy_to_a_from_b(par->texAnimResourceFolder,"MedicalBed",TARGET_STRING_LENGTH);
        } else if (par->index == 746) { par->textureAnimating = true; par->texAnimClip = 2; par->texFrame = 0; } // weapon_grenadeenergmine_live
        else if (entIdx == 720) { /*u16 mist=*/AddInstance(648,World.position[parent]); }// ambient_mist
        else if (entIdx == 733) { /*u16 pipewater=*/AddInstance(649,World.position[parent]);/*ambient_pipewater_loop*/ /*u16 rain=*/AddInstance(653,(V3){World.position[parent].x,World.position[parent].y - 1.26f,World.position[parent].z});/*ambient_rain*/ }
        if (par->texAnimResourceFolder[0] != '\0' && par->tickTime <= 0.01f) par->tickTime = 0.35f;
        TextureSequenceInit(parent, par->texAnimResourceFolder);
    }
    for (int i=0;i<=lightsIdx;++i) { if (!(lightsFromFile[i].lflags & LSPOT)) {lightsFromFile[i].spotAng = 0.0f;} AddLight(&lightsFromFile[i],&lanimsFromFile[i]); }
    if (curlevel == 1 || curlevel == 2 || curlevel == 5 || curlevel == 6 || curlevel == 7) { // Shield generators
        World.shd1 = AddInstance(754, (V3){-51.30664f,  -47.42f,  56.42651f}); World.rotation[World.shd1] = (Quaternion){0.0f,0.0f,0.0f,1.0f};
        World.shd2 = AddInstance(754, (V3){ 71.5f,      -47.42f, -66.6f    }); World.rotation[World.shd2] = (Quaternion){0.0f,0.0f,0.0f,1.0f};
        World.shd3 = AddInstance(754, (V3){-51.306650f, -47.42f, -66.66652f}); World.rotation[World.shd3] = (Quaternion){0.0f,0.0f,0.0f,1.0f};
        World.shd4 = AddInstance(754, (V3){ 71.78664f,  -47.42f,  56.42651f}); World.rotation[World.shd4] = (Quaternion){0.0f,0.0f,0.0f,1.0f};
    } else World.shd1=World.shd2=World.shd3=World.shd4=U16_MAX;
    Light hl = (Light){.pos=World.position[PLAYER1],.col=(Color3){1.0f,1.0f,1.0f},.range=11.52f,.lflags=LIGHTON,.intensity=0.0f,.minIntensity=0.0f,.maxIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY};
    LightAnimation lam = {0};
    headmountedLanternLight = AddLight(&hl, &lam);
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
    World.curLev = curlevel;
    SetLevelPointers(curlevel); // Ensures writing to correct current level
    mset(World.instances + 3,0,(INSTANCE_COUNT - 3) * sizeof(Entity)); // Clear previous level slots. Claimed slots are fully initialized by AddInstance().
    World.instCount = 3; // 0 == NULL, 1 == Player1, 2 == Player2
    mset(World.lights,0,LIGHT_COUNT * sizeof(Light)); mset(World.lanims,0,LIGHT_COUNT * sizeof(LightAnimation)); World.loadedLights=0; mset(alreadyReadLightOnOnce,0,sizeof(alreadyReadLightOnOnce));
    mset(camViews,0,64 * sizeof(CamView)); camViewCount=0;
    char filename[20]; // Minimum size for 0 through 13.
    sFormat(filename, sizeof(filename), "./Data/level%d.txt", curlevel);
    FHandle fh; int fsize; void* fbuf = OS_OpenAndAllocateFileBufferReadonly(filename, &fh, &fsize); if (!fbuf) { OS_Exit(1); }
    mm_ptr = (const char*)fbuf;
    mm_end = mm_ptr + fsize;
    LoadLevelMod(curlevel);
    OS_Free(fbuf, (size_t)fsize);
    for (int i = 0; i < World.loadedLights; ++i) World.lightsNewPosition[i] = World.lights[i].pos;
    for (int i = PLAYER1; i < World.instCount; ++i) {
        i32 cellIdx = PosGetCellCoords(World.position[i].x, World.position[i].z);
        World.instances[i].cellIndex = cellIdx; World.instances[i].cellX = PosGetCellCoordX(World.position[i].x); World.instances[i].cellZ = PosGetCellCoordZ(World.position[i].z);
        World.radius[i] = modelBounds[World.instances[i].modelIndex] * vmax(vmax(World.scale[i].x, World.scale[i].y), World.scale[i].z);
        World.instances[i].shadRadius = World.radius[i] * 1.70f;
        ComputeConvexMeshInertiaTensor(i);
        if (World.mass[i] < 0.001f && World.collider[i] != COLTYPE_NONE && World.collider[i] != COLTYPE_MSH && (World.instances[i].entflags & EF_RIGIDBODY)) { World.mass[i]=0.2f;/*At least something!*/ }
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
                AnimationClip c = DoorGetClip(&World.instances[i],DOOR_CLIP_OPENING);
                DoorSetClipFrame(i,DOOR_CLIP_OPENING,DoorFrameFromProgress(c,World.instances[i].ajarPercentage));
                World.instances[i].doorOpen = World.instances[i].doorState = DoorState_Opening;
                return;
            }
            switch (World.instances[i].doorOpen) {
                case DoorState_Open:    DoorSetClipFrame(i,DOOR_CLIP_IDLE_OPEN,DoorGetClip(&World.instances[i],DOOR_CLIP_IDLE_OPEN).frameStart); break;
                case DoorState_Opening: DoorSetClipFrame(i,DOOR_CLIP_OPENING,DoorFrameFromProgress(DoorGetClip(&World.instances[i],DOOR_CLIP_OPENING),0.0f/*TODO percent of anim*/)); break;
                case DoorState_Closing: DoorSetClipFrame(i,DOOR_CLIP_CLOSING,DoorFrameFromProgress(DoorGetClip(&World.instances[i],DOOR_CLIP_CLOSING),0.0f/*TODO percent of anim*/)); break;
                default:                DoorSetClipFrame(i,DOOR_CLIP_IDLE_CLOSED,DoorGetClip(&World.instances[i],DOOR_CLIP_IDLE_CLOSED).frameStart); break;
            }
        } else if (IdxIsNPC(constIndex)) { World.layer[i] = L_NPC; /* TODO AIInit funcion */ }
        else if (IdxIsSearchable(constIndex)) {
            if (World.instances[i].generateContents) {
                int numRandomGeneratedItems = 0;
                for(int j=0;j<4;j++) {
                    if (World.instances[i].randomItemDropChance[j] <= 0.0f) continue;
                    u8 tempInt = random_range_u8(0,100);
                    if (((float)tempInt / 100.0f) <= World.instances[i].randomItemDropChance[j]) { World.instances[i].contents[numRandomGeneratedItems] = World.instances[i].randomItem[j]; numRandomGeneratedItems++; if (numRandomGeneratedItems > World.instances[i].maxRandomItems) {break;} }
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

void RenderLoading(i32 offset, const char * restrict text);
void LoadAllLevels() {
    double start_time = get_time();
    DebugRAM("start of LoadAllLevels");
    RenderLoading(100,"Loading level data...");
    World.levelCurrentlyLoading = true;
    for (u8 lev = 0; lev < World.numLevels; ++lev) LoadLevelData(lev);
    DualLog("Entity counts::0:%u|1:%u|2:%u|3:%u|4:%u|5:%u|6:%u|7:%u|8:%u|9:%u|10:%u|11:%u|12:%u|13:%u\n Light counts::0:%u|1:%u|2:%u|3:%u|4:%u|5:%u|6:%u|7:%u|8:%u|9:%u|10:%u|11:%u|12:%u|13:%u\nLoad all levels... took %f secs\n",
            World.levelInstCount[0],World.levelInstCount[1],World.levelInstCount[2],World.levelInstCount[3],World.levelInstCount[4],World.levelInstCount[5],World.levelInstCount[6],World.levelInstCount[7],World.levelInstCount[8],World.levelInstCount[9],World.levelInstCount[10],World.levelInstCount[11],World.levelInstCount[12],World.levelInstCount[13],
            World.levelLoadedLights[0],World.levelLoadedLights[1],World.levelLoadedLights[2],World.levelLoadedLights[3],World.levelLoadedLights[4],World.levelLoadedLights[5],World.levelLoadedLights[6],World.levelLoadedLights[7],World.levelLoadedLights[8],World.levelLoadedLights[9],World.levelLoadedLights[10],World.levelLoadedLights[11],World.levelLoadedLights[12],World.levelLoadedLights[13],
            get_time() - start_time);
    DebugRAM("end of LoadAllLevels");
}

void ResetLevelAudio(); void ResetLevelMusic(); void CullInit();
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
    mset(modelMatrices,0,INSTANCE_COUNT * 16 * sizeof(float));
    for (int i=0;i<World.loadedLights;++i) World.lightsNewPosition[i]=World.lights[i].pos;
    DualLog("Switched to Level %d\n",curlevel);
    ResetLevelAudio(); ResetLevelMusic();
    RenderLoading(110,"Loading cull system..."); CullInit(); // Must be after level!
    glUseProgram(voxelUpdateSP); glUniform2f(0,World.voxMinCtrX[World.curLev],World.voxMinCtrZ[World.curLev]); glUniform1f(1,World.farPlane[World.curLev] * World.farPlane[World.curLev]); glUniform1ui(2,World.loadedLights); glUniform2f(3,World.worldMin_x[World.curLev],World.worldMin_z[World.curLev]);
    RenderLoading(120,"Loading voxel lighting data...");
    for (u16 i = 0; i < World.loadedLights; i++) { World.lightsNewPosition[i] = World.lights[i].pos; }
    mset(shadowmapIndirectionList,MAX_SHADOWMAPS + 1,World.loadedLights * sizeof(u32)); // Set to invalid values for all
    World.levelCurrentlyLoading = false;
    SetPosition(PLAYER1,pos,true);
    DebugRAM("end of LoadLevel");
}
// Save Game System
INLINE size_t GetMaxCompressedSize(size_t srcSize) { return srcSize + (srcSize / 128) + 16; } // Worst-case buffer size for allocation
size_t VoidSquasher(const u8* src, size_t srcSize, u8* dst, size_t dstCapacity) { // Find and pop the zeroes bubbles.  Turns an otherwise 232mb save file into ~23mb.
    size_t s = 0, d = 0;
    while (s < srcSize) { // 1. Hunt for Zeros
        size_t zeroCount = 0;
        while (s + zeroCount < srcSize && src[s + zeroCount] == 0) { zeroCount++; }
        if (zeroCount > 0) {
            if (zeroCount < 128) { if (d >= dstCapacity){return 0;} dst[d++] = (u8)(0x80 + (zeroCount - 1)); }
            else { if (d + 5 > dstCapacity) {return 0;} dst[d++] = 0xFF; u32 zCount32=(u32)zeroCount; mcpy(&dst[d],&zCount32,sizeof(u32)); d += 4; }
            s += zeroCount; continue; // Go back to hunting zeros
        }
        
        size_t litCount = 0; // 2. Process Literal Data (Non-Zeros). It costs 2 bytes of overhead to break a literal run to compress 1 or 2 zeros. Only break a literal run if 3 or more zeros ahead.
        while (s + litCount < srcSize && litCount < 128) {
            if (src[s + litCount] == 0) { size_t remain = srcSize - (s + litCount); if (remain >= 3 && src[s + litCount + 1] == 0 && src[s + litCount + 2] == 0) { break; } /*Found a juicy patch of zeros, break the literal run!*/ }
            litCount++;
        }
        if (litCount > 0) { if (d + 1 + litCount > dstCapacity) {return 0;} dst[d++] = (u8)(litCount - 1); mcpy(&dst[d], &src[s], litCount); s += litCount; d += litCount; }
    }
    return d; // Return final compressed size
}

static size_t BlowBubblesOfVoid(const u8* src, size_t srcSize, u8* dst, size_t dstCapacity) { // Put the bubbles of zero back.
    size_t s = 0, d = 0;
    while (s < srcSize && d < dstCapacity) {
        u8 cmd = src[s++];
        if (cmd < 128) { // Literal Run
            size_t litCount = cmd + 1;
            if (s + litCount > srcSize || d + litCount > dstCapacity) return 0; 
            mcpy(&dst[d], &src[s], litCount); s += litCount; d += litCount;
        } else if (cmd < 0xFF) { // Short Zero Run
            size_t zeroCount = cmd - 128 + 1;
            if (d + zeroCount > dstCapacity) return 0;
            mset(&dst[d], 0, zeroCount); d += zeroCount;
        } else { // Long Zero Run
            if (s + 4 > srcSize) return 0;
            u32 zeroCount;
            mcpy(&zeroCount, &src[s], sizeof(u32));
            s += 4;
            if (d + zeroCount > dstCapacity) return 0;
            mset(&dst[d], 0, zeroCount); d += zeroCount;
        }
    }
    return d;
}

#pragma pack(push, 1)
typedef struct { u32 magicNumber; u32 version; u32 uncompressedSize; u32 compressedSize; char savename[48]; } SaveHeader;
#pragma pack(pop)
void SaveGame(u8 slot, const char* savename) {
    if (slot > 7) return;
    char path[] = "./Data/sav0.bin"; path[10] = '0' + slot;
    FHandle fd = OS_OpenWriteonly(path); if (fd == (FHandle)-1) return;
    // Allocate memory for the compression buffer
    size_t uncompressedSize = sizeof(GlobalContext);
    size_t maxCompSize = GetMaxCompressedSize(uncompressedSize);
    u8* compBuffer = (u8*)OS_Alloc(maxCompSize);
    size_t finalCompSize = VoidSquasher((const u8*)&World,uncompressedSize,compBuffer,maxCompSize);
    if (finalCompSize > 0) {
        SaveHeader header = {.magicNumber=0x56415343/*'CSAV'*/, .version=2, .uncompressedSize=(u32)uncompressedSize, .compressedSize=(u32)finalCompSize};
        if (savename) { int i=0;   while(savename[i] != '\0' && i < 47){header.savename[i]=savename[i]; i++;}   header.savename[i]='\0'; }
        World.justSavedTimeStamp = get_time(); OS_Write(fd,&header,sizeof(SaveHeader),path); OS_Write(fd,compBuffer,finalCompSize,path); CenterStatusPrint("Saved to Slot %d",slot);
    } else { DualLogError("Compression failed during SaveGame!\n"); }
    OS_Free(compBuffer,maxCompSize); OS_Close(fd);
}

void LoadGame(u8 slot) {
    if (slot > 7) return;
    char path[] = "./Data/sav0.bin"; path[10] = '0' + slot;
    FHandle fd = OS_OpenReadonly(path); if (fd == (FHandle)-1) return;
    SaveHeader header;
    if (OS_Read(fd, &header, sizeof(SaveHeader)) != sizeof(SaveHeader) || header.magicNumber != 0x56415343) { DualLogError("Corrupted save file header!\n"); OS_Close(fd); return; }
    // Allocate memory to read the compressed file
    u8* compBuffer = (u8*)OS_Alloc(header.compressedSize);
    if (OS_Read(fd,compBuffer,header.compressedSize) == (long)header.compressedSize) {
        size_t result = BlowBubblesOfVoid(compBuffer, header.compressedSize, (u8*)&World, header.uncompressedSize); // Decompress straight into the World struct
        if (result == header.uncompressedSize) { SetLevelPointers(World.currentLevel); CenterStatusPrint("Loaded Game: %s", header.savename); }
        else { DualLogError("Decompression failed! Expected %u bytes, got %u\n", header.uncompressedSize, (u32)result); }
    }
    OS_Free(compBuffer,header.compressedSize); OS_Close(fd);
    for (int i=0;i<World.loadedLights;++i) { flag_set(&World.lights[i].lflags,LDIRTY,true); }
}

u8 GetCurrentLevelSecurity() { return (World.diffMis < 1 || Cheats.superoverride) ? 0u : World.levelSecurity[World.curLev]; }
