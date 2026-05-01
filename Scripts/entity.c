#include "mod.h"
#define LINE_LEN_MAX 81920
Entity EntityDefinitions[MAX_ENTITIES] = {
    {.path="chunk_black",.index=0,.modelIndex=178,.cardchunk=true,.texIndex=0,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,1.44f,0.0f},.colliderSize=(Vector3){2.56f,0.32f,2.56f},.colliderMeshIndex=U16_MAX},
    {.path="chunk_blocker",.index=1,.modelIndex=178,.cardchunk=true,.texIndex=1230,.normIndex=160,.specIndex=1230,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,1.44f,0.0f},.colliderSize=(Vector3){2.56f,0.32f,2.56f},.colliderMeshIndex=U16_MAX},
    {.path="chunk_bridg1_1",.index=2,.modelIndex=661,.cardchunk=true,.texIndex=44,.normIndex=43,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,1.54f,0.0f},.colliderSize=(Vector3){2.56f,0.32f,2.56f},.colliderMeshIndex=U16_MAX},
    {.path="chunk_bridg1_1flipx",.index=3,.modelIndex=667,.cardchunk=true,.texIndex=44,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,1.54f,0.0f},.colliderSize=(Vector3){2.56f,0.32f,2.56f},.colliderMeshIndex=U16_MAX},
    {.path="chunk_bridg1_2",.index=4,.modelIndex=662,.cardchunk=true,.texIndex=45,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,1.44f,0.0f},.colliderSize=(Vector3){2.56f,0.32f,2.56f},.colliderMeshIndex=U16_MAX},
    {.path="chunk_bridg1_3",.index=5,.modelIndex=20,.cardchunk=true,.texIndex=47,.collider=COLLIDER_TYPE_MESH,.colliderMeshIndex=20},
    {.path="chunk_bridg1_3_slice45",.index=6,.modelIndex=21,.texIndex=47},
    {.path="chunk_bridg1_3flipx",.index=7,.modelIndex=663,.cardchunk=true,.texIndex=47},
    {.path="chunk_bridg1_4",.index=8,.modelIndex=22,.cardchunk=true,.texIndex=48},
    {.path="chunk_bridg1_4_slice32",.index=9,.modelIndex=23,.texIndex=48},
    {.path="chunk_bridg1_4_slice32flipx",.index=10,.modelIndex=24,.texIndex=48},
    {.path="chunk_bridg1_5",.index=11,.modelIndex=25,.cardchunk=true,.texIndex=50,.glowIndex=49},
    {.path="chunk_bridg2_2",.index=12,.modelIndex=26,.cardchunk=true,.texIndex=53},
    {.path="chunk_bridg2_3",.index=13,.modelIndex=27,.cardchunk=true,.texIndex=56,.glowIndex=54,.normIndex=55},
    {.path="chunk_bridg2_4",.index=14,.modelIndex=28,.cardchunk=true,.texIndex=57},
    {.path="chunk_bridg2_5",.index=15,.modelIndex=29,.cardchunk=true,.texIndex=59,.normIndex=58},
    {.path="chunk_bridg2_6",.index=16,.modelIndex=30,.cardchunk=true,.texIndex=60},
    {.path="chunk_bridg2_7",.index=17,.modelIndex=664,.cardchunk=true,.texIndex=61},
    {.path="chunk_bridg2_8",.index=18,.modelIndex=31,.cardchunk=true,.texIndex=62},
    {.path="chunk_bridg2_9",.index=19,.modelIndex=32,.cardchunk=true,.texIndex=64,.glowIndex=63},
    {.path="chunk_crate_impenetrable",.index=20,.modelIndex=61,.texIndex=150},
    {.path="chunk_cyberpanel",.index=21,.modelIndex=178,.cardchunk=true,.texIndex=151,.glowIndex=151},
    {.path="chunk_cyberpanel_slice45",.index=22,.modelIndex=180,.texIndex=152,.glowIndex=152},
    {.path="chunk_eng1_1",.index=23,.modelIndex=96,.cardchunk=true,.texIndex=254},
    {.path="chunk_eng1_1d",.index=24,.modelIndex=95,.cardchunk=true,.texIndex=253},
    {.path="chunk_eng1_2",.index=25,.modelIndex=98,.cardchunk=true,.texIndex=256},
    {.path="chunk_eng1_2d",.index=26,.modelIndex=97,.cardchunk=true,.texIndex=255},
    {.path="chunk_eng1_3",.index=27,.modelIndex=100,.cardchunk=true,.texIndex=259,.glowIndex=258},
    {.path="chunk_eng1_3d",.index=28,.modelIndex=99,.cardchunk=true,.texIndex=257},
    {.path="chunk_eng1_4",.index=29,.modelIndex=101,.cardchunk=true,.texIndex=260},
    {.path="chunk_eng1_5",.index=30,.modelIndex=103,.cardchunk=true,.texIndex=262},
    {.path="chunk_eng1_5_slice45lh",.index=31,.modelIndex=104,.texIndex=262},
    {.path="chunk_eng1_5_slice45rh",.index=32,.modelIndex=105,.texIndex=262},
    {.path="chunk_eng1_5d",.index=33,.modelIndex=102,.cardchunk=true,.texIndex=261},
    {.path="chunk_eng1_6",.index=34,.modelIndex=107,.cardchunk=true,.texIndex=266,.glowIndex=265},
    {.path="chunk_eng1_6d",.index=35,.modelIndex=106,.cardchunk=true,.texIndex=264,.glowIndex=263},
    {.path="chunk_eng1_7",.index=36,.modelIndex=108,.cardchunk=true,.texIndex=269,.glowIndex=268},
    {.path="chunk_eng1_7d",.index=37,.modelIndex=665,.cardchunk=true,.texIndex=267},
    {.path="chunk_eng1_8",.index=38,.modelIndex=109,.cardchunk=true,.texIndex=271,.glowIndex=270},
    {.path="chunk_eng1_9",.index=39,.modelIndex=111,.cardchunk=true,.texIndex=273,.glowIndex=251},
    {.path="chunk_eng1_9d",.index=40,.modelIndex=110,.cardchunk=true,.texIndex=272},
    {.path="chunk_eng2_1",.index=41,.modelIndex=113,.cardchunk=true,.texIndex=276},
    {.path="chunk_eng2_1_slice45",.index=42,.modelIndex=116,.texIndex=276},
    {.path="chunk_eng2_1_slice384high",.index=43,.modelIndex=114,.texIndex=276},
    {.path="chunk_eng2_1_slice384highrh",.index=44,.modelIndex=115,.texIndex=276},
    {.path="chunk_eng2_1d",.index=45,.modelIndex=112,.cardchunk=true,.texIndex=275,.glowIndex=274},
    {.path="chunk_eng2_2",.index=46,.modelIndex=117,.cardchunk=true,.texIndex=279},
    {.path="chunk_eng2_2d",.index=47,.modelIndex=666,.cardchunk=true,.texIndex=277},
    {.path="chunk_eng2_3",.index=48,.modelIndex=119,.cardchunk=true,.texIndex=282},
    {.path="chunk_eng2_3d",.index=49,.modelIndex=118,.cardchunk=true,.texIndex=281},
    {.path="chunk_eng2_4",.index=50,.modelIndex=178,.cardchunk=true,.texIndex=283},
    {.path="chunk_eng2_5",.index=51,.modelIndex=120,.cardchunk=true,.texIndex=285,.normIndex=284},
    {.path="chunk_eng2_5_slice45",.index=52,.modelIndex=121,.texIndex=285,.normIndex=284},
    {.path="chunk_eng2_6",.index=53,.modelIndex=0,.texIndex=141,.glowIndex=142,.numclips=1,.animationNum=21},
    {.path="chunk_exec1_1",.index=54,.modelIndex=124,.cardchunk=true,.texIndex=287},
    {.path="chunk_exec1_1d",.index=55,.modelIndex=123,.cardchunk=true,.texIndex=286},
    {.path="chunk_exec1_2",.index=56,.modelIndex=126,.cardchunk=true,.texIndex=291,.glowIndex=290},
    {.path="chunk_exec1_2d",.index=57,.modelIndex=125,.cardchunk=true,.texIndex=289,.glowIndex=288},
    {.path="chunk_exec2_1",.index=58,.modelIndex=127,.cardchunk=true,.texIndex=292},
    {.path="chunk_exec2_2",.index=59,.modelIndex=129,.cardchunk=true,.texIndex=295},
    {.path="chunk_exec2_2d",.index=60,.modelIndex=128,.cardchunk=true,.texIndex=294,.glowIndex=293},
    {.path="chunk_exec2_3",.index=61,.modelIndex=130,.cardchunk=true,.texIndex=296},
    {.path="chunk_exec2_4",.index=62,.modelIndex=131,.cardchunk=true,.texIndex=297},
    {.path="chunk_exec2_4_slice45",.index=63,.modelIndex=132,.texIndex=297},
    {.path="chunk_exec2_5",.index=64,.modelIndex=133,.cardchunk=true,.texIndex=298,.specIndex=1257},
    {.path="chunk_exec2_6",.index=65,.modelIndex=134,.cardchunk=true,.texIndex=299,.specIndex=1257},
    {.path="chunk_exec2_7",.index=66,.modelIndex=133,.cardchunk=true,.texIndex=300,.specIndex=1257},
    {.path="chunk_exec3_1",.index=67,.modelIndex=127,.cardchunk=true,.texIndex=303},
    {.path="chunk_exec3_1d",.index=68,.modelIndex=135,.cardchunk=true,.texIndex=302,.glowIndex=301},
    {.path="chunk_exec3_2",.index=69,.modelIndex=129,.cardchunk=true,.texIndex=304},
    {.path="chunk_exec3_4",.index=70,.modelIndex=178,.cardchunk=true,.texIndex=305},
    {.path="chunk_exec4_1",.index=71,.modelIndex=136,.cardchunk=true,.texIndex=307,.glowIndex=306},
    {.path="chunk_exec4_2",.index=72,.modelIndex=137,.texIndex=308,.cardchunk=true},
    {.path="chunk_exec4_3",.index=73,.modelIndex=138,.cardchunk=true,.texIndex=309},
    {.path="chunk_exec4_4",.index=74,.modelIndex=139,.cardchunk=true,.texIndex=311},
    {.path="chunk_exec4_5",.index=75,.modelIndex=178,.cardchunk=true,.texIndex=312},
    {.path="chunk_exec4_6",.index=76,.modelIndex=141,.cardchunk=true,.texIndex=313},
    {.path="chunk_exec6_1",.index=77,.modelIndex=142,.cardchunk=true,.texIndex=315,.glowIndex=314},
    {.path="chunk_exteriorpanel1",.index=78,.modelIndex=131,.cardchunk=true,.texIndex=1228},
    {.path="chunk_fan1",.index=79,.modelIndex=0,.numclips=1,.animationNum=22,.cardchunk=true,.texIndex=96,.glowIndex=192},
    {.path="chunk_flight1_1",.index=80,.modelIndex=146,.cardchunk=true,.texIndex=319},
    {.path="chunk_flight1_1b",.index=81,.modelIndex=146,.cardchunk=true,.texIndex=318},
    {.path="chunk_flight1_2",.index=82,.modelIndex=147,.cardchunk=true,.texIndex=320},
    {.path="chunk_flight1_2_slice45rh",.index=83,.modelIndex=149,.texIndex=320},
    {.path="chunk_flight1_3",.index=84,.modelIndex=150,.cardchunk=true,.texIndex=321},
    {.path="chunk_flight1_4",.index=85,.modelIndex=151,.cardchunk=true,.texIndex=322},
    {.path="chunk_flight1_5",.index=86,.modelIndex=147,.cardchunk=true,.texIndex=323},
    {.path="chunk_flight1_5_slice45lh",.index=87,.modelIndex=148,.texIndex=323},
    {.path="chunk_flight1_6",.index=88,.modelIndex=152,.cardchunk=true,.texIndex=325},
    {.path="chunk_flight2_1",.index=89,.modelIndex=153,.cardchunk=true,.texIndex=326},
    {.path="chunk_flight2_2",.index=90,.modelIndex=154,.cardchunk=true,.texIndex=327},
    {.path="chunk_flight2_2_slice45",.index=91,.modelIndex=155,.texIndex=327},
    {.path="chunk_flight2_3",.index=92,.modelIndex=156,.cardchunk=true,.texIndex=328},
    {.path="chunk_grove1_1",.index=93,.modelIndex=189,.cardchunk=true,.texIndex=362},
    {.path="chunk_grove1_2",.index=94,.modelIndex=178,.cardchunk=true,.texIndex=363},
    {.path="chunk_grove1_2_slice45",.index=95,.modelIndex=180,.texIndex=363},
    {.path="chunk_grove1_3",.index=96,.modelIndex=178,.texIndex=364},
    {.path="chunk_grove1_4",.index=97,.modelIndex=178,.cardchunk=true,.texIndex=365},
    {.path="chunk_grove1_5",.index=98,.modelIndex=178,.cardchunk=true,.texIndex=367},
    {.path="chunk_grove1_6",.index=99,.modelIndex=178,.cardchunk=true,.texIndex=368},
    {.path="chunk_grove1_7",.index=100,.modelIndex=178,.cardchunk=true,.texIndex=369},
    {.path="chunk_grove2_1",.index=101,.modelIndex=190,.cardchunk=true,.texIndex=370},
    {.path="chunk_grove2_2",.index=102,.modelIndex=190,.cardchunk=true,.texIndex=371},
    {.path="chunk_grove2_3",.index=103,.modelIndex=191,.texIndex=372},
    {.path="chunk_grove2_4",.index=104,.modelIndex=341,.cardchunk=true,.texIndex=374,.glowIndex=373},
    {.path="chunk_grove2_5",.index=105,.modelIndex=192,.cardchunk=true,.texIndex=375},
    {.path="chunk_grove2_6",.index=106,.modelIndex=192,.cardchunk=true,.texIndex=376},
    {.path="chunk_grove2_7",.index=107,.modelIndex=191,.cardchunk=true,.texIndex=378},
    {.path="chunk_grove2_8",.index=108,.modelIndex=191,.cardchunk=true,.texIndex=379},
    {.path="chunk_grove2_9",.index=109,.modelIndex=191,.cardchunk=true,.texIndex=385},
    {.path="chunk_grove2_9b",.index=110,.modelIndex=191,.cardchunk=true,.texIndex=381},
    {.path="chunk_grove2_9c",.index=111,.modelIndex=191,.cardchunk=true,.texIndex=383},
    {.path="chunk_lift1",.index=112,.modelIndex=213,.cardchunk=true,.texIndex=1246,.glowIndex=1247},
    {.path="chunk_maint1_1",.index=113,.modelIndex=218,.cardchunk=true,.texIndex=430},
    {.path="chunk_maint1_2",.index=114,.modelIndex=220,.cardchunk=true,.texIndex=432},
    {.path="chunk_maint1_2d",.index=115,.modelIndex=219,.cardchunk=true,.texIndex=431},
    {.path="chunk_maint1_3",.index=116,.modelIndex=222,.cardchunk=true,.texIndex=436,.glowIndex=435,.specIndex=437},
    {.path="chunk_maint1_3b",.index=117,.modelIndex=221,.cardchunk=true,.texIndex=434,.glowIndex=433},
    {.path="chunk_maint1_4",.index=118,.modelIndex=224,.cardchunk=true,.texIndex=441,.glowIndex=440},
    {.path="chunk_maint1_4b",.index=119,.modelIndex=223,.cardchunk=true,.texIndex=439,.glowIndex=438},
    {.path="chunk_maint1_5",.index=120,.modelIndex=225,.cardchunk=true,.texIndex=443,.glowIndex=442},
    {.path="chunk_maint1_6",.index=121,.modelIndex=226,.cardchunk=true,.texIndex=96},
    {.path="chunk_maint1_7",.index=122,.modelIndex=227,.cardchunk=true,.texIndex=447,.glowIndex=446},
    {.path="chunk_blockerflightbay",.index=123,.modelIndex=178,.normIndex=160,.texIndex=1230,.specIndex=1242,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,1.44f,0.0f},.colliderSize=(Vector3){2.56f,0.32f,2.56f},.colliderMeshIndex=U16_MAX},
    {.path="chunk_maint1_9",.index=124,.modelIndex=606,.cardchunk=true,.texIndex=450},
    {.path="chunk_maint1_9d",.index=125,.modelIndex=620,.cardchunk=true,.texIndex=449,.glowIndex=448},
    {.path="chunk_maint2_1",.index=126,.modelIndex=230,.cardchunk=true,.texIndex=455},
    {.path="chunk_maint2_1b",.index=127,.modelIndex=228,.texIndex=451},
    {.path="chunk_maint2_1d",.index=128,.modelIndex=229,.cardchunk=true,.texIndex=453,.glowIndex=452},
    {.path="chunk_maint2_2",.index=129,.modelIndex=230,.cardchunk=true,.texIndex=457},
    {.path="chunk_maint2_3",.index=130,.modelIndex=232,.cardchunk=true,.texIndex=460},
    {.path="chunk_maint2_3d",.index=131,.modelIndex=231,.cardchunk=true,.texIndex=459,.glowIndex=458},
    {.path="chunk_maint2_4",.index=132,.modelIndex=233,.cardchunk=true,.texIndex=464,.glowIndex=463},
    {.path="chunk_maint2_4d",.index=133,.modelIndex=233,.cardchunk=true,.texIndex=462,.glowIndex=461},
    {.path="chunk_maint2_5",.index=134,.modelIndex=235,.cardchunk=true,.texIndex=468,.glowIndex=467},
    {.path="chunk_maint2_5d",.index=135,.modelIndex=234,.cardchunk=true,.texIndex=466,.glowIndex=465},
    {.path="chunk_maint2_6",.index=136,.modelIndex=236,.cardchunk=true,.texIndex=472,.glowIndex=471},
    {.path="chunk_maint2_6d",.index=137,.modelIndex=238,.cardchunk=true,.texIndex=470,.glowIndex=470},
    {.path="chunk_maint2_7",.index=138,.modelIndex=238,.cardchunk=true,.texIndex=476,.glowIndex=475},
    {.path="chunk_maint2_7d",.index=139,.modelIndex=237,.cardchunk=true,.texIndex=474,.glowIndex=473},
    {.path="chunk_maint2_8",.index=140,.modelIndex=239,.cardchunk=true,.texIndex=478,.glowIndex=477},
    {.path="chunk_maint2_9",.index=141,.modelIndex=240,.cardchunk=true,.texIndex=480,.glowIndex=479},
    {.path="chunk_maint2_9_slice45RH",.index=142,.modelIndex=242,.texIndex=480,.glowIndex=479},
    {.path="chunk_maint2_9_slice128_top",.index=143,.modelIndex=241,.texIndex=480,.glowIndex=479},
    {.path="chunk_maint3_1",.index=144,.modelIndex=244,.cardchunk=true,.texIndex=483},
    {.path="chunk_maint3_1_slice32_lh",.index=145,.modelIndex=246,.texIndex=483},
    {.path="chunk_maint3_1_slice32_rh",.index=146,.modelIndex=245,.texIndex=483},
    {.path="chunk_maint3_1_slice45",.index=147,.modelIndex=247,.texIndex=483},
    {.path="chunk_maint3_1d",.index=148,.modelIndex=243,.cardchunk=true,.texIndex=482,.glowIndex=481},
    {.path="chunk_med1_1",.index=149,.modelIndex=249,.cardchunk=true,.texIndex=486,.specIndex=1256,.normIndex=1255},
    {.path="chunk_med1_1_half_top",.index=150,.modelIndex=250,.texIndex=486,.specIndex=1256,.normIndex=1255},
    {.path="chunk_med1_1_slice128high",.index=151,.modelIndex=251,.texIndex=486,.specIndex=1256,.normIndex=1255},
    {.path="chunk_med1_1_slice192RH",.index=152,.modelIndex=252,.texIndex=486,.specIndex=1256,.normIndex=1255},
    {.path="chunk_med1_1_slice256",.index=153,.modelIndex=253,.texIndex=486,.specIndex=1256,.normIndex=1255},
    {.path="chunk_med1_1d",.index=154,.modelIndex=248,.cardchunk=true,.texIndex=485,.glowIndex=484,.specIndex=1236,.normIndex=1255},
    {.path="chunk_med1_2",.index=155,.modelIndex=255,.cardchunk=true,.texIndex=489,.glowIndex=488,.specIndex=1256},
    {.path="chunk_med1_2d",.index=156,.modelIndex=254,.cardchunk=true,.texIndex=487,.specIndex=1256},
    {.path="chunk_med1_3",.index=157,.modelIndex=257,.cardchunk=true,.texIndex=493,.glowIndex=492,.specIndex=1256},
    {.path="chunk_med1_3d",.index=158,.modelIndex=256,.cardchunk=true,.texIndex=491,.glowIndex=490,.specIndex=1256},
    {.path="chunk_med1_4",.index=159,.modelIndex=258,.cardchunk=true,.texIndex=494,.specIndex=1256},
    {.path="chunk_med1_5",.index=160,.modelIndex=669,.cardchunk=true,.texIndex=495,.specIndex=1256},
    {.path="chunk_med1_6",.index=161,.modelIndex=259,.cardchunk=true,.texIndex=496,.normIndex=509,.specIndex=1256},
    {.path="chunk_med1_7",.index=162,.modelIndex=262,.cardchunk=true,.texIndex=499,.specIndex=1268,.normIndex=498},
    {.path="chunk_med1_7_slice14_64",.index=163,.modelIndex=263,.texIndex=499,.specIndex=1268,.normIndex=1254},
    {.path="chunk_med1_7_slice45_320lh",.index=164,.modelIndex=264,.texIndex=499,.specIndex=1268,.normIndex=1254},
    {.path="chunk_med1_7_slice45_320rh",.index=165,.modelIndex=265,.texIndex=499,.specIndex=1268,.normIndex=1254},
    {.path="chunk_med1_7_slice96high",.index=166,.modelIndex=266,.texIndex=499,.specIndex=1268,.normIndex=1254},
    {.path="chunk_med1_7d",.index=167,.modelIndex=260,.cardchunk=true,.texIndex=497,.specIndex=1269,.normIndex=1270},
    {.path="chunk_med1_7d_slice128",.index=168,.modelIndex=261,.texIndex=497,.specIndex=1269,.normIndex=1270},
    {.path="chunk_med1_8",.index=169,.modelIndex=268,.cardchunk=true,.texIndex=503,.normIndex=502,.specIndex=1242},
    {.path="chunk_med1_8d",.index=170,.modelIndex=267,.cardchunk=true,.texIndex=501,.normIndex=163,.specIndex=1242},
    {.path="chunk_med1_9",.index=171,.modelIndex=278,.cardchunk=true,.texIndex=507,.normIndex=506,.specIndex=1267},
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 172 unused slot
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 173 empty
    {.path="chunk_med1_9d",.index=174,.modelIndex=269,.cardchunk=true,.texIndex=505,.normIndex=504,.specIndex=1267},
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 175 nothin to see here
    {.path="chunk_med1_9d_ofs112_90",.index=176,.modelIndex=270,.texIndex=505,.normIndex=504,.specIndex=1267,.collider=COLLIDER_TYPE_MESH,.colliderMeshIndex=270},
    {.path="chunk_med1_9d_ofs144_90",.index=177,.modelIndex=272,.texIndex=505,.normIndex=504,.specIndex=1267,.collider=COLLIDER_TYPE_MESH,.colliderMeshIndex=272},
    {.path="chunk_med2_1",.index=178,.modelIndex=280,.cardchunk=true,.texIndex=513,.specIndex=1254,.glowIndex=511,.normIndex=512},
    {.path="chunk_med2_1_slice32RH",.index=179,.modelIndex=281,.texIndex=513,.normIndex=512,.specIndex=1254},
    {.path="chunk_med2_1d",.index=180,.modelIndex=279,.glowIndex=508,.texIndex=510,.specIndex=1254},
    {.path="chunk_med2_2",.index=181,.modelIndex=283,.cardchunk=true,.texIndex=517,.glowIndex=516,.specIndex=1242},
    {.path="chunk_med2_2_half_bottom",.index=182,.modelIndex=284,.texIndex=517,.glowIndex=516,.specIndex=1242},
    {.path="chunk_med2_2d",.index=183,.modelIndex=282,.cardchunk=true,.texIndex=515,.glowIndex=516,.specIndex=1242},
    {.path="chunk_med2_3",.index=184,.modelIndex=286,.cardchunk=true,.texIndex=521,.glowIndex=520,.specIndex=1242},
    {.path="chunk_med2_3d",.index=185,.modelIndex=285,.cardchunk=true,.texIndex=519,.glowIndex=518,.specIndex=1242},
    {.path="chunk_med2_4",.index=186,.modelIndex=287,.cardchunk=true,.texIndex=523,.glowIndex=522,.specIndex=1242},
    {.path="chunk_med2_5",.index=187,.modelIndex=288,.texIndex=527,.glowIndex=526,.specIndex=539,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,1.44f,0.0f},.colliderSize=(Vector3){2.56f,0.32f,2.56f},.colliderMeshIndex=U16_MAX},
    {.path="chunk_med2_6",.index=188,.modelIndex=289,.texIndex=528,.specIndex=1271},
    {.path="chunk_med2_7",.index=189,.modelIndex=290,.cardchunk=true,.texIndex=530,.glowIndex=529,.specIndex=1245},
    {.path="chunk_med2_8",.index=190,.modelIndex=291,.cardchunk=true,.texIndex=531,.specIndex=1242},
    {.path="chunk_med2_8_half_top",.index=191,.modelIndex=292,.texIndex=531,.specIndex=1242},
    {.path="chunk_med2_8_slice32RH",.index=192,.modelIndex=293,.texIndex=531,.specIndex=1242},
    {.path="chunk_med2_8_slice45",.index=193,.modelIndex=294,.texIndex=531,.specIndex=1242},
    {.path="chunk_med2_9",.index=194,.modelIndex=296,.cardchunk=true,.texIndex=535,.glowIndex=534,.specIndex=1242},
    {.path="chunk_med2_9d",.index=195,.modelIndex=295,.cardchunk=true,.texIndex=533,.glowIndex=532,.specIndex=1242},
    {.path="chunk_med3_1",.index=196,.modelIndex=297,.cardchunk=true,.texIndex=536,.specIndex=1236},
    {.path="chunk_rad1_1",.index=197,.modelIndex=501,.cardchunk=true,.texIndex=660,.glowIndex=659,.specIndex=1231},
    {.path="chunk_rad1_2",.index=198,.modelIndex=501,.cardchunk=true,.texIndex=662,.glowIndex=661,.specIndex=1231},
    {.path="chunk_reac1_1",.index=199,.modelIndex=502,.cardchunk=true,.texIndex=664,.specIndex=1243},
    {.path="chunk_reac1_1_slice45",.index=200,.modelIndex=339,.texIndex=664,.specIndex=1243},
    {.path="chunk_reac1_2",.index=201,.modelIndex=503,.cardchunk=true,.texIndex=665,.specIndex=1243},
    {.path="chunk_reac1_3",.index=202,.modelIndex=504,.cardchunk=true,.texIndex=666,.specIndex=1243},
    {.path="chunk_reac1_4",.index=203,.modelIndex=505,.cardchunk=true,.texIndex=668,.glowIndex=667,.specIndex=669},
    {.path="chunk_reac1_5",.index=204,.modelIndex=506,.cardchunk=true,.texIndex=671,.glowIndex=670,.specIndex=1239},
    {.path="chunk_reac1_6",.index=205,.modelIndex=507,.cardchunk=true,.texIndex=673,.glowIndex=672,.specIndex=1243},
    {.path="chunk_reac1_7",.index=206,.modelIndex=342,.cardchunk=true,.texIndex=676,.glowIndex=675,.specIndex=1243},
    {.path="chunk_reac1_8",.index=207,.modelIndex=508,.cardchunk=true,.texIndex=678,.glowIndex=678,.specIndex=1243},
    {.path="chunk_reac1_9",.index=208,.modelIndex=509,.cardchunk=true,.texIndex=680,.glowIndex=680,.specIndex=1243},
    {.path="chunk_reac2_1",.index=209,.modelIndex=512,.cardchunk=true,.texIndex=682,.specIndex=1235},
    {.path="chunk_reac2_1_slice45LH",.index=210,.modelIndex=514,.texIndex=682,.specIndex=1235},
    {.path="chunk_reac2_1_slice45LH_up",.index=211,.modelIndex=515,.texIndex=682,.specIndex=1235},
    {.path="chunk_reac2_1_slice45RH",.index=212,.modelIndex=516,.texIndex=682,.specIndex=1235},
    {.path="chunk_reac2_1_slice45RH_up",.index=213,.modelIndex=517,.texIndex=682,.specIndex=1235},
    {.path="chunk_reac2_1b",.index=214,.modelIndex=510,.cardchunk=true,.texIndex=681,.specIndex=1235},
    {.path="chunk_reac2_1bmirror",.index=215,.modelIndex=511,.cardchunk=true,.texIndex=681,.specIndex=1235},
    {.path="chunk_reac2_1mirror",.index=216,.modelIndex=513,.cardchunk=true,.texIndex=682,.specIndex=1235},
    {.path="chunk_reac2_2",.index=217,.modelIndex=518,.cardchunk=true,.texIndex=684,.glowIndex=683,.specIndex=1235},
    {.path="chunk_reac2_4",.index=218,.modelIndex=519,.cardchunk=true,.texIndex=685,.specIndex=1235},
    {.path="chunk_reac2_4_slice128lower",.index=219,.modelIndex=340,.texIndex=685,.specIndex=1235},
    {.path="chunk_reac2_5",.index=220,.modelIndex=520,.cardchunk=true,.texIndex=687,.glowIndex=686},
    {.path="chunk_reac2_6",.index=221,.modelIndex=521,.cardchunk=true,.texIndex=689,.glowIndex=688},
    {.path="chunk_reac2_7",.index=222,.modelIndex=522,.cardchunk=true,.texIndex=691,.glowIndex=690},
    {.path="chunk_reac2_8",.index=223,.modelIndex=523,.cardchunk=true,.texIndex=693,.glowIndex=692},
    {.path="chunk_reac2_9",.index=224,.modelIndex=524,.cardchunk=true,.texIndex=694},
    {.path="chunk_reac3_1",.index=225,.modelIndex=525,.cardchunk=true,.texIndex=696,.glowIndex=695},
    {.path="chunk_reac3_2",.index=226,.modelIndex=526,.cardchunk=true,.texIndex=697},
    {.path="chunk_reac3_3",.index=227,.modelIndex=527,.cardchunk=true,.texIndex=698},
    {.path="chunk_reac3_4",.index=228,.modelIndex=528,.cardchunk=true,.texIndex=699},
    {.path="chunk_reac3_5",.index=229,.modelIndex=529,.cardchunk=true,.texIndex=701,.glowIndex=700},
    {.path="chunk_reac3_6",.index=230,.modelIndex=530,.cardchunk=true,.texIndex=703,.glowIndex=702},
    {.path="chunk_reac3_7",.index=231,.modelIndex=531,.cardchunk=true,.texIndex=704,.specIndex=705},
    {.path="chunk_reac4_1",.index=232,.modelIndex=532,.cardchunk=true,.texIndex=707,.glowIndex=706},
    {.path="chunk_reac4_1_slice45lh",.index=233,.modelIndex=533,.texIndex=707},
    {.path="chunk_reac4_2",.index=234,.modelIndex=534,.cardchunk=true,.texIndex=709,.glowIndex=708},
    {.path="chunk_reac5_1",.index=235,.modelIndex=535,.cardchunk=true,.texIndex=711,.glowIndex=710},
    {.path="chunk_reac5_2",.index=236,.modelIndex=536,.cardchunk=true,.texIndex=713,.glowIndex=712},
    {.path="chunk_reac5_3",.index=237,.modelIndex=537,.cardchunk=true,.texIndex=715,.glowIndex=714},
    {.path="chunk_reac6_1",.index=238,.modelIndex=538,.cardchunk=true,.texIndex=716},
    {.path="chunk_reac6_2",.index=239,.cardchunk=true,.modelIndex=539,.texIndex=717},
    {.path="chunk_reac6_3",.index=240,.modelIndex=539,.cardchunk=true,.texIndex=719,.glowIndex=718},
    {.path="chunk_sci1_1",.index=241,.modelIndex=545,.cardchunk=true,.texIndex=722},
    {.path="chunk_sci1_1_slice45_toplh",.index=242,.modelIndex=542,.texIndex=722},
    {.path="chunk_sci1_1_slice45_toprh",.index=243,.modelIndex=543,.texIndex=722},
    {.path="chunk_sci1_1d",.index=244,.modelIndex=541,.cardchunk=true,.texIndex=721},
    {.path="chunk_sci1_2",.index=245,.modelIndex=545,.cardchunk=true,.texIndex=724},
    {.path="chunk_sci1_2_slice45lh",.index=246,.modelIndex=546,.cardchunk=true,.texIndex=724},
    {.path="chunk_sci1_2_slice45lh_up",.index=247,.modelIndex=547,.texIndex=724},
    {.path="chunk_sci1_2_slice45rh",.index=248,.modelIndex=548,.texIndex=724},
    {.path="chunk_sci1_2_slice45rh_up",.index=249,.modelIndex=549,.texIndex=724},
    {.path="chunk_sci1_2d",.index=250,.modelIndex=544,.cardchunk=true,.texIndex=723},
    {.path="chunk_sci1_3",.index=251,.modelIndex=550,.cardchunk=true,.texIndex=726,.glowIndex=725},
    {.path="chunk_sci1_4",.index=252,.cardchunk=true,.texIndex=727},
    {.path="chunk_sci1_5",.index=253,.modelIndex=551,.cardchunk=true,.texIndex=728},
    {.path="chunk_sci1_6",.index=254,.modelIndex=552,.cardchunk=true,.texIndex=729},
    {.path="chunk_sci1_6_slice45",.index=255,.modelIndex=553,.texIndex=729},
    {.path="chunk_sci1_7",.index=256,.modelIndex=555,.cardchunk=true,.texIndex=731},
    {.path="chunk_sci1_7d",.index=257,.modelIndex=554,.cardchunk=true,.texIndex=730},
    {.path="chunk_sci1_8",.index=258,.modelIndex=557,.cardchunk=true,.texIndex=734},
    {.path="chunk_sci1_8d",.index=259,.modelIndex=556,.cardchunk=true,.texIndex=733},
    {.path="chunk_sci1_9",.index=260,.modelIndex=559,.cardchunk=true,.texIndex=737},
    {.path="chunk_sci1_9d",.index=261,.modelIndex=558,.cardchunk=true,.texIndex=736,.glowIndex=735},
    {.path="chunk_sci2_1",.index=262,.modelIndex=561,.cardchunk=true,.texIndex=739},
    {.path="chunk_sci2_1_slice45lh",.index=263,.modelIndex=563,.cardchunk=true,.texIndex=739},
    {.path="chunk_sci2_1_slice45rh",.index=264,.modelIndex=562,.texIndex=739},
    {.path="chunk_sci2_1d",.index=265,.modelIndex=560,.cardchunk=true,.texIndex=738},
    {.path="chunk_sci2_2",.index=266,.modelIndex=565,.cardchunk=true,.texIndex=742,.glowIndex=741},
    {.path="chunk_sci2_2d",.index=267,.modelIndex=564,.cardchunk=true,.texIndex=740},
    {.path="chunk_sci2_3",.index=268,.modelIndex=566,.cardchunk=true,.texIndex=744,.glowIndex=743},
    {.path="chunk_sci2_4",.index=269,.modelIndex=567,.cardchunk=true,.texIndex=745},
    {.path="chunk_sci2_5",.index=270,.modelIndex=569,.cardchunk=true,.texIndex=747},
    {.path="chunk_sci2_5d",.index=271,.modelIndex=568,.cardchunk=true,.texIndex=746},
    {.path="chunk_sci3_1",.index=272,.modelIndex=571,.cardchunk=true,.texIndex=749},
    {.path="chunk_sci3_1d",.index=273,.modelIndex=570,.cardchunk=true,.texIndex=748},
    {.path="chunk_sci3_2",.index=274,.modelIndex=572,.cardchunk=true,.texIndex=750},
    {.path="chunk_sci3_3",.index=275,.modelIndex=573,.cardchunk=true,.texIndex=752,.glowIndex=751},
    {.path="chunk_sci3_4",.index=276,.modelIndex=574,.cardchunk=true,.texIndex=754},
    {.path="chunk_sci3_5",.index=277,.modelIndex=575,.cardchunk=true,.texIndex=756,.glowIndex=755},
    {.path="chunk_sci3_6",.index=278,.modelIndex=576,.cardchunk=true,.texIndex=758,.glowIndex=757},
    {.path="chunk_screen",.index=279,.modelIndex=5988,.cardchunk=true,.texIndex=881},
    {.path="chunk_sec1_1",.index=280,.modelIndex=178,.cardchunk=true,.texIndex=787,.specIndex=787},
    {.path="chunk_sec1_1b",.index=281,.modelIndex=178,.cardchunk=true,.texIndex=785,.specIndex=785},
    {.path="chunk_sec1_1c",.index=282,.modelIndex=577,.cardchunk=true,.texIndex=786,.specIndex=786},
    {.path="chunk_sec1_1c_slice45",.index=283,.modelIndex=580,.texIndex=786,.specIndex=786},
    {.path="chunk_sec1_1c_slice64highlh",.index=284,.modelIndex=581,.texIndex=786,.specIndex=786},
    {.path="chunk_sec1_1c_slice64highrh",.index=285,.modelIndex=582,.texIndex=786,.specIndex=786},
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 286 move along
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 287 don't think too much about it
    {.path="chunk_sec1_2",.index=288,.modelIndex=584,.cardchunk=true,.texIndex=789,.specIndex=1233},
    {.path="chunk_sec1_2b",.index=289,.modelIndex=583,.cardchunk=true,.texIndex=788,.specIndex=1233},
    {.path="chunk_sec1_3",.index=290,.modelIndex=585,.cardchunk=true,.texIndex=790,.specIndex=1233},
    {.path="chunk_sec1_3_slice45",.index=291,.modelIndex=586,.texIndex=790,.specIndex=1233},
    {.path="chunk_stor1_1",.index=292,.modelIndex=597,.cardchunk=true,.texIndex=824,.glowIndex=823},
    {.path="chunk_stor1_2",.index=293,.modelIndex=598,.cardchunk=true,.texIndex=825},
    {.path="chunk_stor1_3",.index=294,.modelIndex=598,.cardchunk=true,.texIndex=826},
    {.path="chunk_stor1_4",.index=295,.modelIndex=599,.cardchunk=true,.texIndex=827},
    {.path="chunk_stor1_5",.index=296,.modelIndex=600,.cardchunk=true,.texIndex=828},
    {.path="chunk_stor1_6",.index=297,.modelIndex=601,.cardchunk=true,.texIndex=829},
    {.path="chunk_stor1_6_slice128_up_lh",.index=298,.modelIndex=602,.texIndex=829},
    {.path="chunk_stor1_6_slice128_up_rh",.index=299,.modelIndex=603,.texIndex=829},
    {.path="chunk_stor1_6_slice192lh",.index=300,.modelIndex=604,.texIndex=829},
    {.path="chunk_stor1_6_slice192rh",.index=301,.modelIndex=605,.texIndex=829},
    {.path="chunk_stor1_7",.index=302,.modelIndex=606,.cardchunk=true,.texIndex=833,.specIndex=834,.normIndex=832},
    {.path="chunk_stor1_7_slice45",.index=303,.modelIndex=607,.texIndex=833,.specIndex=834,.normIndex=832},
    {.path="chunk_stor1_7d",.index=304,.modelIndex=620,.cardchunk=true,.texIndex=831,.glowIndex=830,.normIndex=832,.specIndex=834},
    {.path="chunk_teleporter",.index=305,.modelIndex=178,.cardchunk=true,.texIndex=1166},
    {.path="chunk_white",.index=306,.modelIndex=178,.cardchunk=true,.texIndex=881},
    {.path="item_paper_wad",.index=307,.modelIndex=487,.texIndex=1250,.mass=0.06f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_warecasing",.index=308,.modelIndex=637,.texIndex=1251,.mass=0.8f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_beaker",.index=309,.modelIndex=14,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=682,.texIndex=36,.specIndex=1242,.mass=0.28f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_beverage",.index=310,.modelIndex=18,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=683,.texIndex=37,.mass=0.12f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_skull",.index=311,.modelIndex=593,.mass=0.451f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_arm",.index=312,.modelIndex=7,.texIndex=28,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=678,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_audiolog",.index=313,.modelIndex=11,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=679,.texIndex=52,.glowIndex=80,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadefrag",.index=314,.modelIndex=182,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadeconc",.index=315,.modelIndex=165,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadeemp",.index=316,.modelIndex=168,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadeearth",.index=317,.modelIndex=181,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenademine",.index=318,.modelIndex=184,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadenitro",.index=319,.modelIndex=185,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadegas",.index=320,.modelIndex=183,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_patch_berserk",.index=321,.modelIndex=488,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=491,.texIndex=590,.mass=0.12f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_patch_detox",.index=322,.modelIndex=488,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=491,.texIndex=591,.mass=0.12f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_patch_genius",.index=323,.modelIndex=488,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=491,.texIndex=592,.mass=0.12f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_patch_medi",.index=324,.modelIndex=488,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=491,.texIndex=600,.mass=0.12f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_patch_reflex",.index=325,.modelIndex=488,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=491,.texIndex=641,.mass=0.12f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_patch_sight",.index=326,.modelIndex=488,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=491,.texIndex=646,.mass=0.12f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_patch_staminup",.index=327,.modelIndex=488,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=491,.texIndex=647,.mass=0.12f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_system",.index=328,.modelIndex=207,.texIndex=405,.glowIndex=404,.mass=0.17f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_navunit",.index=329,.modelIndex=204,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=696,.texIndex=1258,.glowIndex=1259,.mass=0.1f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_ereader",.index=330,.modelIndex=200,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=692,.mass=0.12f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_sensaround",.index=331,.modelIndex=205,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=697,.mass=0.12f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_targetid",.index=332,.modelIndex=208,.mass=0.08f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_shield",.index=333,.modelIndex=206,.mass=0.14f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_bio",.index=334,.modelIndex=197,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=689,.mass=0.1f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_lantern",.index=335,.modelIndex=203,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=695,.mass=0.11f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_envirosuit",.index=336,.modelIndex=199,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=691,.mass=0.451f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_booster",.index=337,.modelIndex=198,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=690,.mass=0.16f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_jumpjets",.index=338,.modelIndex=202,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=694,.mass=0.32f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_hw_infrared",.index=339,.modelIndex=201,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=693,.mass=0.1f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_fireextinguisher",.index=340,.modelIndex=144,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=684,.mass=1.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_admin",.index=341,.modelIndex=0,.texIndex=9,.glowIndex=82,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_workerhelmet",.index=342,.modelIndex=648,.mass=1.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_mk3",.index=343,.modelIndex=646,.mass=0.75f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_blaster",.index=344,.modelIndex=638,.mass=0.5f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_dartgun",.index=345,.modelIndex=640,.texIndex=876,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_flechette",.index=346,.modelIndex=642,.mass=0.4f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_ionrifle",.index=347,.modelIndex=643,.mass=0.8f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_rapier",.index=348,.modelIndex=653,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_pipe",.index=349,.modelIndex=649,.texIndex=887,.mass=0.85f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_magnum",.index=350,.modelIndex=644,.mass=0.6f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_magpulse",.index=351,.modelIndex=645,.mass=0.65f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_pistol",.index=352,.modelIndex=650,.texIndex=878,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_plasma",.index=353,.modelIndex=651,.mass=1.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_railgun",.index=354,.modelIndex=652,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_riotgun",.index=355,.modelIndex=654,.mass=0.55f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_skorpion",.index=356,.modelIndex=655,.mass=1.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_sparqbeam",.index=357,.modelIndex=656,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_stungun",.index=358,.modelIndex=657,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_battery",.index=359,.modelIndex=13,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=680,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_battery_icad",.index=360,.modelIndex=13,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=680,.mass=0.35f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_logic_probe",.index=361,.modelIndex=217,.texIndex=427,.mass=0.15f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_healthkit",.index=362,.modelIndex=196,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=688,.mass=0.25f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_plastique",.index=363,.modelIndex=492,.mass=1.4f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_chipset_interfacedemod",.index=364,.modelIndex=45,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.003744498f,0.0001704991f,0.03192701f},.colliderSize=(Vector3){0.459303f,0.3412231f,0.06385402f},.colliderMeshIndex=U16_MAX,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_flask",.index=365,.modelIndex=145,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=685,.texIndex=36,.specIndex=1242,.mass=0.22f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_chipset_bitflag",.index=366,.modelIndex=45,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.003744498f,0.0001704991f,0.03192701f},.colliderSize=(Vector3){0.459303f,0.3412231f,0.06385402f},.colliderMeshIndex=U16_MAX,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_rubber",.index=367,.modelIndex=8,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=676,.mass=0.25f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_isotopex22",.index=368,.modelIndex=209,.mass=1.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_testtube",.index=369,.modelIndex=622,.texIndex=36,.specIndex=1242,.mass=0.21f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadefrag_live",.index=370,.modelIndex=182,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_chipset_isolinear",.index=371,.modelIndex=46,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){-0.0009825006f,-0.0129465f,0.0148115f},.colliderSize=(Vector3){0.223635f,0.4175691f,0.02912301f},.colliderMeshIndex=U16_MAX,.mass=0.26f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadeconc_live",.index=372,.modelIndex=165,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_needle",.index=373,.modelIndex=4,.texIndex=15,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){-0.0004654949f,0.0004549972f,0.0244365f},.colliderSize=(Vector3){0.131339f,0.1442801f,0.04838703f},.colliderMeshIndex=U16_MAX,.mass=0.15f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_tranq",.index=374,.modelIndex=4,.texIndex=27,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){-0.0004654949f,0.0004549972f,0.0244365f},.colliderSize=(Vector3){0.131339f,0.1442801f,0.04838703f},.colliderMeshIndex=U16_MAX,.mass=0.15f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_standard",.index=375,.modelIndex=5,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0001984993f,0.0f,0.02172501f},.colliderSize=(Vector3){0.1209471f,0.2176701f,0.04345007f},.colliderMeshIndex=U16_MAX,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_teflon",.index=376,.modelIndex=5,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0001984993f,0.0f,0.02172501f},.colliderSize=(Vector3){0.1209471f,0.2176701f,0.04345007f},.colliderMeshIndex=U16_MAX,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_hollow",.index=377,.modelIndex=5,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0002185023f,0.0f,0.02122951f},.colliderSize=(Vector3){0.1423431f,0.2127061f,0.04245907f},.colliderMeshIndex=U16_MAX,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_slug",.index=378,.modelIndex=3,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0002185023f,0.0f,0.02122951f},.colliderSize=(Vector3){0.1423431f,0.2127061f,0.04245907f},.colliderMeshIndex=U16_MAX,.glowIndex=22,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_magnesium",.index=379,.modelIndex=3,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=673,.mass=0.35f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_penetrator",.index=380,.modelIndex=3,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=673,.mass=0.35f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_hornet",.index=381,.modelIndex=1,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=673,.mass=0.35f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_splinter",.index=382,.modelIndex=630,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=673,.mass=0.35f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_rail",.index=383,.modelIndex=6,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=675,.mass=0.4f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_slag",.index=384,.modelIndex=9,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=673,.mass=0.35f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_slaglarge",.index=385,.modelIndex=10,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=677,.mass=0.40f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_ammo_magcart",.index=386,.modelIndex=2,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=674,.mass=0.35f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadeemp_live",.index=387,.modelIndex=168,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_std",.index=388,.modelIndex=0,.texIndex=79,.glowIndex=867,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadeearth_live",.index=389,.modelIndex=181,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_group1",.index=390,.modelIndex=0,.texIndex=7,.glowIndex=159,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_science",.index=391,.modelIndex=0,.texIndex=2,.glowIndex=343,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_eng",.index=392,.modelIndex=0,.texIndex=3,.glowIndex=81,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_groupB",.index=393,.modelIndex=0,.texIndex=7,.glowIndex=159,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_security",.index=394,.modelIndex=0,.texIndex=10,.glowIndex=344,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_per5diego",.index=395,.modelIndex=0,.texIndex=8,.glowIndex=341,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_medi",.index=396,.modelIndex=0,.texIndex=1,.glowIndex=161,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_group3",.index=397,.modelIndex=0,.texIndex=7,.glowIndex=159,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_access_card_purple",.index=398,.modelIndex=0,.texIndex=5,.glowIndex=342,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_head_male",.index=399,.modelIndex=194,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=687,.mass=1.29f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_head_female",.index=400,.modelIndex=193,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=686,.mass=1.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_severedhead",.index=401,.modelIndex=590,.mass=1.28f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenademine_live",.index=402,.modelIndex=184,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadenitro_live",.index=403,.modelIndex=185,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="weapon_grenadegas_live",.index=404,.modelIndex=183,.mass=1.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="line_sparqbeam",.index=405,.modelIndex=656},
    {.path="line_blaster",.index=406,.modelIndex=638},
    {.path="line_ion",.index=407,.modelIndex=643},
    {.path="line_hopperbeam",.index=408,.modelIndex=MODEL_IDX_MAX},
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 409 wut I decided I didn't need these ok
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 410
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 411
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 412
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 413
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 414
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 415
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 416
    {.path="item_access_card_perdarcy",.index=417,.modelIndex=0,.texIndex=8,.glowIndex=341,.collider=COLLIDER_TYPE_CONVEXMESH,.colliderMeshIndex=672,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 418
    {.path="npc_autobomb",.index=419,.modelIndex=299,.texIndex=542,.glowIndex=541,.mass=1.0f,.linearDrag=0.0f,.angularDrag=1.0f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cyborg_assassin",.index=420,.modelIndex=306,.numclips=8,.animationNum=24,.texIndex=545,.glowIndex=544,.mass=1.5f,.linearDrag=0.0f,.angularDrag=1.5f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_avian_mutant",.index=421,.modelIndex=328,.numclips=5,.animationNum=35,.texIndex=568,.mass=2.0f,.linearDrag=0.0f,.angularDrag=1.0f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_exec_bot",.index=422,.modelIndex=316,.numclips=5,.animationNum=29,.texIndex=555,.mass=2.2f,.linearDrag=0.0f,.angularDrag=1.5f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cyborg_drone",.index=423,.modelIndex=312,.numclips=7,.animationNum=3,.texIndex=547,.mass=1.5f,.linearDrag=0.0f,.angularDrag=2.0f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cortex_reaver",.index=424,.modelIndex=300,.numclips=6,.animationNum=23,.texIndex=543,.mass=5.0f,.linearDrag=0.0f,.angularDrag=3.0f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cyborg_warrior",.index=425,.modelIndex=315,.numclips=7,.animationNum=28,.texIndex=554,.mass=1.5f,.linearDrag=0.0f,.angularDrag=2.0f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cyborg_enforcer",.index=426,.modelIndex=314,.numclips=8,.animationNum=27,.texIndex=550,.mass=1.5f,.linearDrag=0.0f,.angularDrag=2.2f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cyborg_elite",.index=427,.modelIndex=313,.numclips=10,.animationNum=26,.texIndex=548,.mass=3.5f,.linearDrag=0.0f,.angularDrag=2.2f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cyborg_diego",.index=428,.modelIndex=309,.numclips=6,.animationNum=25,.texIndex=546,.mass=2.0f,.linearDrag=0.0f,.angularDrag=2.2f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_sec1_bot",.index=429,.modelIndex=333,.numclips=2,.animationNum=38,.texIndex=573,.mass=1.5f,.linearDrag=0.0f,.angularDrag=0.8f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},    
    {.path="npc_sec2_bot",.index=430,.modelIndex=335,.numclips=6,.animationNum=39,.texIndex=574,.mass=4.51f,.linearDrag=0.0f,.angularDrag=2.2f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_maint_bot",.index=431,.modelIndex=325,.numclips=4,.animationNum=34,.texIndex=567,.mass=1.5f,.linearDrag=0.0f,.angularDrag=1.5f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_mutant_cyborg",.index=432,.modelIndex=329,.numclips=7,.animationNum=51,.texIndex=569,.mass=3.0f,.linearDrag=0.0f,.angularDrag=2.2f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_hopper",.index=433,.modelIndex=322,.numclips=8,.animationNum=32,.texIndex=562,.mass=1.0f,.linearDrag=0.0f,.angularDrag=1000.451f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.005f,.staticFriction=0.1f,.bounciness=0.1f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="npc_humanoid_mutant",.index=434,.modelIndex=323,.numclips=6,.animationNum=2,.texIndex=563,.mass=1.4f,.linearDrag=0.0f,.angularDrag=2.0f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_invisomut",.index=435,.modelIndex=324,.numclips=5,.animationNum=33,.texIndex=565,.mass=1.3f,.linearDrag=0.0f,.angularDrag=0.8f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_virus_mutant",.index=436,.modelIndex=330,.numclips=6,.animationNum=41,.texIndex=576,.mass=1.4f,.linearDrag=0.0f,.angularDrag=2.0f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_servbot",.index=437,.modelIndex=337,.numclips=5,.animationNum=40,.texIndex=575,.mass=2.5f,.linearDrag=0.0f,.angularDrag=1.0f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_flier_bot",.index=438,.modelIndex=318,.numclips=5,.animationNum=30,.texIndex=558,.mass=1.75f,.linearDrag=0.0f,.angularDrag=0.8f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_zerog_mutant",.index=439,.modelIndex=395,.numclips=3,.animationNum=42,.texIndex=1170,.mass=1.3f,.linearDrag=0.0f,.angularDrag=1.0f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_gorilla_tiger_mutant",.index=440,.modelIndex=320,.numclips=7,.animationNum=31,.texIndex=560,.mass=2.0f,.linearDrag=0.0f,.angularDrag=2.2f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_repairbot",.index=441,.modelIndex=331,.numclips=4,.animationNum=37,.texIndex=572,.mass=1.5f,.linearDrag=0.0f,.angularDrag=2.0f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_plant_mutant",.index=442,.modelIndex=330,.numclips=6,.animationNum=36,.texIndex=570,.mass=0.8f,.linearDrag=0.0f,.angularDrag=1.5f,.gravity=1.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cyberdog",.index=443,.modelIndex=302,.mass=1.5f,.linearDrag=0.0f,.angularDrag=3.0f,.gravity=0.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cyberguard",.index=444,.modelIndex=303,.mass=2.0f,.linearDrag=0.0f,.angularDrag=3.0f,.gravity=0.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cyberram",.index=445,.modelIndex=304,.mass=2.0f,.linearDrag=0.0f,.angularDrag=3.0f,.gravity=0.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cyber_reaver",.index=446,.modelIndex=305,.mass=2.2f,.linearDrag=0.0f,.angularDrag=3.0f,.gravity=0.0f,.kinematic=true,.dynamicFriction=0.15f,.staticFriction=1.0f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="npc_cybershodan",.index=447,.modelIndex=MODEL_IDX_MAX,.mass=4.51f,.linearDrag=0.0f,.angularDrag=3.0f,.gravity=0.0f,.kinematic=true,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="item_cyber_data",.index=448,.modelIndex=65,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.5f,1.5f,1.5f}},
    {.path="item_cyber_decoy",.index=449,.modelIndex=72,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.5f,1.5f,1.5f}},
    {.path="item_cyber_drill",.index=450,.modelIndex=68,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.5f,1.5f,1.5f}},
    {.path="item_cyber_game",.index=451,.modelIndex=65,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.5f,1.5f,1.5f}},
    {.path="item_cyber_integrity",.index=452,.modelIndex=69,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.5f,1.5f,1.5f}},
    {.path="item_cyber_keycard",.index=453,.modelIndex=70,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.5f,1.5f,1.5f}},
    {.path="item_cyber_pulser",.index=454,.modelIndex=65,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.5f,1.5f,1.5f}},
    {.path="item_cyber_recall",.index=455,.modelIndex=65,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.5f,1.5f,1.5f}},
    {.path="item_cyber_shield",.index=456,.modelIndex=65,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.5f,1.5f,1.5f}},
    {.path="item_cyber_turbo",.index=457,.modelIndex=65,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.5f,1.5f,1.5f}},
    {.path="prop_phys_barrel_chemical",.index=458,.modelIndex=12,.texIndex=30,.mass=1.5f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="prop_phys_barrel_radiation",.index=459,.modelIndex=12,.texIndex=31,.mass=1.5f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="prop_phys_barrel_toxic",.index=460,.modelIndex=12,.texIndex=33,.mass=1.5f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="prop_phys_cart",.index=461,.modelIndex=40,.texIndex=416,.mass=2.5f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="prop_phys_pot",.index=462,.modelIndex=494,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="prop_phys_toolcart",.index=463,.modelIndex=624,.texIndex=865,.normIndex=864,.specIndex=866,.mass=20.0f,.linearDrag=1.0f,.angularDrag=0.2f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="se_briefcase",.index=464,.modelIndex=34,.texIndex=66,.glowIndex=65},
    {.path="se_corpse_blueshirt",.index=465,.texIndex=126,.specIndex=127,.modelIndex=51},
    {.path="se_corpse_brownshirt",.index=466,.texIndex=128,.specIndex=129,.modelIndex=52},
    {.path="se_corpse_eaten",.index=467,.texIndex=130,.specIndex=131,.modelIndex=53},
    {.path="se_corpse_labcoat",.index=468,.texIndex=132,.specIndex=133,.modelIndex=55},
    {.path="se_corpse_security",.index=469,.texIndex=136,.specIndex=137,.modelIndex=56},
    {.path="se_corpse_tan",.index=470,.texIndex=138,.modelIndex=57},
    {.path="se_corpse_torso",.index=471,.texIndex=126,.specIndex=127,.modelIndex=58},
    {.path="se_crate1",.index=472,.texIndex=145,.modelIndex=60,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,0.0f,0.3420931f},.colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f},.colliderMeshIndex=U16_MAX,.mass=0.75f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="se_crate2",.index=473,.texIndex=143,.modelIndex=60,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,0.0f,0.3420931f},.colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f},.colliderMeshIndex=U16_MAX,.mass=0.75f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="se_crate3",.index=474,.texIndex=144,.modelIndex=60,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,0.0f,0.3420931f},.colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f},.colliderMeshIndex=U16_MAX,.mass=0.75f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="se_crate4",.index=475,.texIndex=146,.modelIndex=60,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,0.0f,0.3420931f},.colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f},.colliderMeshIndex=U16_MAX,.mass=2.25f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="se_crate5",.index=476,.modelIndex=60,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,0.0f,0.3420931f},.colliderSize=(Vector3){0.684186f,0.6841861f,0.6841861f},.colliderMeshIndex=U16_MAX,.mass=2.25f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=1.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="sec_camera",.index=477,.modelIndex=589,.texIndex=73,.glowIndex=72},
    {.path="sec_cpunode",.index=478,.modelIndex=587,.texIndex=242,.glowIndex=248},
    {.path="sec_cpunode_small",.index=479,.modelIndex=588,.texIndex=107},
    {.path="weapon_cyber_mine",.index=480,.modelIndex=71,.texIndex=1224},
    {.path="proj_enemshot2",.index=481,.modelIndex=MODEL_IDX_MAX,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_magpulse_shot",.index=482,.modelIndex=MODEL_IDX_MAX,.texIndex=807,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_stungun_shot",.index=483,.modelIndex=MODEL_IDX_MAX,.texIndex=835,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_rail_shot",.index=484,.modelIndex=652,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_plasmarifle_shot",.index=485,.modelIndex=651,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.1f,.staticFriction=0.2f,.bounciness=0.9f,.frictionCombine=PHYS_COMBINE_MUL,.bounceCombine=PHYS_COMBINE_MAX},
    {.path="proj_enemshot6",.index=486,.modelIndex=MODEL_IDX_MAX,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_enemshot5",.index=487,.modelIndex=MODEL_IDX_MAX,.mass=0.2f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_enemshot4",.index=488,.modelIndex=MODEL_IDX_MAX,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_throwingstar",.index=489,.modelIndex=307,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_magpulsenpc_shot",.index=490,.modelIndex=645,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_railnpc_shot",.index=491,.modelIndex=MODEL_IDX_MAX,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_cyberplayer_shot",.index=492,.modelIndex=72,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false},
    {.path="proj_cyberdog_shot",.index=493,.modelIndex=63,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_cyberreaver_shot",.index=494,.modelIndex=64,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="proj_cyberice_shot",.index=495,.modelIndex=68,.mass=0.3f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=false,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="doorA",.index=496,.modelIndex=719,.texIndex=185,.numclips=4,.animationNum=1},
    {.path="doorB",.index=497,.modelIndex=0,.texIndex=189,.glowIndex=188,.numclips=4,.animationNum=0},
    {.path="doorC",.index=498,.modelIndex=0,.texIndex=184,.numclips=4,.animationNum=5},
    {.path="doorD",.index=499,.modelIndex=0,.numclips=4,.animationNum=4,.texIndex=196,.glowIndex=197},
    {.path="doorE",.index=500,.modelIndex=0,.numclips=4,.animationNum=9,.texIndex=208,.glowIndex=207},
    {.path="doorF",.index=501,.modelIndex=0,.numclips=4,.animationNum=10,.texIndex=187},
    {.path="doorG",.index=502,.modelIndex=0,.numclips=4,.animationNum=11,.texIndex=193,.glowIndex=194},
    {.path="doorH",.index=503,.modelIndex=0,.numclips=4,.animationNum=12,.texIndex=190},
    {.path="doorI",.index=504,.modelIndex=0,.numclips=4,.animationNum=13,.texIndex=200,.glowIndex=199},
    {.path="doorJ",.index=505,.modelIndex=0,.numclips=4,.animationNum=6,.texIndex=215},
    {.path="doorK",.index=506,.modelIndex=0,.numclips=4,.animationNum=7,.texIndex=214},
    {.path="doorL",.index=507,.modelIndex=0,.numclips=4,.animationNum=8,.texIndex=191},
    {.path="door_elevator1",.index=508,.modelIndex=0,.numclips=4,.animationNum=14,.texIndex=202},
    {.path="door_elevator2",.index=509,.modelIndex=0,.numclips=4,.animationNum=15,.texIndex=203},
    {.path="door_elevator3",.index=510,.modelIndex=0,.numclips=4,.animationNum=16,.texIndex=206,.glowIndex=205},
    {.path="door_elevator4",.index=511,.modelIndex=0,.numclips=4,.animationNum=17,.texIndex=203},
    {.path="door_secret1",.index=512,.modelIndex=0,.numclips=4,.animationNum=19,.texIndex=210},
    {.path="door_secret2",.index=513,.modelIndex=0,.numclips=4,.animationNum=18,.texIndex=209},
    {.path="door_secret3",.index=514,.modelIndex=94,.numclips=4,.animationNum=20,.texIndex=211},
    {.path="func_forcebridge",.index=515,.modelIndex=78,.texIndex=38},
    {.path="prop_lift2",.index=516,.modelIndex=215,.texIndex=155,.glowIndex=154,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){1.0f,1.0f,1.0f},.colliderMeshIndex=U16_MAX},
    {.path="func_wall",.index=517,.modelIndex=MODEL_IDX_MAX,.mass=10.0f,.linearDrag=0.0f,.angularDrag=0.05f,.gravity=0.0f,.kinematic=true,.dynamicFriction=0.6f,.staticFriction=0.6f,.bounciness=0.0f,.frictionCombine=PHYS_COMBINE_AVG,.bounceCombine=PHYS_COMBINE_AVG},
    {.path="BulletHoleLarge",.index=518,.modelIndex=MODEL_IDX_MAX},
    {.path="BulletHoleScorchLarge",.index=519,.modelIndex=MODEL_IDX_MAX},
    {.path="BulletHoleScorchSmall",.index=520,.modelIndex=MODEL_IDX_MAX},
    {.path="BulletHoleSmall",.index=521,.modelIndex=MODEL_IDX_MAX},
    {.path="BulletHoleTiny",.index=522,.modelIndex=MODEL_IDX_MAX},
    {.path="BulletHoleTinySpread",.index=523,.modelIndex=MODEL_IDX_MAX},
    {.path="func_door_cyber",.index=524,.modelIndex=178,.texIndex=1224,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,1.31f,0.0f},.colliderSize=(Vector3){2.56f,0.06f,2.56f},.colliderMeshIndex=U16_MAX},
    {.path="prop_console01",.index=525,.texIndex=100,.modelIndex=49},
    {.path="prop_console02",.index=526,.texIndex=100,.modelIndex=50},
    {.path="prop_grate1_1",.index=527,.modelIndex=186,.texIndex=359},
    {.path="prop_grate1_2",.index=528,.modelIndex=187,.texIndex=360},
    {.path="prop_grate1_3",.index=529,.modelIndex=188,.texIndex=361},
    {.path="se_cabinet",.index=530,.modelIndex=39,.texIndex=70},
    {.path="se_thermos",.index=531,.texIndex=863,.modelIndex=623},
    {.path="prop_beaker_holder",.index=532,.modelIndex=15,.texIndex=36},
    {.path="prop_bed",.index=533,.modelIndex=16,.texIndex=246},
    {.path="prop_bed_hospital",.index=534,.modelIndex=608,.texIndex=759},
    {.path="prop_bed_neurosurgery",.index=535,.texIndex=18,.normIndex=29,.specIndex=1238,.modelIndex=17},
    {.path="prop_bonepile1",.index=536,.modelIndex=19,.texIndex=815},
    {.path="prop_bridgewall1",.index=537,.modelIndex=33},
    {.path="prop_broken_clock",.index=538,.modelIndex=38,.texIndex=1117,.altTexIndex=1118,.glowIndex=1115,.altGlowIndex=1116},
    {.path="prop_brokengun",.index=539,.modelIndex=639,.texIndex=878},
    {.path="prop_chair01",.index=540,.modelIndex=41,.texIndex=195},
    {.path="prop_chair02",.index=541,.modelIndex=42,.texIndex=195},
    {.path="prop_chair03",.index=542,.modelIndex=43,.texIndex=195},
    {.path="prop_chair04",.index=543,.modelIndex=41,.texIndex=195},
    {.path="prop_chair05",.index=544,.modelIndex=42,.texIndex=195},
    {.path="prop_chandelier",.index=545,.modelIndex=496,.texIndex=644},
    {.path="prop_charge_station",.index=546,.modelIndex=44,.texIndex=77,.glowIndex=76},
    {.path="prop_clothes",.index=547,.modelIndex=47,.texIndex=97},
    {.path="prop_computer",.index=548,.modelIndex=48},
    {.path="prop_couch",.index=549,.modelIndex=59},
    {.path="prop_couch2",.index=550,.modelIndex=59},
    {.path="prop_cpuscreen",.index=551,.modelIndex=178,.texIndex=768},
    {.path="prop_cyber_datafrag",.index=552,.modelIndex=78},
    {.path="prop_cyber_decoy",.index=553,.modelIndex=78},
    {.path="prop_cyber_exit",.index=554,.modelIndex=78},
    {.path="prop_cyber_switch",.index=555,.modelIndex=0},
    {.path="prop_cyberport",.index=556,.modelIndex=62,.texIndex=117,.glowIndex=116},
    {.path="prop_desk01",.index=557,.modelIndex=74,.texIndex=125},
    {.path="prop_desk02",.index=558,.modelIndex=75,.texIndex=124},
    {.path="prop_dexmissile",.index=559,.modelIndex=76,.texIndex=164,.glowIndex=162},
    {.path="prop_foliage_fernpoison",.index=560,.modelIndex=160,.texIndex=331},
    {.path="prop_foliage_bush",.index=561,.modelIndex=495,.texIndex=643,.glowIndex=642},
    {.path="prop_foliage_fern",.index=562,.modelIndex=160,.texIndex=333,.glowIndex=330},
    {.path="prop_foliage_fernblueflower",.index=563,.modelIndex=159,.texIndex=333,.glowIndex=330},
    {.path="prop_foliage_pinetreem",.index=564,.modelIndex=489,.texIndex=594},
    {.path="prop_foliage_poisonbush1",.index=565,.modelIndex=493,.texIndex=638},
    {.path="prop_gear_large",.index=566,.modelIndex=166,.texIndex=335},
    {.path="prop_gear_small",.index=567,.modelIndex=167,.texIndex=336},
    {.path="prop_grass1",.index=568,.modelIndex=MODEL_IDX_MAX,.texIndex=329},
    {.path="prop_grass2",.index=569,.modelIndex=MODEL_IDX_MAX,.texIndex=329},
    {.path="prop_grass3",.index=570,.modelIndex=MODEL_IDX_MAX,.texIndex=329},
    {.path="prop_grass4",.index=571,.modelIndex=MODEL_IDX_MAX,.texIndex=329},
    {.path="prop_grass5",.index=572,.modelIndex=MODEL_IDX_MAX,.texIndex=329},
    {.path="prop_grate4",.index=573,.modelIndex=161,.texIndex=329},
    {.path="prop_healingbed",.index=574,.modelIndex=195,.texIndex=1139},
    {.path="prop_lamp",.index=575,.modelIndex=212,.texIndex=423},
    {.path="prop_light_emergsignal",.index=576,.modelIndex=216,.texIndex=426,.altTexIndex=424,.glowIndex=0,.altGlowIndex=424},
    {.path="prop_microscope",.index=577,.modelIndex=298,.texIndex=645,.specIndex=1241},
    {.path="prop_pipe",.index=578,.modelIndex=490,.texIndex=595}, // TODO, support texture index 596 somehow from level override
    {.path="prop_puddle",.index=579,.modelIndex=157,.texIndex=648},
    {.path="prop_puddle_grease",.index=580,.modelIndex=157,.texIndex=650},
    {.path="prop_puddle_oil",.index=581,.modelIndex=157,.texIndex=652},
    {.path="prop_shelves",.index=582,.modelIndex=591,.texIndex=94},
    {.path="prop_skeleton",.index=583,.modelIndex=592,.texIndex=815},
    {.path="prop_sleeping_cables",.index=584,.modelIndex=595,.texIndex=71},
    {.path="prop_sparkingwire",.index=585,.modelIndex=0,.numclips=1,.animationNum=46,.texIndex=71},
    {.path="prop_table",.index=586,.modelIndex=619,.texIndex=92},
    {.path="prop_tv_on_a_post",.index=587,.modelIndex=625,.texIndex=1228},
    {.path="prop_vendingmachines1",.index=588,.modelIndex=627,.texIndex=870}, // TODO: Get child vending1_2.obj and vending1_3.obj
    {.path="prop_vendingmachines2",.index=589,.modelIndex=614,.texIndex=871}, // TODO: Get child vending2_2.obj
    {.path="prop_weapon_rack",.index=590,.modelIndex=641,.texIndex=113},
    {.path="prop_xray",.index=591,.modelIndex=660,.texIndex=153},
    {.path="text_decal",.index=592,.modelIndex=77},
    {.path="text_decalStopDSS1",.index=593,.modelIndex=77},
    {.path="trigger_counter",.index=594,.modelIndex=MODEL_IDX_MAX},
    {.path="trigger_cyberpush",.index=595,.modelIndex=MODEL_IDX_MAX},
    {.path="trigger_gravitylift",.index=596,.modelIndex=MODEL_IDX_MAX},
    {.path="trigger_ladder",.index=597,.modelIndex=MODEL_IDX_MAX},
    {.path="trigger_multiple",.index=598,.modelIndex=MODEL_IDX_MAX},
    {.path="trigger_music",.index=599,.modelIndex=MODEL_IDX_MAX},
    {.path="trigger_once",.index=600,.modelIndex=MODEL_IDX_MAX},
    {.path="trigger_radiation",.index=601,.modelIndex=MODEL_IDX_MAX},
    {.path="us_isotopepanel",.index=602,.modelIndex=0,.texIndex=616,.numclips=5,.animationNum=44},
    {.path="us_paperlog",.index=603,.modelIndex=486,.texIndex=580},
    {.path="us_puz_elevatorkeypad",.index=604,.modelIndex=615,.texIndex=247},
    {.path="us_puz_elevatorkeypad2",.index=605,.modelIndex=618,.texIndex=250},
    {.path="us_puz_elevatorkeypad3",.index=606,.modelIndex=615,.texIndex=247},
    {.path="us_puz_elevatorkeypad4",.index=607,.modelIndex=210,.texIndex=249},
    {.path="us_puz_keypad",.index=608,.modelIndex=211,.texIndex=414},
    {.path="us_puz_panel_blue_grid",.index=609,.modelIndex=0,.texIndex=604,.numclips=3,.animationNum=43},
    {.path="us_puz_panel_brown_grid",.index=610,.modelIndex=0,.texIndex=604,.numclips=3,.animationNum=43},
    {.path="us_puz_panel_gray_grid",.index=611,.modelIndex=0,.texIndex=634,.numclips=3,.animationNum=43},
    {.path="us_puz_panel_red_grid",.index=612,.modelIndex=0,.texIndex=625,.numclips=3,.animationNum=43},
    {.path="us_puz_panel_teal_grid",.index=613,.modelIndex=0,.texIndex=601,.numclips=3,.animationNum=43},
    {.path="us_relaypanel",.index=614,.modelIndex=0,.texIndex=617,.numclips=4,.animationNum=45},
    {.path="us_retinalscanner",.index=615,.modelIndex=79,.texIndex=46},
    {.path="prop_vending1_1",.index=616,.modelIndex=627,.texIndex=870},
    {.path="prop_vending1_2",.index=617,.modelIndex=628,.texIndex=870},
    {.path="prop_vending1_3",.index=618,.modelIndex=629,.texIndex=870},
    {.path="prop_vending2_1",.index=619,.modelIndex=614,.texIndex=871},
    {.path="prop_vending2_2",.index=620,.modelIndex=621,.texIndex=871},
    {.path="ambient_airhiss",.index=621,.volume=0.05f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_clicker",.index=622,.volume=0.20f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_compressor",.index=623,.volume=0.4f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_dishwasher",.index=624,.volume=0.2f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_drip_amb",.index=625,.volume=0.5f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_fan",.index=626,.volume=0.3f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_generator_gas",.index=627,.volume=0.3f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_gurgle",.index=628,.volume=0.3f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_icemaker",.index=629,.volume=0.6f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_intake",.index=630,.volume=0.2f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_lathe",.index=631,.volume=0.4f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_lev3loop1",.index=632,.volume=0.1f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_lev3loop2",.index=633,.volume=0.1f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_lev3loop3",.index=634,.volume=0.1f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_lev3loop4",.index=635,.volume=0.1f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_liquid_bubble",.index=636,.volume=1.0f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_liquid_lava2",.index=637,.volume=0.4f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_looping",.index=638,.volume=0.4f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_machgear_loop",.index=639,.volume=0.4f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_machine_ambience",.index=640,.volume=0.8f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_machine_go",.index=641,.volume=0.6f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_machine_humamb7",.index=642,.volume=1.0f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_machine_humlonoise",.index=643,.volume=0.4f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_machine_loop1",.index=644,.volume=0.4f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_machine_loop2",.index=645,.volume=0.4f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_machinea1",.index=646,.volume=0.4f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_machinevat_loop",.index=647,.volume=0.8f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_mist",.index=648,.volume=0.02f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_pipewater_loop",.index=649,.volume=0.65f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_powerloom",.index=650,.volume=0.3f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_pump",.index=651,.volume=0.2f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_pump2",.index=652,.volume=0.05f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_rain",.index=653,.volume=0.55f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_steam_loop",.index=654,.volume=0.1f,.modelIndex=MODEL_IDX_MAX},
    {.path="ambient_washing_machine",.index=655,.volume=0.5f,.modelIndex=MODEL_IDX_MAX},
    {.path="decal_blood_die",.index=656,.modelIndex=77,.texIndex=237,.shadows=false},
    {.path="decal_blood_resist",.index=657,.modelIndex=77,.texIndex=240,.shadows=false},
    {.path="decal_blood_stayaway",.index=658,.modelIndex=77,.texIndex=235,.shadows=false},
    {.path="decal_blood_words2",.index=659,.modelIndex=77,.texIndex=236,.shadows=false},
    {.path="decal_bloodfonta",.index=660,.modelIndex=178,.texIndex=118,.shadows=false},
    {.path="decal_bloodfonte",.index=661,.modelIndex=178,.texIndex=121,.shadows=false},
    {.path="decal_bloodfontg",.index=662,.modelIndex=178,.texIndex=122,.shadows=false},
    {.path="decal_bloodfonth",.index=663,.modelIndex=178,.texIndex=89,.shadows=false},
    {.path="decal_bloodfontr",.index=664,.modelIndex=178,.texIndex=139,.shadows=false},
    {.path="decal_bloodfonty",.index=665,.modelIndex=178,.texIndex=140,.shadows=false},
    {.path="decal_bloodsplat2",.index=666,.modelIndex=157,.texIndex=130,.shadows=false},
    {.path="decal_logo_antenna",.index=667,.modelIndex=77,.texIndex=182,.shadows=false},
    {.path="decal_logo_armory",.index=668,.modelIndex=77,.texIndex=178,.shadows=false},
    {.path="decal_logo_biohazard",.index=669,.modelIndex=77,.texIndex=180,.shadows=false},
    {.path="decal_logo_bridge",.index=670,.modelIndex=77,.texIndex=181,.shadows=false},
    {.path="decal_logo_cyborg",.index=671,.modelIndex=77,.texIndex=176,.shadows=false},
    {.path="decal_logo_gears",.index=672,.modelIndex=77,.texIndex=174,.shadows=false},
    {.path="decal_logo_medical",.index=673,.modelIndex=77,.texIndex=165,.shadows=false},
    {.path="decal_logo_radhazard",.index=674,.modelIndex=77,.texIndex=177,.shadows=false},
    {.path="decal_logo_research",.index=675,.modelIndex=77,.texIndex=175,.shadows=false},
    {.path="decal_logo_security",.index=676,.modelIndex=77,.texIndex=167,.shadows=false},
    {.path="decal_painting1",.index=677,.modelIndex=77,.texIndex=218,.glowIndex=216,.normIndex=217,.shadows=false},
    {.path="decal_painting2",.index=678,.modelIndex=77,.texIndex=220,.glowIndex=219,.shadows=false},
    {.path="decal_painting3",.index=679,.modelIndex=77,.texIndex=222,.glowIndex=221,.shadows=false},
    {.path="decal_posterbetterfuture",.index=680,.modelIndex=77,.texIndex=226,.normIndex=225,.shadows=false},
    {.path="decal_postergenetics",.index=681,.modelIndex=77,.texIndex=224,.normIndex=223,.shadows=false},
    {.path="decal_scorch1",.index=682,.modelIndex=77,.texIndex=227,.shadows=false},
    {.path="decal_scorch2",.index=683,.modelIndex=77,.texIndex=228,.shadows=false},
    {.path="decal_scorch3",.index=684,.modelIndex=77,.texIndex=229,.shadows=false},
    {.path="decal_scorch4",.index=685,.modelIndex=77,.texIndex=230,.shadows=false},
    {.path="decal_scorchtiny",.index=686,.modelIndex=77,.texIndex=232,.shadows=false},
    {.path="decal_blood_splat",.index=687,.modelIndex=77,.texIndex=234,.shadows=false},
    {.path="func_switch1",.index=688,.modelIndex=609,.texIndex=837,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){0.32f,0.04f,0.32f},.colliderMeshIndex=U16_MAX},
    {.path="func_switch2",.index=689,.modelIndex=610,.texIndex=839,.mainSwitchMaterial=839,.altTexIndex=841,.glowIndex=0,.altGlowIndex=840,.changeTexOnActive=true,.blinkTexOnActive=true,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){-0.6088825f,0.0f,0.0001220703f},.colliderSize=(Vector3){1.190795f,16.0f,16.0f},.colliderMeshIndex=U16_MAX},
    {.path="func_switch3",.index=690,.modelIndex=611,.texIndex=842,.altTexIndex=844,.glowIndex=0,.altGlowIndex=843,.changeTexOnActive=true,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){-0.571252f,0.001326527f,-0.001424824f},.colliderSize=(Vector3){0.5f,8.0f,8.0f},.colliderMeshIndex=U16_MAX},
    {.path="func_switch4",.index=691,.modelIndex=612,.texIndex=846,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.06f,0.0f,0.0f},.colliderSize=(Vector3){0.2f,0.64f,0.64f},.colliderMeshIndex=U16_MAX},
    {.path="func_switch5",.index=692,.modelIndex=614,.texIndex=848,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){0.64f,0.64f,0.08f},.colliderMeshIndex=U16_MAX},
    {.path="func_switch5broken",.index=693,.modelIndex=613,.texIndex=847,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){0.64f,0.64f,0.08f},.colliderMeshIndex=U16_MAX},
    {.path="func_switch7",.index=694,.modelIndex=612,.texIndex=854,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){1.523325f,0.0f,0.0f},.colliderSize=(Vector3){5.020065f,16.0f,16.0f},.colliderMeshIndex=U16_MAX},
    {.path="func_switch8",.index=695,.modelIndex=616,.texIndex=856,.altTexIndex=858,.glowIndex=855,.altGlowIndex=857,.changeTexOnActive=true,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){-1.0f,0.0f,0.0001220703f},.colliderSize=(Vector3){2.0f,16.0f,16.0f},.colliderMeshIndex=U16_MAX},
    {.path="func_switchbroken1",.index=696,.modelIndex=617,.texIndex=618},
    {.path="clip_npc",.index=697,.modelIndex=MODEL_IDX_MAX,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){1.005016f,0.0f,0.0f},.colliderSize=(Vector3){2.010033f,16.0f,16.0f},.colliderMeshIndex=U16_MAX},
    {.path="clip_objects",.index=698,.modelIndex=MODEL_IDX_MAX,.collider=COLLIDER_TYPE_BOX,.colliderCenter=(Vector3){0.0f,0.0f,0.0f},.colliderSize=(Vector3){2.56f,2.56f,2.56f},.colliderMeshIndex=U16_MAX},
    {.path="logic_relay",.index=699,.modelIndex=MODEL_IDX_MAX},
    {.path="logic_branch",.index=700,.modelIndex=MODEL_IDX_MAX},
    {.path="logic_timer",.index=701,.modelIndex=MODEL_IDX_MAX},
    {.path="logic_spawner",.index=702,.modelIndex=MODEL_IDX_MAX},
    {.path="info_teleport_destination",.index=703,.modelIndex=MODEL_IDX_MAX},
    {.path="prop_debris_panel",.index=704,.modelIndex=MODEL_IDX_MAX}, // TODO ???
    {.path="info_cyborgconversion",.index=705,.modelIndex=MODEL_IDX_MAX},
    {.path="info_elev_destination",.index=706,.modelIndex=MODEL_IDX_MAX},
    {.path="info_email",.index=707,.modelIndex=MODEL_IDX_MAX},
    {.path="info_gameend",.index=708,.modelIndex=MODEL_IDX_MAX},
    {.path="info_message",.index=709,.modelIndex=MODEL_IDX_MAX},
    {.path="info_mission",.index=710,.modelIndex=MODEL_IDX_MAX},
    {.path="info_note",.index=711,.modelIndex=MODEL_IDX_MAX},
    {.path="info_playsound",.index=712,.modelIndex=MODEL_IDX_MAX},
    {.path="info_ressurection_point",.index=713,.modelIndex=MODEL_IDX_MAX},
    {.path="info_screenshake",.index=714,.modelIndex=MODEL_IDX_MAX},
    {.path="info_spawnpoint",.index=715,.modelIndex=MODEL_IDX_MAX},
    {.path="fx_reverbzone",.index=716,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_cyber_ice",.index=717,.collider=COLLIDER_TYPE_SPHERE,.colliderCenter=(Vector3){0.0f,0.004354001f,-0.014725f},.colliderSize=(Vector3){1.0f,0.0f,0.0f},.colliderMeshIndex=U16_MAX},
    {.path="ef_fragexplosion",.index=718,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_line_sparqbeam",.index=719,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_mist",.index=720,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_bloodspurtsmall",.index=721,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_bloodspurtsmallgreen",.index=722,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_bloodspurtsmallyellow",.index=723,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_bloodspurttiny",.index=724,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_camerahit",.index=725,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_darthit",.index=726,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_sec2muzburst",.index=727,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_sec2rotmuzburst",.index=728,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_sparksmall",.index=729,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_sparksmallblue",.index=730,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_sparqhit",.index=731,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_sparkspits",.index=732,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_spraydrips",.index=733,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_steam",.index=734,.modelIndex=MODEL_IDX_MAX},
    {.path="env_sparksmall",.index=735,.modelIndex=MODEL_IDX_MAX},
    {.path="TargetIDInstance",.index=736,.modelIndex=MODEL_IDX_MAX},
    {.path="prop_papers01",.index=737,.modelIndex=484,.texIndex=580},
    {.path="prop_papers02",.index=738,.modelIndex=485,.texIndex=580},
    {.path="ef_particle_blasterhit",.index=739,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_ionhit",.index=740,.modelIndex=MODEL_IDX_MAX},
    {.path="us_puz_panel_blue_wire",.index=741,.modelIndex=0,.texIndex=604,.numclips=3,.animationNum=43},
    {.path="us_puz_panel_brown_wire",.index=742,.modelIndex=0,.texIndex=631,.numclips=3,.animationNum=43},
    {.path="us_puz_panel_gray_wire",.index=743,.modelIndex=0,.texIndex=634,.numclips=3,.animationNum=43},
    {.path="us_puz_panel_red_wire",.index=744,.modelIndex=0,.texIndex=625,.numclips=3,.animationNum=43},
    {.path="us_puz_panel_teal_wire",.index=745,.modelIndex=0,.texIndex=601,.numclips=3,.animationNum=43},
    {.path="weapon_grenadeenergmine_live",.index=746,.modelIndex=169,.texIndex=852}, // TODO tex anim hook up at init
    {.path="decal_logo_storage",.index=747,.modelIndex=77,.texIndex=169,.shadows=false},
    {.path="light_animated",.index=748,.modelIndex=MODEL_IDX_MAX},
    {.path="generic_transform",.index=749,.modelIndex=MODEL_IDX_MAX},
    {.path="chunk_crate_impenetrable2",.index=750,.modelIndex=61,.texIndex=147},
    {.path="chunk_crate_impenetrable3",.index=751,.modelIndex=61,.texIndex=148},
    {.path="chunk_crate_impenetrable4",.index=752,.modelIndex=61,.texIndex=149},
    {.path="npc_sec3_bot",.index=753,.modelIndex=681,.texIndex=553},
    {.path="prop_shieldgenerator",.index=754,.modelIndex=143,.texIndex=316},
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 755 cameraview now handled directly in mod code instead of as an entity.
    {.path="ef_particle_leafburst",.index=756,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_mutationburst",.index=757,.modelIndex=MODEL_IDX_MAX},
    {.path="ef_particle_graytationburst",.index=758,.modelIndex=MODEL_IDX_MAX},
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 759 stop
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 760 judging
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 761 me
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 762 alright!
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 763 I
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 764 didn't feel
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 765 like
    {.index=U16_MAX,.modelIndex=MODEL_IDX_MAX}, // 766 reordering everything!
    {.path="player",.index=767,.modelIndex=MODEL_IDX_MAX}
};

#define GEOMETRY_LOD_CARD_MODEL_IDX 178
MOD_TO_ENGINE void ModEntityDefinitionsInitAfterLoad(void) { // Global conditions for all entities.  No sense inflating the table data in entity.c
    for (i32 i = 0; i < MAX_ENTITIES; i++) {
        if (EntityDefinitions[i].index == U16_MAX) continue;
        
        EntityDefinitions[i].rotation = QUAT_IDENTITY;
        EntityDefinitions[i].lodIndex = MODEL_IDX_MAX;
        if (!EntityDefinitions[i].layer) EntityDefinitions[i].layer = PhysicsLayer_Default;
        // TODO REMOVE!  TESTING WITH SPHERES FOR EVERYTHING NON BOX!
        if (EntityDefinitions[i].collider != COLLIDER_TYPE_BOX) {
            EntityDefinitions[i].collider = COLLIDER_TYPE_SPHERE;
            EntityDefinitions[i].colliderCenter=(Vector3){0.0f,0.0f,0.0f};
            EntityDefinitions[i].colliderSize=(Vector3){0.16f,0.16f,0.16f};
        }
        
        EntityDefinitions[i].velocity = (Vector3){0.0f,0.0f,0.0f};
        flag_set(&EntityDefinitions[i].entflags,ENTFLAG_ACTIVE,true); // Individual value setting to allow mods to set custom starting flags themselves. (or here too if they want, tis your oyster).
        flag_set(&EntityDefinitions[i].entflags,ENTFLAG_RIGIDBODY,ConstIndexIsDynamicObject(EntityDefinitions[i].index));
        if (EntityDefinitions[i].cardchunk) {
            EntityDefinitions[i].lodIndex = GEOMETRY_LOD_CARD_MODEL_IDX;
            EntityDefinitions[i].collider = COLLIDER_TYPE_BOX;
            EntityDefinitions[i].colliderCenter = (Vector3){0.0f,1.44f,0.0f};
            EntityDefinitions[i].colliderSize = (Vector3){2.56f,0.32f,2.56f};
        }
        
        EntityDefinitions[i].currentFrameFinished = Eng_Global->pauseRelativeTime + 0.1;
        if (ConstIndexIsButtonSwitch(EntityDefinitions[i].index)) {
            EntityDefinitions[i].lockedMessageLingdex = 193; // ButtonSwitch
            EntityDefinitions[i].tickTime = 1.5;
        }
    }
    
}

u16 AddInstance(u16 entIdx, Vector3 pos) {
    if (entIdx >= MAX_ENTITIES) { DualLogError("\nEntity index when loading non-light entity was %d, exceeds max defined entity count of %d, skipped\n",entIdx,MAX_ENTITIES); return INSTANCE_COUNT; }
    
    u16 i = Eng_Global->loadedInstances;
    Entity* e = &Eng_Global->instances[i];
    e->index = entIdx;
    e->position = pos;
    if (ConstIndexIsNPC(entIdx)) InitializeAIAfterLoad(i);
    e->cardchunk = EntityDefinitions[entIdx].cardchunk;
    e->modelIndex = EntityDefinitions[entIdx].modelIndex;
    e->colliderMeshIndex = EntityDefinitions[entIdx].colliderMeshIndex;
    e->numclips = EntityDefinitions[entIdx].numclips;
    e->animationNum = EntityDefinitions[entIdx].animationNum;
    e->texIndex = EntityDefinitions[entIdx].texIndex;
    e->glowIndex = EntityDefinitions[entIdx].glowIndex >= MAX_VALID_TEXTURE ? 0 : EntityDefinitions[entIdx].glowIndex;
    e->specIndex = EntityDefinitions[entIdx].specIndex >= MAX_VALID_TEXTURE ? 0 : EntityDefinitions[entIdx].specIndex;
    e->normIndex = EntityDefinitions[entIdx].normIndex >= MAX_VALID_TEXTURE ? 0 : EntityDefinitions[entIdx].normIndex;
    e->lodIndex = EntityDefinitions[entIdx].lodIndex;
    e->gravity = EntityDefinitions[entIdx].gravity >= 0.0f ? EntityDefinitions[entIdx].gravity : 0.0f; // No up falling.
    flag_set(&e->entflags,ENTFLAG_KINEMATIC,EntityDefinitions[entIdx].entflags & ENTFLAG_KINEMATIC);
    flag_set(&e->entflags,ENTFLAG_RIGIDBODY,EntityDefinitions[entIdx].entflags & ENTFLAG_RIGIDBODY);
    flag_set(&e->entflags,ENTFLAG_NO_SHADOWS, EntityDefinitions[entIdx].entflags & ENTFLAG_NO_SHADOWS);
    e->collider = EntityDefinitions[entIdx].collider;
    e->colliderCenter = EntityDefinitions[entIdx].colliderCenter;
    e->colliderSize = EntityDefinitions[entIdx].colliderSize;
    e->mass = EntityDefinitions[entIdx].mass > 0.0f ? EntityDefinitions[entIdx].mass : 1.0f; // Nonzero fallback.
    e->linearDrag = EntityDefinitions[entIdx].linearDrag > 0.0f ? EntityDefinitions[entIdx].linearDrag : 0.0f;
    e->angularDrag = EntityDefinitions[entIdx].angularDrag > 0.0f ? EntityDefinitions[entIdx].angularDrag : 0.05f;    
    Eng_Global->instances[i].lockedMessageLingdex = EntityDefinitions[entIdx].lockedMessageLingdex;
    Eng_Global->dirtyInstances[i] = true;
    Eng_Global->loadedInstances++;
    return i;
}

void DeleteInstance(u16 i) {
    if (i <= PLAYER2 || i >= Eng_Global->loadedInstances) return; // Don't delete null ent, player 1, nor player 2 or already empty slots.
    
    u16 endInstance = vmax(vmin(INSTANCE_COUNT - 1, Eng_Global->loadedInstances - 1),START_INDEX_LEVEL_INSTANCES);
//     for (;i<endInstance;++i) Eng_Global->instances[i] = Eng_Global->instances[i + 1]; // Shift the entire list down, overwriting the entity we're deleting at starting i
    for (;i<endInstance;++i) __builtin_memcpy(&Eng_Global->instances[i], &Eng_Global->instances[i+1], sizeof(Entity));
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
    SetMemoryToValueForNBytes(entsFromFile,0,INSTANCE_COUNT * sizeof(Entity));
    SetMemoryToValueForNBytes(lightsFromFile,0,LIGHT_COUNT * sizeof(Light));
    SetMemoryToValueForNBytes(lanimsFromFile,0,LIGHT_COUNT * sizeof(LightAnimation));
    for (int i = 0; i < LIGHT_COUNT; ++i) lightsFromFile[i].lflags = LIGHT_AND_SHADOW_ON;
    u32 lineNum = 0;
    i32 entCount = -1;  // incremented to 0 on first entity line
    i32 lightsIdx = -1; // incremented to 0 on first light line
    char lineSpace[LINE_LEN_MAX];
    char* line = &lineSpace[0];
    char firstKeyCheck[11];
    char initialLine[LINE_LEN_MAX];

    while (GetLevelFileNextStringUpToNewlineOrEOF(lineSpace, LINE_LEN_MAX)) {
        size_t len = GetStringLength(lineSpace);
        while (len && (lineSpace[len - 1] == '\n' || lineSpace[len - 1] == '\r'))
            lineSpace[--len] = '\0';
        line = lineSpace;
        StringFormat(initialLine, sizeof(initialLine), "%s", line);
        __builtin_memcpy(firstKeyCheck, line, 10); firstKeyCheck[10] = '\0';
        lineNum++;
        bool isLight = !StringsEqual(firstKeyCheck, "constIndex");
        if (isLight) {
            lightsIdx++;
            if (lightsIdx >= LIGHT_COUNT) { DualLogError("Too many lights %u in level%d.txt!\n", lightsIdx, curlevel); continue; }
        } else {
            entCount++;
            if (entCount >= INSTANCE_COUNT) { DualLogError("Too many instances %u in level%d.txt!\n", entCount, curlevel); continue; }
        }

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
                else if (StringsEqual(trimmed_key,"requireReset"))    flag_set(&inst->entflags, ENTFLAG_REQUIRE_RESET, parse_bool(trimmed_value,initialLine,lineNum));
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
                else if (StringsEqual(trimmed_key,"doorOpen"))        flag_set(&inst->ioflags,TARG_IOFLAGS_DOOROPEN,parse_bool(trimmed_value, initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorOpenIfUnlocked")) flag_set(&inst->ioflags,TARG_IOFLAGS_DOOROPENIFUNLOCKED, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorClose"))       flag_set(&inst->ioflags,TARG_IOFLAGS_DOORCLOSE,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorLock"))        flag_set(&inst->ioflags,TARG_IOFLAGS_DOORLOCK,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorUnlock"))      flag_set(&inst->ioflags,TARG_IOFLAGS_DOORUNLOCK,parse_bool(trimmed_value,initialLine,lineNum));
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
                else if (StringsEqual(trimmed_key,"sendEmail"))       flag_set(&inst->ioflags,TARG_IOFLAGS_SEND_EMAIL,parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"switchLockToggle")) flag_set(&inst->ioflags,TARG_IOFLAGS_SWITCH_LOCK_TOGGLE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"spawnerActivate")) flag_set(&inst->ioflags,TARG_IOFLAGS_SPAWNER_ACTIVATE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"spawnerActivateAlerted")) flag_set(&inst->ioflags,TARG_IOFLAGS_SPAWNER_ACTALERTED, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"cyborgConversionToggle")) flag_set(&inst->ioflags,TARG_IOFLAGS_CYBORG_CONV_TOGGLE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"GOSetActive"))     flag_set(&inst->ioflags,TARG_IOFLAGS_INST_ACTIVATE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"GOSetDeactive"))   flag_set(&inst->ioflags,TARG_IOFLAGS_INST_DEACTIVATE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"GOToggleActive"))  flag_set(&inst->ioflags,TARG_IOFLAGS_INST_TOGGLE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"disableThisGOOnAwake")) flag_set(&inst->ioflags,TARG_IOFLAGS_DISABLE_ON_AWAKE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"playSoundOnce"))   flag_set(&inst->ioflags,TARG_IOFLAGS_PLAY_SOUND_ONCE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"stopSound"))       flag_set(&inst->ioflags,TARG_IOFLAGS_STOP_SOUND, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"sendSprintMessage")) flag_set(&inst->ioflags,TARG_IOFLAGS_SEND_CENTERPRINT, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"radiationTreatment")) flag_set(&inst->ioflags,TARG_IOFLAGS_RADIATION_TREATMNT, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"startFlashingMaterials")) flag_set(&inst->ioflags,TARG_IOFLAGS_START_FLASHING_TEX, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"stopFlashingMaterials")) flag_set(&inst->ioflags,TARG_IOFLAGS_STOP_FLASHING_TEX, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"unlockElevatorPad")) flag_set(&inst->ioflags,TARG_IOFLAGS_UNLOCK_ELEVATORPAD, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"unlockKeycodePad")) flag_set(&inst->ioflags,TARG_IOFLAGS_UNLOCK_KEYPAD, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"unlockPuzzlePad")) flag_set(&inst->ioflags,TARG_IOFLAGS_UNLOCK_PUZPAD, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"screenShake"))     flag_set(&inst->ioflags,TARG_IOFLAGS_SCREENSHAKE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"awakeSleepingEnemy")) flag_set(&inst->ioflags,TARG_IOFLAGS_AWAKE_SLEEPING_NPC, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"branchFlip"))      flag_set(&inst->ioflags,TARG_IOFLAGS_BRANCH_FLIP, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"branchFlipOnly"))  flag_set(&inst->ioflags,TARG_IOFLAGS_BRANCH_FLIPONLY, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorAccessCardOverrideToggle")) flag_set(&inst->ioflags, TARG_IOFLAGS_TOG_DORACESOVERIDE, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"unlockSwitch"))    flag_set(&inst->ioflags,TARG_IOFLAGS_UNLOCK_SWITCH, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"lockElevatorPad")) flag_set(&inst->ioflags,TARG_IOFLAGS_LOCK_ELEVATORPAD, parse_bool(trimmed_value,initialLine,lineNum));
                else if (StringsEqual(trimmed_key,"doorToggle"))      flag_set(&inst->ioflags,TARG_IOFLAGS_DOOR_TOGGLE, parse_bool(trimmed_value,initialLine,lineNum));
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

        // Store activeStateRead alongside the parsed entity so the commit pass can use it.
        // Reuse a spare field or parallel array — here we use a bit in entflags as a sentinel.
        if (!isLight && !activeStateRead) flag_set(&entsFromFile[entCount].entflags, ENTFLAG_ACTIVE, true); // Default active if not specified
    }

    // --- Commit pass: push all parsed entities through AddInstance ---
    DualLog("Ended level parse with %u entCount\n",entCount);
    i32 totalEnts = entCount + 1;
    for (i32 e = 0; e < totalEnts; ++e) {
        Entity* src = &entsFromFile[e];
        u16 entIdx = src->index;
        u16 parent = AddInstance(entIdx,src->position);
        Entity* par = &Eng_Global->instances[parent];
        par->rotation              = src->rotation;
        par->scale                 = src->scale;
        par->entflags             |= src->entflags; // Merge (AddInstance already set definition flags)
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
    
    for (int i = 0; i < lightsIdx; ++i) { if (!(lightsFromFile[i].lflags & LSPOT)){lightsFromFile[i].spotAng=0.0f;} AddLight(&lightsFromFile[i], &lanimsFromFile[i]); } // Add all level lights

    // Shield generators
    if (curlevel == 1 || curlevel == 2 || curlevel == 5 || curlevel == 6 || curlevel == 7) {
        u16 shd1 = AddInstance(754, (Vector3){-51.30664f,  -47.42f,  56.42651f}); Eng_Global->instances[shd1].rotation = (Quaternion){0.0f,0.0f,0.0f,1.0f};
        u16 shd2 = AddInstance(754, (Vector3){ 71.5f,      -47.42f, -66.6f    }); Eng_Global->instances[shd2].rotation = (Quaternion){0.0f,0.0f,0.0f,1.0f};
        u16 shd3 = AddInstance(754, (Vector3){-51.306650f, -47.42f, -66.66652f}); Eng_Global->instances[shd3].rotation = (Quaternion){0.0f,0.0f,0.0f,1.0f};
        u16 shd4 = AddInstance(754, (Vector3){ 71.78664f,  -47.42f,  56.42651f}); Eng_Global->instances[shd4].rotation = (Quaternion){0.0f,0.0f,0.0f,1.0f};
    }

    Light hl = (Light){.pos=Eng_Global->instances[PLAYER1].position,.col=(Color3){1.0f,1.0f,1.0f},.range=11.52f,.lflags=LIGHTON,.intensity=0.0f,.minIntensity=0.0f,.maxIntensity=0.0f,.spotAng=0.0f,.spotDir=QUAT_IDENTITY};
    LightAnimation lam = {0};
    headmountedLanternLight = AddLight(&hl, &lam); lightsIdx++;
    Color c = fogLUT[curlevel]; c.a *= 3.8f;
    Eng_Global->fogColor = c;
}
