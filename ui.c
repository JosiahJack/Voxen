/*ui.c - User Interface(UI) aka HUD*/

/*UIRegionID: explicit index per UI component/module. Button with image+text is one ID; standalone text & image each get own ID.*/
typedef enum {UI_ID_NONE=0,
    /*Menu*/
    UI_ID_MENU_BACKGROUND=1,UI_ID_MENU_CONFIG_BACKGROUND=2,UI_ID_MENU_NEWGAME_BACKGROUND=3,UI_ID_MENU_NEWGAME_INSET=4,UI_ID_MENU_TITLE=5,UI_ID_MENU_BACK_BUTTON=6,UI_ID_MENU_SINGLEPLAYER=7,UI_ID_MENU_MULTIPLAYER=8,UI_ID_MENU_OPTIONS=9,UI_ID_MENU_QUIT=10,UI_ID_MENU_CONTINUE=11,UI_ID_MENU_NEW_GAME=12,UI_ID_MENU_PLAY_INTRO=13,UI_ID_MENU_PLAY_CREDITS=14,UI_ID_MENU_LOAD=15,UI_ID_MENU_CONFIG_TAB_GRAPHICS=16,UI_ID_MENU_CONFIG_TAB_INPUT=17,
    UI_ID_MENU_CONFIG_TAB_AUDIO_LANG=18,UI_ID_MENU_CONFIG_TAB_HILITE=19,UI_ID_MENU_CONFIG_TAB_UNHILITE=20,UI_ID_MENU_CHECKBOX=21,UI_ID_MENU_CHECKBOX_CHECK=22,UI_ID_MENU_SLIDER=23,UI_ID_MENU_FXAA=24,UI_ID_MENU_SSR=25,UI_ID_MENU_VSYNC=26,UI_ID_MENU_SHADOWS=27,UI_ID_MENU_MODEL_DETAIL=28,UI_ID_MENU_FOV_SLIDER=29,UI_ID_MENU_GAMMA_SLIDER=30,UI_ID_MENU_RESOLUTION=31,UI_ID_MENU_FULLSCREEN=32,UI_ID_MENU_MASTER_VOLUME_SLIDER=33,UI_ID_MENU_MUSIC_VOLUME_SLIDER=34,
    UI_ID_MENU_TOGGLE_MONITOR=35,UI_ID_MENU_NAME_INPUT=36,UI_ID_MENU_DIFFICULTY=37,UI_ID_MENU_START=38,
    /*Pause*/
    UI_ID_PAUSE_BACKGROUND=39,UI_ID_PAUSE_BACKGROUND_OUTLINE=40,UI_ID_PAUSE_QUIT_BACKGROUND=41,UI_ID_PAUSE_RESUME=42,UI_ID_PAUSE_LOAD=43,UI_ID_PAUSE_SAVE=44,UI_ID_PAUSE_OPTIONS=45,UI_ID_PAUSE_QUIT_TO_MENU=46,UI_ID_PAUSE_QUIT_GAME=47,
    /*HUD*/
    UI_ID_HUD_ENERGY_INDICATOR=48,UI_ID_HUD_HEALTH_INDICATOR=49,UI_ID_HUD_ENERGY_TICK=50,UI_ID_HUD_WEAPON_ICON=51,
    /*MFD side tabs*/
    UI_ID_LMFD_WEAPON_TAB_BUTTON=52,UI_ID_LMFD_ITEM_TAB_BUTTON=53,UI_ID_LMFD_AUTOMAP_TAB_BUTTON=54,UI_ID_LMFD_DATA_TAB_BUTTON=55,UI_ID_LMFD_WEAPON_NAME=56,UI_ID_LMFD_WEAPON_ICON=57,UI_ID_RMFD_WEAPON_TAB_BUTTON=58,UI_ID_RMFD_ITEM_TAB_BUTTON=59,UI_ID_RMFD_AUTOMAP_TAB_BUTTON=60,UI_ID_RMFD_DATA_TAB_BUTTON=61,UI_ID_RMFD_WEAPON_NAME=62,UI_ID_RMFD_WEAPON_ICON=63,
    /*LMFD data views*/
    UI_ID_LMFD_BLOCKED_SECURITY_TEXT=64,UI_ID_LMFD_ELEV_CURRENT_FLOOR_INDICATOR=65,UI_ID_LMFD_ELEV_BUTTON_BANK=66,UI_ID_LMFD_ELEV_BUTTON_1=67,UI_ID_LMFD_ELEV_BUTTON_2=68,UI_ID_LMFD_ELEV_BUTTON_3=69,UI_ID_LMFD_ELEV_BUTTON_4=70,UI_ID_LMFD_ELEV_BUTTON_5=71,UI_ID_LMFD_ELEV_BUTTON_6=72,UI_ID_LMFD_ELEV_BUTTON_7=73,UI_ID_LMFD_ELEV_BUTTON_8=74,UI_ID_LMFD_ELEV_TEXT_1=75,UI_ID_LMFD_ELEV_TEXT_2=76,UI_ID_LMFD_ELEV_TEXT_3=77,UI_ID_LMFD_ELEV_TEXT_4=78,
    UI_ID_LMFD_ELEV_TEXT_5=79,UI_ID_LMFD_ELEV_TEXT_6=80,UI_ID_LMFD_ELEV_TEXT_7=81,UI_ID_LMFD_ELEV_TEXT_8=82,UI_ID_LMFD_ELEV_CLOSE_BUTTON=83,UI_ID_LMFD_KEYCODE_BUTTON_0=84,UI_ID_LMFD_KEYCODE_BUTTON_1=85,UI_ID_LMFD_KEYCODE_BUTTON_2=86,UI_ID_LMFD_KEYCODE_BUTTON_3=87,UI_ID_LMFD_KEYCODE_BUTTON_4=88,UI_ID_LMFD_KEYCODE_BUTTON_5=89,UI_ID_LMFD_KEYCODE_BUTTON_6=90,UI_ID_LMFD_KEYCODE_BUTTON_7=91,UI_ID_LMFD_KEYCODE_BUTTON_8=92,UI_ID_LMFD_KEYCODE_BUTTON_9=93,
    UI_ID_LMFD_KEYCODE_BUTTON_BACKSPACE=94,UI_ID_LMFD_KEYCODE_BUTTON_C=95,UI_ID_LMFD_KEYCODE_DIGIT_HUNDREDS=96,UI_ID_LMFD_KEYCODE_DIGIT_TENS=97,UI_ID_LMFD_KEYCODE_DIGIT_ONES=98,UI_ID_LMFD_KEYCODE_CLOSE_BUTTON=99,UI_ID_LMFD_AUDIOLOG_IMAGE=100,UI_ID_LMFD_AUDIOLOG_NAME=101,UI_ID_LMFD_AUDIOLOG_SENDER=102,UI_ID_LMFD_AUDIOLOG_SUBJECT=103,UI_ID_LMFD_PUZZLE_OUTER_BORDER=104,UI_ID_LMFD_PUZZLE_CONTAINER=105,UI_ID_LMFD_PUZZLE_NODE_SOURCE=106,UI_ID_LMFD_PUZZLE_NODE=107,
    UI_ID_LMFD_PUZZLE_CELL_0=108,UI_ID_LMFD_PUZZLE_CELL_1=109,UI_ID_LMFD_PUZZLE_CELL_2=110,UI_ID_LMFD_PUZZLE_CELL_3=111,UI_ID_LMFD_PUZZLE_CELL_4=112,UI_ID_LMFD_PUZZLE_CELL_5=113,UI_ID_LMFD_PUZZLE_CELL_6=114,UI_ID_LMFD_PUZZLE_CELL_7=115,UI_ID_LMFD_PUZZLE_CELL_8=116,UI_ID_LMFD_PUZZLE_CELL_9=117,UI_ID_LMFD_PUZZLE_CELL_10=118,UI_ID_LMFD_PUZZLE_CELL_11=119,UI_ID_LMFD_PUZZLE_CELL_12=120,UI_ID_LMFD_PUZZLE_CELL_13=121,UI_ID_LMFD_PUZZLE_CELL_14=122,
    UI_ID_LMFD_PUZZLE_CELL_15=123,UI_ID_LMFD_PUZZLE_CELL_16=124,UI_ID_LMFD_PUZZLE_CELL_17=125,UI_ID_LMFD_PUZZLE_CELL_18=126,UI_ID_LMFD_PUZZLE_CELL_19=127,UI_ID_LMFD_PUZZLE_CELL_20=128,UI_ID_LMFD_PUZZLE_CELL_21=129,UI_ID_LMFD_PUZZLE_CELL_22=130,UI_ID_LMFD_PUZZLE_CELL_23=131,UI_ID_LMFD_PUZZLE_CELL_24=132,UI_ID_LMFD_PUZZLE_CELL_25=133,UI_ID_LMFD_PUZZLE_CELL_26=134,UI_ID_LMFD_PUZZLE_CELL_27=135,UI_ID_LMFD_PUZZLE_CELL_28=136,UI_ID_LMFD_PUZZLE_CELL_29=137,
    UI_ID_LMFD_PUZZLE_CELL_30=138,UI_ID_LMFD_PUZZLE_CELL_31=139,UI_ID_LMFD_PUZZLE_CELL_32=140,UI_ID_LMFD_PUZZLE_CELL_33=141,UI_ID_LMFD_PUZZLE_PROGRESS_BAR=142,UI_ID_LMFD_PUZZLE_FILL=143,UI_ID_LMFD_PUZZLE_HANDLE=144,UI_ID_LMFD_PUZZLE_CLOSE_BUTTON=145,UI_ID_LMFD_WIRE_CONTAINER_CENTER=146,UI_ID_LMFD_WIRE_LEVELS_BOX=147,UI_ID_LMFD_WIRE_FILL=148,UI_ID_LMFD_WIRE_HANDLE=149,UI_ID_LMFD_WIRE_TARGET_LINE=150,UI_ID_LMFD_WIRE_NODE_0=151,UI_ID_LMFD_WIRE_NODE_1=152,
    UI_ID_LMFD_WIRE_NODE_2=153,UI_ID_LMFD_WIRE_NODE_3=154,UI_ID_LMFD_WIRE_NODE_4=155,UI_ID_LMFD_WIRE_NODE_5=156,UI_ID_LMFD_WIRE_NODE_6=157,UI_ID_LMFD_WIRE_NODE_7=158,UI_ID_LMFD_WIRE_NODE_8=159,UI_ID_LMFD_WIRE_NODE_9=160,UI_ID_LMFD_WIRE_NODE_10=161,UI_ID_LMFD_WIRE_NODE_11=162,UI_ID_LMFD_WIRE_NODE_12=163,UI_ID_LMFD_WIRE_CLOSE_BUTTON=164,UI_ID_LMFD_SYS_HEADER=165,UI_ID_LMFD_SYS_DESC_LEVEL_SECURITY=166,UI_ID_LMFD_SYS_VAL_LEVEL_SECURITY=167,
    UI_ID_LMFD_SYS_DESC_MINING_LASER=168,UI_ID_LMFD_SYS_VAL_MINING_LASER=169,UI_ID_LMFD_SYS_DESC_LIFEPODS=170,UI_ID_LMFD_SYS_VAL_LIFEPODS=171,UI_ID_LMFD_SYS_DESC_SHIELD=172,UI_ID_LMFD_SYS_VAL_SHIELD=173,UI_ID_LMFD_SYS_DESC_REACTOR=174,UI_ID_LMFD_SYS_VAL_REACTOR=175,UI_ID_LMFD_SYS_DESC_PROCESSORS=176,UI_ID_LMFD_SYS_VAL_PROCESSORS=177,UI_ID_LMFD_SYS_DESC_MAIN_PROGRAM=178,UI_ID_LMFD_SYS_VAL_MAIN_PROGRAM=179,UI_ID_LMFD_SYS_DESC_GROVE_ALPHA=180,
    UI_ID_LMFD_SYS_VAL_GROVE_ALPHA=181,UI_ID_LMFD_SYS_DESC_GROVE_BETA=182,UI_ID_LMFD_SYS_VAL_GROVE_BETA=183,UI_ID_LMFD_SYS_DESC_GROVE_GAMMA=184,UI_ID_LMFD_SYS_VAL_GROVE_GAMMA=185,UI_ID_LMFD_SYS_DESC_GROVE_DELTA=186,UI_ID_LMFD_SYS_VAL_GROVE_DELTA=187,UI_ID_LMFD_SYS_CLOSE_BUTTON=188,UI_ID_LMFD_MINIGAMES_CONTAINER=189,UI_ID_LMFD_MINIGAMES_HEADER=190,UI_ID_LMFD_MINIGAME_PING=191,UI_ID_LMFD_MINIGAME_15=192,UI_ID_LMFD_MINIGAME_WING0=193,
    UI_ID_LMFD_MINIGAME_BOTBOUNCE=194,UI_ID_LMFD_MINIGAME_EELZAPPER=195,UI_ID_LMFD_MINIGAME_ROAD=196,UI_ID_LMFD_MINIGAME_TRIOPTOE=197,UI_ID_LMFD_MINIGAME_CORP_CONQ=198,UI_ID_LMFD_MINIGAME_CHESS=199,UI_ID_LMFD_MINIGAMES_FOOTER=200,UI_ID_LMFD_MINIGAME_BACK=201,UI_ID_LMFD_MINIGAME_CLOSE=202,UI_ID_LMFD_MINIGAME_VIEW=203,UI_ID_LMFD_MINIGAME_GAME_OVER=204,UI_ID_LMFD_SEARCH_OBJECT_NAME=205,UI_ID_LMFD_SEARCH_ITEM_ICON_0=206,UI_ID_LMFD_SEARCH_ITEM_ICON_1=207,
    UI_ID_LMFD_SEARCH_ITEM_ICON_2=208,UI_ID_LMFD_SEARCH_ITEM_ICON_3=209,UI_ID_LMFD_SEARCH_EMPTY_TEXT=210,UI_ID_LMFD_SEARCH_CLOSE=211,UI_ID_LMFD_GENERAL_ITEM_NAME=212,UI_ID_LMFD_GENERAL_ITEM_ICON_0=213,UI_ID_LMFD_GENERAL_ITEM_ICON_1=214,UI_ID_LMFD_GENERAL_ITEM_ICON_2=215,UI_ID_LMFD_GENERAL_ITEM_ICON_3=216,UI_ID_LMFD_GENERAL_ITEM_EMPTY_TEXT=217,UI_ID_LMFD_GENERAL_ITEM_CLOSE=218,
    /*RMFD data views*/
    UI_ID_RMFD_BLOCKED_SECURITY_TEXT=219,UI_ID_RMFD_ELEV_CURRENT_FLOOR_INDICATOR=220,UI_ID_RMFD_ELEV_BUTTON_BANK=221,UI_ID_RMFD_ELEV_BUTTON_1=222,UI_ID_RMFD_ELEV_BUTTON_2=223,UI_ID_RMFD_ELEV_BUTTON_3=224,UI_ID_RMFD_ELEV_BUTTON_4=225,UI_ID_RMFD_ELEV_BUTTON_5=226,UI_ID_RMFD_ELEV_BUTTON_6=227,UI_ID_RMFD_ELEV_BUTTON_7=228,UI_ID_RMFD_ELEV_BUTTON_8=229,UI_ID_RMFD_ELEV_TEXT_1=230,UI_ID_RMFD_ELEV_TEXT_2=231,UI_ID_RMFD_ELEV_TEXT_3=232,UI_ID_RMFD_ELEV_TEXT_4=233,
    UI_ID_RMFD_ELEV_TEXT_5=234,UI_ID_RMFD_ELEV_TEXT_6=235,UI_ID_RMFD_ELEV_TEXT_7=236,UI_ID_RMFD_ELEV_TEXT_8=237,UI_ID_RMFD_ELEV_CLOSE_BUTTON=238,UI_ID_RMFD_KEYCODE_BUTTON_0=239,UI_ID_RMFD_KEYCODE_BUTTON_1=240,UI_ID_RMFD_KEYCODE_BUTTON_2=241,UI_ID_RMFD_KEYCODE_BUTTON_3=242,UI_ID_RMFD_KEYCODE_BUTTON_4=243,UI_ID_RMFD_KEYCODE_BUTTON_5=244,UI_ID_RMFD_KEYCODE_BUTTON_6=245,UI_ID_RMFD_KEYCODE_BUTTON_7=246,UI_ID_RMFD_KEYCODE_BUTTON_8=247,
    UI_ID_RMFD_KEYCODE_BUTTON_9=248,UI_ID_RMFD_KEYCODE_BUTTON_BACKSPACE=249,UI_ID_RMFD_KEYCODE_BUTTON_C=250,UI_ID_RMFD_KEYCODE_DIGIT_HUNDREDS=251,UI_ID_RMFD_KEYCODE_DIGIT_TENS=252,UI_ID_RMFD_KEYCODE_DIGIT_ONES=253,UI_ID_RMFD_KEYCODE_CLOSE_BUTTON=254,UI_ID_RMFD_AUDIOLOG_IMAGE=255,UI_ID_RMFD_AUDIOLOG_NAME=256,UI_ID_RMFD_AUDIOLOG_SENDER=257,UI_ID_RMFD_AUDIOLOG_SUBJECT=258,UI_ID_RMFD_PUZZLE_OUTER_BORDER=259,UI_ID_RMFD_PUZZLE_CONTAINER=260,
    UI_ID_RMFD_PUZZLE_NODE_SOURCE=261,UI_ID_RMFD_PUZZLE_NODE=262,UI_ID_RMFD_PUZZLE_CELL_0=263,UI_ID_RMFD_PUZZLE_CELL_1=264,UI_ID_RMFD_PUZZLE_CELL_2=265,UI_ID_RMFD_PUZZLE_CELL_3=266,UI_ID_RMFD_PUZZLE_CELL_4=267,UI_ID_RMFD_PUZZLE_CELL_5=268,UI_ID_RMFD_PUZZLE_CELL_6=269,UI_ID_RMFD_PUZZLE_CELL_7=270,UI_ID_RMFD_PUZZLE_CELL_8=271,UI_ID_RMFD_PUZZLE_CELL_9=272,UI_ID_RMFD_PUZZLE_CELL_10=273,UI_ID_RMFD_PUZZLE_CELL_11=274,UI_ID_RMFD_PUZZLE_CELL_12=275,
    UI_ID_RMFD_PUZZLE_CELL_13=276,UI_ID_RMFD_PUZZLE_CELL_14=277,UI_ID_RMFD_PUZZLE_CELL_15=278,UI_ID_RMFD_PUZZLE_CELL_16=279,UI_ID_RMFD_PUZZLE_CELL_17=280,UI_ID_RMFD_PUZZLE_CELL_18=281,UI_ID_RMFD_PUZZLE_CELL_19=282,UI_ID_RMFD_PUZZLE_CELL_20=283,UI_ID_RMFD_PUZZLE_CELL_21=284,UI_ID_RMFD_PUZZLE_CELL_22=285,UI_ID_RMFD_PUZZLE_CELL_23=286,UI_ID_RMFD_PUZZLE_CELL_24=287,UI_ID_RMFD_PUZZLE_CELL_25=288,UI_ID_RMFD_PUZZLE_CELL_26=289,UI_ID_RMFD_PUZZLE_CELL_27=290,
    UI_ID_RMFD_PUZZLE_CELL_28=291,UI_ID_RMFD_PUZZLE_CELL_29=292,UI_ID_RMFD_PUZZLE_CELL_30=293,UI_ID_RMFD_PUZZLE_CELL_31=294,UI_ID_RMFD_PUZZLE_CELL_32=295,UI_ID_RMFD_PUZZLE_CELL_33=296,UI_ID_RMFD_PUZZLE_PROGRESS_BAR=297,UI_ID_RMFD_PUZZLE_FILL=298,UI_ID_RMFD_PUZZLE_HANDLE=299,UI_ID_RMFD_PUZZLE_CLOSE_BUTTON=300,UI_ID_RMFD_WIRE_CONTAINER_CENTER=301,UI_ID_RMFD_WIRE_LEVELS_BOX=302,UI_ID_RMFD_WIRE_FILL=303,UI_ID_RMFD_WIRE_HANDLE=304,UI_ID_RMFD_WIRE_TARGET_LINE=305,
    UI_ID_RMFD_WIRE_NODE_0=306,UI_ID_RMFD_WIRE_NODE_1=307,UI_ID_RMFD_WIRE_NODE_2=308,UI_ID_RMFD_WIRE_NODE_3=309,UI_ID_RMFD_WIRE_NODE_4=310,UI_ID_RMFD_WIRE_NODE_5=311,UI_ID_RMFD_WIRE_NODE_6=312,UI_ID_RMFD_WIRE_NODE_7=313,UI_ID_RMFD_WIRE_NODE_8=314,UI_ID_RMFD_WIRE_NODE_9=315,UI_ID_RMFD_WIRE_NODE_10=316,UI_ID_RMFD_WIRE_NODE_11=317,UI_ID_RMFD_WIRE_NODE_12=318,UI_ID_RMFD_WIRE_CLOSE_BUTTON=319,UI_ID_RMFD_SYS_HEADER=320,UI_ID_RMFD_SYS_DESC_LEVEL_SECURITY=321,
    UI_ID_RMFD_SYS_VAL_LEVEL_SECURITY=322,UI_ID_RMFD_SYS_DESC_MINING_LASER=323,UI_ID_RMFD_SYS_VAL_MINING_LASER=324,UI_ID_RMFD_SYS_DESC_LIFEPODS=325,UI_ID_RMFD_SYS_VAL_LIFEPODS=326,UI_ID_RMFD_SYS_DESC_SHIELD=327,UI_ID_RMFD_SYS_VAL_SHIELD=328,UI_ID_RMFD_SYS_DESC_REACTOR=329,UI_ID_RMFD_SYS_VAL_REACTOR=330,UI_ID_RMFD_SYS_DESC_PROCESSORS=331,UI_ID_RMFD_SYS_VAL_PROCESSORS=332,UI_ID_RMFD_SYS_DESC_MAIN_PROGRAM=333,UI_ID_RMFD_SYS_VAL_MAIN_PROGRAM=334,
    UI_ID_RMFD_SYS_DESC_GROVE_ALPHA=335,UI_ID_RMFD_SYS_VAL_GROVE_ALPHA=336,UI_ID_RMFD_SYS_DESC_GROVE_BETA=337,UI_ID_RMFD_SYS_VAL_GROVE_BETA=338,UI_ID_RMFD_SYS_DESC_GROVE_GAMMA=339,UI_ID_RMFD_SYS_VAL_GROVE_GAMMA=340,UI_ID_RMFD_SYS_DESC_GROVE_DELTA=341,UI_ID_RMFD_SYS_VAL_GROVE_DELTA=342,UI_ID_RMFD_SYS_CLOSE_BUTTON=343,UI_ID_RMFD_MINIGAMES_CONTAINER=344,UI_ID_RMFD_MINIGAMES_HEADER=345,UI_ID_RMFD_MINIGAME_PING=346,UI_ID_RMFD_MINIGAME_15=347,
    UI_ID_RMFD_MINIGAME_WING0=348,UI_ID_RMFD_MINIGAME_BOTBOUNCE=349,UI_ID_RMFD_MINIGAME_EELZAPPER=350,UI_ID_RMFD_MINIGAME_ROAD=351,UI_ID_RMFD_MINIGAME_TRIOPTOE=352,UI_ID_RMFD_MINIGAME_CORP_CONQ=353,UI_ID_RMFD_MINIGAME_CHESS=354,UI_ID_RMFD_MINIGAMES_FOOTER=355,UI_ID_RMFD_MINIGAME_BACK=356,UI_ID_RMFD_MINIGAME_CLOSE=357,UI_ID_RMFD_MINIGAME_VIEW=358,UI_ID_RMFD_MINIGAME_GAME_OVER=359,UI_ID_RMFD_SEARCH_OBJECT_NAME=360,UI_ID_RMFD_SEARCH_ITEM_ICON_0=361,
    UI_ID_RMFD_SEARCH_ITEM_ICON_1=362,UI_ID_RMFD_SEARCH_ITEM_ICON_2=363,UI_ID_RMFD_SEARCH_ITEM_ICON_3=364,UI_ID_RMFD_SEARCH_EMPTY_TEXT=365,UI_ID_RMFD_SEARCH_CLOSE=366,UI_ID_RMFD_GENERAL_ITEM_NAME=367,UI_ID_RMFD_GENERAL_ITEM_ICON_0=368,UI_ID_RMFD_GENERAL_ITEM_ICON_1=369,UI_ID_RMFD_GENERAL_ITEM_ICON_2=370,UI_ID_RMFD_GENERAL_ITEM_ICON_3=371,UI_ID_RMFD_GENERAL_ITEM_EMPTY_TEXT=372,UI_ID_RMFD_GENERAL_ITEM_CLOSE=373,
    /*Center MFD*/
    UI_ID_CMFD_WEAPON_TAB_BUTTON=374,UI_ID_CMFD_HARDWARE_TAB_BUTTON=375,UI_ID_CMFD_GENERAL_TAB_BUTTON=376,UI_ID_CMFD_SOFTWARE_TAB_BUTTON=377,UI_ID_CMFD_EREADER_TAB_BUTTON=378,UI_ID_CMFD_WEAPON_TEXT=379,UI_ID_CMFD_AMMO_TEXT=380,UI_ID_CMFD_COLUMN_HEADERS=381,UI_ID_CMFD_HARDWARE_LIST=382,UI_ID_CMFD_SOFTWARE_LIST=383,UI_ID_CMFD_LOG_TABLE_OF_CONTENTS=384,UI_ID_CMFD_LOG_FOLDER=385,UI_ID_CMFD_LOG_TEXT_READER=386,UI_ID_CMFD_LOG_ENTRY_BUTTON=387,
    UI_ID_CMFD_COUNT_TEXT_0=388,UI_ID_CMFD_COUNT_TEXT_1=389,UI_ID_CMFD_COUNT_TEXT_2=390,UI_ID_CMFD_COUNT_TEXT_3=391,UI_ID_CMFD_COUNT_TEXT_4=392,UI_ID_CMFD_COUNT_TEXT_5=393,UI_ID_CMFD_COUNT_TEXT_6=394,UI_ID_CMFD_COUNT_TEXT_7=395,UI_ID_CMFD_COUNT_TEXT_8=396,UI_ID_CMFD_COUNT_TEXT_9=397,UI_ID_CMFD_EMAIL_TAB=398,UI_ID_CMFD_EMAIL_ENTRY=399,UI_ID_CMFD_DATA_TAB=400,UI_ID_CMFD_DATA_ENTRY=401,UI_ID_CMFD_NOTE_TOGGLE_0=402,UI_ID_CMFD_NOTE_TOGGLE_1=403,
    UI_ID_CMFD_NOTE_TOGGLE_2=404,UI_ID_CMFD_NOTE_TOGGLE_3=405,UI_ID_CMFD_NOTE_TOGGLE_4=406,UI_ID_CMFD_NOTE_TOGGLE_5=407,UI_ID_CMFD_NOTE_TOGGLE_6=408,UI_ID_CMFD_NOTE_TOGGLE_7=409,UI_ID_CMFD_NOTE_TOGGLE_8=410,UI_ID_CMFD_NOTE_TOGGLE_9=411,UI_ID_CMFD_NOTE_TOGGLE_10=412,UI_ID_CMFD_NOTE_TOGGLE_11=413,UI_ID_CMFD_NOTE_TOGGLE_12=414,UI_ID_CMFD_NOTE_TOGGLE_13=415,UI_ID_CMFD_NOTE_TOGGLE_14=416,UI_ID_CMFD_NOTE_TOGGLE_15=417,UI_ID_CMFD_NOTE_TOGGLE_16=418,
    UI_ID_CMFD_NOTE_TOGGLE_17=419,UI_ID_CMFD_NOTE_LABEL_0=420,UI_ID_CMFD_NOTE_LABEL_1=421,UI_ID_CMFD_NOTE_LABEL_2=422,UI_ID_CMFD_NOTE_LABEL_3=423,UI_ID_CMFD_NOTE_LABEL_4=424,UI_ID_CMFD_NOTE_LABEL_5=425,UI_ID_CMFD_NOTE_LABEL_6=426,UI_ID_CMFD_NOTE_LABEL_7=427,UI_ID_CMFD_NOTE_LABEL_8=428,UI_ID_CMFD_NOTE_LABEL_9=429,UI_ID_CMFD_NOTE_LABEL_10=430,UI_ID_CMFD_NOTE_LABEL_11=431,UI_ID_CMFD_NOTE_LABEL_12=432,UI_ID_CMFD_NOTE_LABEL_13=433,UI_ID_CMFD_NOTE_LABEL_14=434,
    UI_ID_CMFD_NOTE_LABEL_15=435,UI_ID_CMFD_NOTE_LABEL_16=436,UI_ID_CMFD_NOTE_LABEL_17=437,UI_ID_CMFD_MORE_BUTTON=438,UI_ID_CMFD_BACK_BUTTON=439,UI_ID_CMFD_MISSION_TIMER=440,UI_ID_CMFD_MISSION_TIMER_TEXT=441,UI_ID_CMFD_BIOMONITOR=442,UI_ID_CMFD_BIOMONITOR_HEADER=443,UI_ID_CMFD_BIOMONITOR_TEXT_HEART=444,UI_ID_CMFD_BIOMONITOR_TEXT_HEART_RATE=445,UI_ID_CMFD_BIOMONITOR_TEXT_BPM=446,UI_ID_CMFD_BIOMONITOR_TEXT_PATCH=447,UI_ID_CMFD_BIOMONITOR_TEXT_PATCH_EFFECTS=448,
    UI_ID_CMFD_BIOMONITOR_TEXT_FATIGUE=449,UI_ID_CMFD_BIOMONITOR_TEXT_FATIGUE_DETAIL=450,UI_ID_CMFD_MULTIMEDIA_HEADER=451,UI_ID_CMFD_EDIT_INFO_PANEL_BG=452,UI_ID_CMFD_VMAIL_VIEWER=453,UI_ID_COUNT=454,
} UIRegionID;

void CreateUIElement(V2 min, V2 max, u32 idx) { if (World.uiComponents[idx].initialized) return; World.uiComponents[idx].initialized=true; World.uiComponents[idx].min=min; World.uiComponents[idx].max=max; }

extern float reloadTime[16];
/*mk3,bls,drt,flch, ion,rpir,pipe,magn,magp,pstl,plsm,rail,riot,skrp,sprq,stun*/
u16 wepIconTexIndices[16]={584,636,819,1067,1068,1494,1072,1069,1070,1071,1073,1165,1989,1990,1991,1992};
const char* elevFloorLabels[14] = {"R","1","2","3","4","5","6","7","8","9","G1","G2","G4","C"};
void MFD_NewGame(void) {
    World.Sys_UI=(SystemUI){.MFD_MediaTab=MM_LOG_TABLE,.MFD_ReaderView=MFD_READER_CONTENTS,.mfdSelected={1,1,1},.mfdReturnTab={1,1,1},.consumableClickRow=-1,.generalClickSlot=-1,.generalClickItem=-1,.generalClickCustom=U16_MAX,.applyButtonReferenceIndex=-1,.linkedElevatorDoor=U16_MAX,.tetheredPGP=U16_MAX,.tetheredPWP=U16_MAX,.tetheredSearchable=U16_MAX,.tetheredKeypadElevator=U16_MAX,.tetheredKeypadKeycode=U16_MAX};
}
void MFD_GeneralChanged(void) { World.Sys_UI.generalClickSlot=-1; }
void MFD_ResetGeneral(void) { World.Sys_UI.mfdGeneralItem=false; World.Sys_UI.mfdConsumable=0; World.Sys_UI.consumableClickRow=-1; MFD_GeneralChanged(); }
void MFD_ShowGeneralItem(void) { u8 side=World.Sys_UI.lastItemSideRH?2:1; World.Sys_UI.mfdConsumable=0; World.Sys_UI.mfdGeneralItem=true; World.Sys_UI.mfdItemReader[0]=World.Sys_UI.mfdItemReader[1]=false; if (side==2) World.Sys_UI.MFD_RightTab=2; else World.Sys_UI.MFD_LefTab=2; World.Sys_UI.mfdSelected[side]=2; }
void MFD_OpenSearch(bool isRH) { for (u8 side=0;side<2;++side) {u8 tab=side?World.Sys_UI.MFD_RightTab:World.Sys_UI.MFD_LefTab,view=side?World.Sys_UI.MFD_DataR:World.Sys_UI.MFD_DataL; if (!(tab==2 && World.Sys_UI.mfdItemReader[side]) && view!=5) { World.Sys_UI.mfdReturnTab[side+1]=tab; World.Sys_UI.mfdReturnView[side+1]=view;}} World.Sys_UI.MFD_DataL=World.Sys_UI.MFD_DataR=5; if (isRH) World.Sys_UI.MFD_RightTab=4; else World.Sys_UI.MFD_LefTab=4;}
void MFD_CloseSearch(void) {for (u8 side=0;side<2;++side) {u8* tab=side?&World.Sys_UI.MFD_RightTab:&World.Sys_UI.MFD_LefTab; u8* view=side?&World.Sys_UI.MFD_DataR:&World.Sys_UI.MFD_DataL; if (*view!=5) continue; *view=World.Sys_UI.mfdReturnView[side+1]; if (*view==5) *view=0; if (*tab==4) *tab=World.Sys_UI.mfdReturnTab[side+1];}}
INLINE void MFD_SelectTab(u8 panel,u8 tab,bool toggle) {
    MFD_GeneralChanged(); if (panel && tab==2) World.Sys_UI.lastItemSideRH=panel==2; u8* current=panel==0?&World.Sys_UI.MFD_CenterTab:panel==1?&World.Sys_UI.MFD_LefTab:&World.Sys_UI.MFD_RightTab; *current=toggle && *current==tab ? 0 : tab; World.Sys_UI.mfdSelected[panel]=tab; u8 view=panel==0?0:panel==1?World.Sys_UI.MFD_DataL:World.Sys_UI.MFD_DataR; 
    if (!(panel && ((tab==2 && World.Sys_UI.mfdItemReader[panel-1]) || (tab==4 && view==5)))) { World.Sys_UI.mfdReturnTab[panel]=*current; if (view!=5) World.Sys_UI.mfdReturnView[panel]=view; } if (panel && tab==4 && view==5) World.Sys_UI.lastSearchSideRH=panel==2; play_wav(sounds[97],SfxVol(),(V3){0,0,0},false); 
}

void WeaponFireStartWeaponDip(float t);
void WeaponSelectSlot(int slot){int wi=(int)World.invP1.weaponInventoryIndices[slot]; if(wi<0||wi>=MAX_ENTITIES)return; if((int)World.invP1.weaponCurrent==slot)return; if(World.invP1.reloadFinished>World.pauseRelativeTime)return; World.invP1.weaponCurrentPending=(i16)slot; World.invP1.weaponIndexPending=(i16)wi; int w=Get16WeaponIndexFromConstIndex(wi); WeaponFireStartWeaponDip((w>=0&&w<16) ? reloadTime[w] : 0.5f);}
INLINE bool CursorIsOverBounds(float x0, float x1, float y0, float y1) { return World.cursorPos_x >= x0 && World.cursorPos_x <= x1 && World.cursorPos_y >= y0 && World.cursorPos_y <= y1;/*0,0=top left*/ }
__attribute__((noinline)) bool MenuEnter() { return !Cheats.consoleActive && (Sys_Input.keyStates[KEY_KP_ENTER].pressed || Sys_Input.keyStates[KEY_ENTER].pressed); }
__attribute__((noinline)) u8 UI_MenuInteractable(i16 x, i16 y, float w, float h, bool* cursorOver, i8 this, bool sustained) {
    bool cursorIsOver = CursorIsOverBounds(x, x + w, (float)y - h, (float)y); if (cursorIsOver && mouseMovementThisFrame) { currentMenuItem = this; if (cursorOver != NULL) {*cursorOver = cursorIsOver;} } if ((sustained ? Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT ].down : Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT ].pressed) && cursorIsOver) return 1u; 
    if ((sustained ? Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down : Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed) && cursorIsOver) return 2u; return 0u;
}

__attribute__((noinline)) u8 UI_Button(i16 x, i16 y, float w, float h, bool* cursorOver, i8 this) { return UI_MenuInteractable(x,y,w,h,cursorOver,this,false); }
__attribute__((noinline)) bool AnyLeftRightMouseDown() { return (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down); }
bool UI_Slider(i16 x, i16 y, i16 w, i16 h, i16 sliderPos, i16 xPosForLabel, u8 currentValue, u8* out, bool* sliderActive, u8 min, u8 max, u8 step, u8 mindex, u16 lingdex) {
    bool over=false,changed=false; *out = currentValue; RenderUIImage(x,y, w,h, 1079);/*Slider background*/ RenderUIImage(x + sliderPos,y, h,h,1078);/*Slider handle*/ if (UI_MenuInteractable(xPosForLabel,y,xPosForLabel + w,h,&over,mindex,true)) *sliderActive = true; if(*sliderActive && World.currentMouse_dx!=0){i32 new=(i32)currentValue+vmin(vmax(World.currentMouse_dx,-1),1); *out=(u8)vmin(vmax(new,min),max); if(*out!=currentValue){changed=true;}}
    if (!AnyLeftRightMouseDown()) { if (*sliderActive) { *sliderActive = false; SaveConfig(); } } if (MenuEnter() && currentMenuItem == mindex) {bool shiftHeld = Sys_Input.keyStates[KEY_LEFT_SHIFT].down || Sys_Input.keyStates[KEY_RIGHT_SHIFT].down; if (shiftHeld)*out=*out<=((min+step)-1) ? max : *out-step; else *out=*out >=((max-step)+1) ?  min : *out+step; changed=true;} 
    over=over||currentMenuItem==mindex; RenderTextL(xPosForLabel,y,over ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,"%s %u",Sys_Text.stringTable[lingdex],*out); return changed;
}

u8 UI_MenuButton(i16 bX, i16 bY, u8 menuItem, i16 bW, i16 bH,  i16 tX, i16 tY, const char* text, i16 pX, i16 pY){bool over=false; u8 retvalue=0u; retvalue=UI_Button(bX,bY,bW,bH,&over,menuItem); if(!retvalue)retvalue=(MenuEnter()&&currentMenuItem==menuItem); over=over||currentMenuItem==menuItem; RenderTextL(tX,tY,over ? T_STOPD_RED : T_RED_MENU,FONT_STOPD,1.5f,text); RenderUIImage(pX,pY,40,40,over ? 1029 : 1028);/*Menu pad*/ return retvalue;}
bool UI_Checkbox(i16 x, i16 y, i8 mitem, u16 textIdx, bool currentlyOn){RenderUIImage(x,y,16,16,910);/*Checkbox background*/ bool over=false; bool changed=(UI_Button(x,y+16,210,16,&over,mitem)||(MenuEnter()&&currentMenuItem==mitem)); over=over||currentMenuItem==mitem; if(currentlyOn)RenderUIImage(x+2,y+2,12,12,912);/*Checkbox check*/ RenderTextL(x+20,y,over ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,Sys_Text.stringTable[textIdx]); return changed;}
__attribute__((noinline)) void UI_HeaderText(i16 x, const char* text) { RenderTextL(x,50,T_GREEN_MENU_SHADOW,FONT_STOPD,1.75f,text); RenderTextL(x,46,T_GREEN_MENU_GLOW,FONT_STOPD,1.75f,text); RenderTextL(x,48,T_GREEN_MENU,FONT_STOPD,1.75f,text); }
void PlayMenuMusic(),mp3_clear();
__attribute__((noinline)) void MenuGoBack() {if(returnToPause){returnToPause=World.menuActive=false; World.paused=true; mp3_clear();} if(currentMenuPage==Mpg_Singleplayer||currentMenuPage==Mpg_Multiplayer||currentMenuPage==Mpg_Options)currentMenuPage=Mpg_FrontPage;/*News*/else if(currentMenuPage==Mpg_Load||currentMenuPage==Mpg_NewGame||currentMenuPage==Mpg_IntroVideo||currentMenuPage==Mpg_CreditsVideo)currentMenuPage=Mpg_Singleplayer;}
static void CreateShadowBuffers() { shadowMapSSBO=MakeSSBO(&shadowMapSSBO,5,(MAX_SHADOWMAPS * (SHADOW_MAP_SIZE * SHADOW_MAP_SIZE * 6U)) * sizeof(u32),NULL,GL_STATIC_DRAW); shadowMapsIndirectionID=MakeSSBO(&shadowMapsIndirectionID,6,LIGHT_COUNT * sizeof(u32),NULL,GL_STATIC_DRAW); shadowBuffersCreated=true; }
__attribute__((noinline)) void ChangeMenuPage(u8 pg) { currentMenuPage = pg; currentMenuItem = currentMenuTab = 0; }
void RenderMenu() {    
    if (currentMenuPage != Mpg_IntroVideo && currentMenuPage != Mpg_CreditsVideo && currentMenuPage != Mpg_Options) RenderUIImage(-417,-384, 2200,1536, 1026);/*Menu background*/
    if (currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) RenderUIImage(-417,-384, 2200,1536, 0);/*Video blackground*/
    if (currentMenuPage == Mpg_Options) RenderUIImage(-417,-384, 2200,1536, 1032);/*Menu background*/
    if (currentMenuPage == Mpg_FrontPage) {
        menuItemCount = 4; menuTabCount = 1;
        RenderUIImage(282,46, 800,128, 1031);/*Title CITADEL with strikethrough effect*/
        if (UI_MenuButton(408,340, 0, 574,84, 304,188,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719],413,276)) ChangeMenuPage(Mpg_Singleplayer);
        if (UI_MenuButton(408,458, 1, 574,84, 304,268,/*"MULTIPLAYER"*/Sys_Text.stringTable[720], 413,396)) ChangeMenuPage(Mpg_Multiplayer);
        if (UI_MenuButton(408,582, 2, 574,84, 304,350,/*"OPTIONS"*/Sys_Text.stringTable[721],     413,520)) ChangeMenuPage(Mpg_Options);
        if (UI_MenuButton(408,702, 3, 574,84, 304,430,/*"QUIT"*/Sys_Text.stringTable[722],        413,638)) OS_Exit(0);
    } else if (currentMenuPage == Mpg_Singleplayer) {
        menuItemCount = 5; menuTabCount = 1;
        UI_HeaderText(250,/*"SINGLEPLAYER"*/Sys_Text.stringTable[719]);
        if (UI_MenuButton(408,340,0,574,84, 304,188,/*"CONTINUE"*/Sys_Text.stringTable[723],    413,276)) ChangeMenuPage(Mpg_Load);
        if (UI_MenuButton(408,458,1,574,84, 304,268,/*"NEW GAME"*/Sys_Text.stringTable[741],    413,396)) ChangeMenuPage(Mpg_NewGame);
        if (UI_MenuButton(408,582,2,574,84, 304,350,/*"PLAY INTRO"*/Sys_Text.stringTable[742],  413,520)) ChangeMenuPage(Mpg_IntroVideo);
        if (UI_MenuButton(408,702,3,574,84, 304,430,/*"PLAY CREDITS"*/Sys_Text.stringTable[743],413,638)) ChangeMenuPage(Mpg_CreditsVideo);
        RenderUIImage(1060,724, 84,36, 1252);/*Back Button background*/
        bool overBack = false;        
        if (UI_Button(1060,758, 84,32, &overBack, 4) || (MenuEnter() && currentMenuItem == 4)) MenuGoBack();
        overBack = overBack || currentMenuItem == 4;
        RenderTextL(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Multiplayer) {
        menuItemCount = 1; menuTabCount = 1;
        UI_HeaderText(266,/*"MULTIPLAYER"*/Sys_Text.stringTable[720]);
        RenderUIImage(1060,724, 84,36, 1252);/*Back Button background*/
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderTextL(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Options) {
        menuTabCount = 3;
        UI_HeaderText(238,/*"CONFIGURATION"*/Sys_Text.stringTable[745]);
        if (currentMenuTab != 0) RenderUIImage(179,220, 1001,548, 1030);/*Config background*/
        if (currentMenuTab == 0) RenderUIImage(179,220, 1001,548, 1033);/*Config background graphics (empty alpha center)*/
        RenderUIImage(520,196, 160,30, currentMenuTab == 2 ? 920 : 921);/*Config tab unhighlighted*/
        if (UI_Button(520,196+30, 160,30, NULL, 2)) currentMenuTab = 2;
        RenderTextL(530,202,currentMenuTab == 2 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"AUDIO / LANG"*/Sys_Text.stringTable[793]);
        RenderUIImage(354,196, 160,30, currentMenuTab == 1 ? 920 : 921);/*Config tab unhighlighted*/
        if (UI_Button(354,196+30, 160,30, NULL, 1)) currentMenuTab = 1;
        RenderTextL(366,202,currentMenuTab == 1 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"INPUT"*/Sys_Text.stringTable[792]);
        RenderUIImage(190,196, 160,30, currentMenuTab == 0 ? 920 : 921);/*Config tab highlighted*/
        if (UI_Button(190,196+30, 160,30, NULL, 0)) currentMenuTab = 0;
        RenderTextL(200,202,currentMenuTab == 0 ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"GRAPHICS"*/Sys_Text.stringTable[791]);
        if (currentMenuTab == 0) {
            bool overRes = false, overFull = false, overChgM = false;
            menuItemCount = 11;/*Graphics*/
            if (UI_Checkbox(200,500,0,Sys_Settings.ModelDetail ?/*High*/915 :/*No Detail Level Models*/914,Sys_Settings.ModelDetail)) { Sys_Settings.ModelDetail = Sys_Settings.ModelDetail ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,530,1,/*"FXAA"*/780,Sys_Settings.FXAA)) { Sys_Settings.FXAA = Sys_Settings.FXAA ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,560,2,Sys_Settings.Shadows ?/*Soft*/787 :/*No Shadows*/785,Sys_Settings.Shadows)) { Sys_Settings.Shadows = Sys_Settings.Shadows ? 0u : 1u; if (!shadowBuffersCreated) {CreateShadowBuffers();} SaveConfig(); }
            if (UI_Checkbox(200,590,3,/*SSR*/788,Sys_Settings.Reflections)) { Sys_Settings.Reflections = Sys_Settings.Reflections ? 0u : 1u; SaveConfig(); }
            if (UI_Checkbox(200,620,4,/*VSYNC*/1026,Sys_Settings.Vsync)) { Sys_Settings.Vsync = Sys_Settings.Vsync ? 0u : 1u; SetVSync(); SaveConfig(); }
            RenderTextL(310,620,T_GREEN,FONT_NORMAL,1.0f,"(FPS: %d)", globalframesPerLastSecond);/*Helper to see vsync take effect.*/
            u8 newVal;
            if (UI_Slider(400,650,128,16,(((Sys_Settings.FOV - 45.0f) / 105.0f) * (128 - 16)),200,Sys_Settings.FOV,&newVal,&fovSliderActive,45,150,5,5,/*Field of View*/775)) { Sys_Settings.FOV = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(400,680,128,16,((Sys_Settings.Brightness / 100.0f) * (128 - 16)),200,Sys_Settings.Brightness,&newVal,&gammaSliderActive,0,100,2,6,/*Gamma*/774)) { Sys_Settings.Brightness = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
/*Resolution*/
            {
/*Header hit area - UI_Button subtracts h from y internally, so pass y+h as y*/
                if (UI_Button(190,726,328,16,&overRes,7) || (MenuEnter() && currentMenuItem == 7)) { resDropdownOpen = !resDropdownOpen; currentMenuItem = 7; }
                overRes = overRes || currentMenuItem == 7;
                char resBuf[32];
                if (resDropdownCount > 0) sFormat(resBuf, sizeof(resBuf), "%ux%u",(u32)resModes[resSelectedIdx].w,(u32)resModes[resSelectedIdx].h);
                else sFormat(resBuf, sizeof(resBuf), "%ux%u",Sys_Settings.ScreenWidth,Sys_Settings.ScreenHeight);

                RenderUIImage(476, 710, 16, 16, overRes ? 1119 : 1077);
                RenderTextL(200, 710, overRes ? T_YELLOW : T_GREEN,FONT_NORMAL, 1.0f, "RESOLUTION %s", resBuf);
            }
/*Fullscreen checkbox*/
            RenderUIImage(200,740, 16,16, 910);/*Checkbox background*/
            if (UI_Button(200,756, 210,16, &overFull, 8) || (MenuEnter() && currentMenuItem == 8)) { Sys_Settings.Fullscreen = Sys_Settings.Fullscreen == 1u ? 0u : 1u; ChangeFullScreenWindowed(true); SaveConfig(); }
            overFull = overFull || currentMenuItem == 8;
            if (Sys_Settings.Fullscreen) RenderUIImage(202,742, 12,12, 912);/*Checkbox check*/
            RenderTextL(220,740,overFull ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"Fullscreen"*/Sys_Text.stringTable[773]);
            RenderUIImage(588,730, 210,30, 1079);/*Toggle monitor button background*/
            if (UI_Button(588,760, 210,30, &overChgM, 9) || (MenuEnter() && currentMenuItem == 9)) { CycleToNextMonitor(); }
            overChgM = overChgM || currentMenuItem == 9;
            RenderTextL(602,735,overChgM ? T_YELLOW : T_GREEN,FONT_NORMAL,1.0f,/*"CHANGE MONITOR"*/Sys_Text.stringTable[1025]);
        } else if (currentMenuTab == 1) { 
            menuItemCount = 49;/*Input*/
        } else {
            menuItemCount = 10;/*Audio / Lang*/
            u8 newVal;
            if (UI_Slider(426,240,128,16,((Sys_Settings.VolumeMaster / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMaster,&newVal,&masterVolumeSliderActive,0,100,5,0,/*Master Volume*/802)) { Sys_Settings.VolumeMaster = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
            if (UI_Slider(426,270,128,16,((Sys_Settings.VolumeMusic / 100.0f) * (128 - 16)),200,Sys_Settings.VolumeMusic,&newVal,&musicVolumeSliderActive,0,100,5,1,/*Music Volume*/803)) { Sys_Settings.VolumeMusic = newVal; if (!AnyLeftRightMouseDown()) {SaveConfig();} }
        }
        RenderUIImage(1087,723, 84,36, 1252);/*Back Button background*/
        i8 lastItem = menuItemCount - 1; bool overBack = false;
        if (UI_Button(1087,757, 84,32, &overBack, lastItem) || (MenuEnter() && currentMenuItem == lastItem)) MenuGoBack();
        overBack = overBack || currentMenuItem == lastItem;
        RenderTextL(1103,731,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_Load || currentMenuPage == Mpg_Save) {
        menuItemCount = 9; menuTabCount = 1; bool isSave = currentMenuPage == Mpg_Save;
        UI_HeaderText(isSave ? 284 : 340, isSave ?/*"SAVE GAME"*/Sys_Text.stringTable[769] :/*"LOAD"*/Sys_Text.stringTable[726]);
        RenderUIImage(400,214, 586,500, 1037);/*Load/Save table background*/
        RenderUIImage(1060,724, 84,36, 1252);/*Back Button background*/
        bool overBack = false;
        if (UI_Button(1060,758, 84,32, &overBack, 0) || (MenuEnter() && currentMenuItem == 0)) MenuGoBack();
        overBack = overBack || currentMenuItem == 0;
        RenderTextL(1076,732, overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU, FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_NewGame) {
        menuItemCount = 7; menuTabCount = (currentMenuItem > 0 && currentMenuItem <= 16) ? 2 : 1;
        UI_HeaderText(290,/*"NEW GAME"*/Sys_Text.stringTable[741]);
        RenderUIImage(136,196,1088,558,1048);/*Newgame inset*/
        RenderUIImage(136,196,1088,558,1049);/*Newgame background*/
        if (UI_MenuButton(276,270,0,795,74, 226,146,/*"NAME:"*/Sys_Text.stringTable[746],299,214)) {/*Just for highlight*/ }
        enteringPlayerName = (currentMenuItem == 0);
        if (World.playerName[0] == '\0') RenderTextL(642,232,T_RED_MENU,FONT_STOPD,1.0f,/*"ENTER NAME..."*/Sys_Text.stringTable[748]);
        else                                  RenderTextL(518,232,enteringPlayerName ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,World.playerName);
        if (UI_MenuButton(174,377,1,496,95, 148,202,/*"COMBAT"*/Sys_Text.stringTable[748],185,299)) { World.diffCbt = World.diffCbt >= 3 ? 0 : World.diffCbt + 1; }  if (UI_MenuButton(704,377,3,496,95, 510,202,/*"MISSION"*/Sys_Text.stringTable[749],726,299)) { World.diffMis = World.diffMis >= 3 ? 0 : World.diffMis + 1; }
        RenderTextL(162,270,World.diffCbt == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderTextL(513,270,World.diffMis == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderTextL(233,270,World.diffCbt == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderTextL(584,270,World.diffMis == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderTextL(307,270,World.diffCbt == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderTextL(658,270,World.diffMis == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderTextL(379,270,World.diffCbt == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderTextL(730,270,World.diffMis == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_MenuButton(174,568,2,496,92, 149,330,/*"PUZZLE"*/Sys_Text.stringTable[751],185,490)) { World.diffPuz = World.diffPuz >= 3 ? 0 : World.diffPuz + 1; }  if (UI_MenuButton(704,568,4,496,92, 509,330,/*"CYBERSPACE"*/Sys_Text.stringTable[750],726,490)) { World.diffCyb = World.diffCyb >= 3 ? 0 : World.diffCyb + 1; }
        RenderTextL(162,399,World.diffPuz == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");    RenderTextL(513,399,World.diffCyb == 0 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"0");
        RenderTextL(233,399,World.diffPuz == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");    RenderTextL(584,399,World.diffCyb == 1 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"1");
        RenderTextL(307,399,World.diffPuz == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");    RenderTextL(658,399,World.diffCyb == 2 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"2");
        RenderTextL(379,399,World.diffPuz == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");    RenderTextL(730,399,World.diffCyb == 3 ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,"3");
        if (UI_Button(221,460,82,79,NULL,1)) {World.diffCbt =0; currentMenuItem=1; } if (UI_Button(330,460,82,79,NULL,1)) {World.diffCbt =1; currentMenuItem=1; } if (UI_Button(439,460,82,79,NULL,1)) {World.diffCbt =2; currentMenuItem=1; } if (UI_Button( 547,460,82,79,NULL,1)) {World.diffCbt =3; currentMenuItem=1; }
        if (UI_Button(221,651,82,79,NULL,2)) {World.diffPuz =0; currentMenuItem=2; } if (UI_Button(330,651,82,79,NULL,2)) {World.diffPuz =1; currentMenuItem=2; } if (UI_Button(439,651,82,79,NULL,2)) {World.diffPuz =2; currentMenuItem=2; } if (UI_Button( 547,651,82,79,NULL,2)) {World.diffPuz =3; currentMenuItem=2; }
        if (UI_Button(748,460,82,79,NULL,3)) {World.diffMis=0; currentMenuItem=3; } if (UI_Button(857,460,82,79,NULL,3)) {World.diffMis=1; currentMenuItem=3; } if (UI_Button(966,460,82,79,NULL,3)) {World.diffMis=2; currentMenuItem=3; } if (UI_Button(1074,460,82,79,NULL,3)) {World.diffMis=3; currentMenuItem=3; }
        if (UI_Button(748,651,82,79,NULL,4)) {World.diffCyb  =0; currentMenuItem=4; } if (UI_Button(857,651,82,79,NULL,4)) {World.diffCyb  =1; currentMenuItem=4; } if (UI_Button(966,651,82,79,NULL,4)) {World.diffCyb  =2; currentMenuItem=4; } if (UI_Button(1074,651,82,79,NULL,4)) {World.diffCyb  =3; currentMenuItem=4; }
        bool overBack = false, overStart = false;
        if (UI_Button(544,747, 282,68, &overStart, 5) || (MenuEnter() && currentMenuItem == 5)) GoIntoGame();
        overStart = overStart || currentMenuItem == 5;
        RenderTextL(400,464,overStart ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.5f,/*"START"*/Sys_Text.stringTable[886]);
        if (UI_Button(1060,758, 84,32, &overBack, 6) || (MenuEnter() && currentMenuItem == 6)) MenuGoBack();
        overBack = overBack || currentMenuItem == 6;
        RenderUIImage(1060,724,84,36,1252);/*Back Button background*/
        RenderTextL(1076,732,overBack ? T_STOPD_RED_HIGHLIGHT : T_RED_MENU,FONT_NORMAL,1.0f,/*"BACK"*/Sys_Text.stringTable[744]);
    } else if (currentMenuPage == Mpg_IntroVideo || currentMenuPage == Mpg_CreditsVideo) {
        menuItemCount = menuTabCount = 1;
        if (MenuEnter()) MenuGoBack();
    }
    if (menuTabCount <= currentMenuTab) currentMenuTab = 0;
    if (menuItemCount <= currentMenuItem) currentMenuItem = 0;
    static const i8 ngSwap[7] = {0,3,4,1,2,6,5};
    if (Sys_Input.keyStates[KEY_RIGHT].pressed || Sys_Input.keyStates[KEY_LEFT].pressed) { int dir = Sys_Input.keyStates[KEY_RIGHT].pressed ? 1 : -1; currentMenuTab = (currentMenuTab + menuTabCount + dir) % menuTabCount; if (currentMenuPage == Mpg_NewGame && currentMenuItem < 7) {currentMenuItem=ngSwap[currentMenuItem];} }
}

void RenderPausedUI() {
    menuItemCount = 6; menuTabCount = 1;
    bool overResume = false, overLoad/*;)*/ = false, overSave = false, overOptions = false, overQuitMenu = false, overQuit = false;
    RenderUIImage(519,276,328,300,1025);/*Pause Menu background*/
    RenderUIImage(519,276,328,300,1080);/*Pause Menu background outline*/
    RenderTextL(610,210,T_STOPD_RED_PAUSETITLE,FONT_STOPD,1.0f,/*"PAUSED"*/Sys_Text.stringTable[724]);
    if (UI_Button(522,330, 322,52, &overResume, 0) || (MenuEnter() && currentMenuItem == 0)) World.paused = false;
    overResume = overResume || currentMenuItem == 0;
    RenderTextL(610,306,overResume ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"RESUME"*/Sys_Text.stringTable[725]);
    if (UI_Button(522,390, 322,52, &overLoad, 1) || (MenuEnter() && currentMenuItem == 1)) { currentMenuPage = Mpg_Load; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overLoad = overLoad || currentMenuItem == 1;
    RenderTextL(630,364, overLoad ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"LOAD"*/Sys_Text.stringTable[726]);
    if (UI_Button(522,450, 322,60, &overSave, 2) || (MenuEnter() && currentMenuItem == 2)) { currentMenuPage = Mpg_Save; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overSave = overSave || currentMenuItem == 2;
    RenderTextL(635,422,overSave ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"SAVE"*/Sys_Text.stringTable[727]);
    if (UI_Button(522,510, 322,60, &overOptions, 3) || (MenuEnter() && currentMenuItem == 3)) { currentMenuPage = Mpg_Options; PlayMenuMusic(); World.menuActive = true; returnToPause = true; }
    overOptions = overOptions || currentMenuItem == 3;
    RenderTextL(599,480,overOptions ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"OPTIONS"*/Sys_Text.stringTable[721]);
    if (UI_Button(522,570, 322,60, &overQuitMenu, 4) || (MenuEnter() && currentMenuItem == 4)) { PlayMenuMusic(); World.menuActive = true; currentMenuPage = Mpg_FrontPage; }
    overQuitMenu = overQuitMenu || currentMenuItem == 4;
    RenderTextL(546,538,overQuitMenu ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT TO MENU"*/Sys_Text.stringTable[728]);
    RenderUIImage(519,672,328,42,1252);/*Pause Quit Game background*/
    if (UI_Button(522,714, 322,42, &overQuit, 5) || (MenuEnter() && currentMenuItem == 5)) OS_Exit(0);
    overQuit = overQuit || currentMenuItem == 5;
    RenderTextL(572,690,overQuit ? T_STOPD_RED_HIGHLIGHT : T_STOPD_RED,FONT_STOPD,1.0f,/*"QUIT GAME"*/Sys_Text.stringTable[729]);
}

void GetWeaponAmmoText(int slot,char* buf,size_t bufSize) {
    buf[0] = '\0'; int wepIdx = World.invP1.weaponInventoryIndices[slot]; bool alt = World.invP1.wepLoadedWithAlternate[slot]; float heat = World.invP1.currentEnergyWeaponHeat[slot];
    u8 mag = alt ? World.invP1.currentMagazineAmount2[slot] : World.invP1.currentMagazineAmount[slot];
    switch(wepIdx) {
        case 343: if (alt){sFormat(buf,bufSize,"%upn | %umg, %upn",mag,World.invP1.wepAmmo[0],World.invP1.wepAmmoSecondary[0]);}else{sFormat(buf,bufSize,"%umg | %umg, %upn",mag,World.invP1.wepAmmo[0],World.invP1.wepAmmoSecondary[0]);} break;/*MK3 Assault Rifle*/
        case 344: case 347: case 353: case 357: case 358: scpy_to_a_from_b(buf,heat > 80.0f ? Sys_Text.stringTable[14] : Sys_Text.stringTable[15],bufSize); break;/*Energy weapons*/
        case 345: if (alt){sFormat(buf,bufSize,"%utq | %und, %utq",mag,World.invP1.wepAmmo[2],World.invP1.wepAmmoSecondary[2]);}else{sFormat(buf,bufSize,"%und | %und, %utq",mag,World.invP1.wepAmmo[2],World.invP1.wepAmmoSecondary[2]);} break;/*SV-23 Dartgun*/
        case 346: if (alt){sFormat(buf,bufSize,"%usp | %uhn, %usp",mag,World.invP1.wepAmmo[3],World.invP1.wepAmmoSecondary[3]);}else{sFormat(buf,bufSize,"%uhn | %uhn, %usp",mag,World.invP1.wepAmmo[3],World.invP1.wepAmmoSecondary[3]);} break;/*AM-27 Flechette*/
        case 348: case 349: break;/*Laser Rapier / Lead Pipe: no ammo*/
        case 350: if (alt){sFormat(buf,bufSize,"%usg | %uhw, %usg",mag,World.invP1.wepAmmo[7],World.invP1.wepAmmoSecondary[7]);}else{sFormat(buf,bufSize,"%uhw | %uhw, %usg",mag,World.invP1.wepAmmo[7],World.invP1.wepAmmoSecondary[7]);} break;/*Magnum 2100*/
        case 351: if (alt){sFormat(buf,bufSize,"%usu | %ucr, %usu",mag,World.invP1.wepAmmo[8],World.invP1.wepAmmoSecondary[8]);}else{sFormat(buf,bufSize,"%ucr | %ucr, %usu",mag,World.invP1.wepAmmo[8],World.invP1.wepAmmoSecondary[8]);} break;/*SB-20 Magpulse*/
        case 352: if (alt){sFormat(buf,bufSize,"%utf | %ust, %utf",mag,World.invP1.wepAmmo[9],World.invP1.wepAmmoSecondary[9]);}else{sFormat(buf,bufSize,"%ust | %ust, %utf",mag,World.invP1.wepAmmo[9],World.invP1.wepAmmoSecondary[9]);} break;/*ML-41 Pistol*/
        case 354: sFormat(buf,bufSize,"%url | %url",World.invP1.currentMagazineAmount[slot],World.invP1.wepAmmo[11]); break;/*MM-76 Railgun*/
        case 355: sFormat(buf,bufSize,"%urb | %urb",World.invP1.currentMagazineAmount[slot],World.invP1.wepAmmo[12]); break;/*DC-05 Riotgun*/
        case 356: if (alt){sFormat(buf,bufSize,"%ulg | %usm, %ulg",mag,World.invP1.wepAmmo[13],World.invP1.wepAmmoSecondary[13]);}else{sFormat(buf,bufSize,"%usm | %usm, %ulg",mag,World.invP1.wepAmmo[13],World.invP1.wepAmmoSecondary[13]);} break;/*RF-07 Skorpion*/
        default: break;
    }
}

void TickBar(bool isEnergy) {
    RenderUIImage(isEnergy ? 1333 : 1332,isEnergy ? 36 : 2,32,32,isEnergy ? 939 : 956);/*Indicator*/ int p1H=isEnergy ? World.invP1.energy : World.instances[PLAYER1].health; if(p1H>255){p1H=255;} i16 tY=isEnergy ? 35 : 4;
    for (int i=7;i>=0;--i) if(i==7/*Always render at least 1 tick*/||p1H>(7-i)*11){RenderUIImage(1050-(i*16),tY,32,32,964);/*Tick Red*/} for (int i=7;i>=0;--i) if(p1H>88+(7-i)*11){RenderUIImage(1178-(i*16),tY,32,32,963);/*Tick Orange*/} for (int i=7;i>=0;--i) if(p1H>176+(7-i)*11){RenderUIImage(1306-(i*16),tY,32,32,962);/*Tick Green*/}
}

void BioMonitorClearGraphs(void);
INLINE bool HwBtnClick(float x0, float x1, float y0, float y1) {if (!World.inventoryMode || !CursorIsOverBounds(x0,x1,y0,y1)){return false;} if (!(Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed)){return false;} World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true; Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed=false; return true;}
INLINE int HwActiveTexIndex(int active, int version, int off, int v1, int v2, int v3, int v4) { if (!active) return off; if (v4 >= 0 && version >= 4) return v4; if (version >= 3) return v3; if (version == 2) return v2; return v1; }
void HardwareButtons() {
    if(Cheats.noHUD){return;} bool noEng=World.invP1.energy<=0.0f, hasBio=World.invP1.hasHardware & HW_BIO, bioOn=World.invP1.hardwareIsActive & HW_BIO, snsOn=World.invP1.hardwareIsActive & HW_SNS, lanOn=World.invP1.hardwareIsActive & HW_LAN, shdOn=World.invP1.hardwareIsActive & HW_SHD, infOn=World.invP1.hardwareIsActive & HW_INF;
    if (hasBio) { RenderUIImage(0,180,40,40,HwActiveTexIndex(bioOn,World.invP1.hwVers[HW_BIO_IDX],989,991,992,992,992)); if (HwBtnClick(0,40,180,220)) { if (World.invP1.hwVersSetting[HW_BIO_IDX] == 0 && noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (hasBio && bioOn) { World.invP1.hardwareIsActive &= ~HW_BIO; if (!Cheats.showFPS) BioMonitorClearGraphs(); } else World.invP1.hardwareIsActive |= HW_BIO; } } }
    if (World.invP1.hasHardware & HW_SNS) { RenderUIImage(0,240,40,40,HwActiveTexIndex(snsOn,World.invP1.hwVers[HW_SNS_IDX],1009,1011,1012,1013,1013)); if (HwBtnClick(0,40,240,280)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else if (snsOn) { play_wav(sounds[82],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive &= ~HW_SNS; } else { play_wav(sounds[93],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive |= HW_SNS; } } }
    if (World.invP1.hasHardware & HW_LAN) { RenderUIImage(0,300,40,40,HwActiveTexIndex(lanOn,World.invP1.hwVers[HW_LAN_IDX],1004,1006,1007,1008,1008)); if (HwBtnClick(0,40,300,340)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (lanOn) World.invP1.hardwareIsActive &= ~HW_LAN; else World.invP1.hardwareIsActive |= HW_LAN; } } }
    if (World.invP1.hasHardware & HW_SHD) { RenderUIImage(0,360,40,40,HwActiveTexIndex(shdOn,World.invP1.hwVers[HW_SHD_IDX],1014,1015,1016,1017,1018)); if (HwBtnClick(0,40,360,400)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else if (shdOn) { play_wav(sounds[95],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive &= ~HW_SHD; } else { play_wav(sounds[96],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive |= HW_SHD; } } }
    if (World.invP1.hasHardware & HW_INF) { RenderUIImage(1326,180,40,40,HwActiveTexIndex(infOn,World.invP1.hwVers[HW_INF_IDX],998,999,999,999,999)); if (HwBtnClick(1326,1366,180,220)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { bool wasOn=(infOn) != 0; play_wav(wasOn ? sounds[82] : sounds[98],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (wasOn) World.invP1.hardwareIsActive &= ~HW_INF; else World.invP1.hardwareIsActive |= HW_INF; } } }
    if (World.invP1.hasHardware & HW_ERD) { RenderUIImage(1326,240,40,40,World.inventoryMode && CursorIsOverBounds(1326,1366,240,280) && (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].down) ? 997 : ((World.invP1.hasNewEmail || World.invP1.hasNewLogs) && ((int)World.pauseRelativeTime & 1)) ? 997 : 996); }
    if (World.invP1.hasHardware & HW_BST) { RenderUIImage(1326,300,40,40,HwActiveTexIndex(World.invP1.hardwareIsActive & HW_BST,World.invP1.hwVers[HW_BST_IDX],993,994,995,995,995)); if (HwBtnClick(1326,1366,300,340)) { if (World.invP1.hwVersSetting[HW_BST_IDX] >= 1 && noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); if (World.invP1.hardwareIsActive & HW_BST) World.invP1.hardwareIsActive &= ~HW_BST; else World.invP1.hardwareIsActive |= HW_BST; } } }
    if (World.invP1.hasHardware & HW_JET) { RenderUIImage(1326,360,40,40,HwActiveTexIndex(World.invP1.hardwareIsActive & HW_JET,World.invP1.hwVers[HW_JET_IDX],1000,1001,1002,1003,1003)); if (HwBtnClick(1326,1366,360,400)) { if (noEng) CenterStatusPrint("%s",Sys_Text.stringTable[314]); else { play_wav(sounds[78],SfxVol(),(V3){0.0f,0.0f,0.0f},false); World.invP1.hardwareIsActive ^= HW_JET; } } }
}

void AddItemToInventory(int index, int custIdx); void ResetHeldItem();
void CenterMFDHeader() {
    RenderUIImage(400,752,64,32,World.Sys_UI.mfdSelected[0] == 1 && World.Sys_UI.MFD_CenterTab!=5 ? 1024 : 1021);/*Main center tab button*/ RenderUIImage(480,752,64,32,World.Sys_UI.mfdSelected[0] == 2 && World.Sys_UI.MFD_CenterTab!=5 ? 1024 : 1021);/*Hardware center tab button*/ RenderUIImage(560,752,64,32,World.Sys_UI.mfdSelected[0] == 3 && World.Sys_UI.MFD_CenterTab!=5 ? 1024 : 1021);/*General center tab button*/ RenderUIImage(902,752,64,32,World.Sys_UI.mfdSelected[0] == 4 && World.Sys_UI.MFD_CenterTab!=5 ? 1024 : 1021);/*Software center tab button*/
    if (World.inventoryMode && World.invP1.holdingObject && CursorIsOverBounds(345,1021,460,768)) {/*Add to Inventory Helper*/
        World.uiIsBlocking = true; RenderUIImage(345,528,676,240,1075); RenderTextL(586,528,T_GREEN,FONT_NORMAL,1.0f,"ADD TO INVENTORY");
        if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed) { AddItemToInventory(World.invP1.heldObjectIndex,World.invP1.heldObjectCustIdx); ResetHeldItem(); Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed = Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed = false; }
    }
/*if (World.Sys_UI.showSensaroundCenter) { SensaroundCenter Plane } TODO*/
    if(World.Sys_UI.MFD_CenterTab==0) return;/*Tabs are off.*/
    if(World.Sys_UI.MFD_CenterTab==1 && !Cheats.noHUD){/*MainTab: WeaponInventory,WeaponShotsInventory,GrenadeInventory,PatchInventory*/
        RenderTextL(372,560,T_RED,FONT_NORMAL,0.8f,"WEAPONS"); RenderTextL(574,560,T_RED,FONT_NORMAL,0.8f,"SHOTS");/*Column headers*/
        for(int slot=0;slot<7;++slot){
            int widx=World.invP1.weaponInventoryIndices[slot]; if(widx<0)continue;
            int y=582+slot*22;
            bool hov = CursorIsOverBounds(372,712,(float)y-5,(float)y+16);/*Slight shift of 6 feels better than just doing y and y + 22 as one would expect, then lopped 1 off one end to prevent double highlighting*/
            u32 col = (hov&&World.inventoryMode && World.invP1.weaponCurrent!=slot) ? T_GREEN_MENU : (World.invP1.weaponCurrent==slot?T_YELLOW:(World.invP1.weaponCurrentPending==slot?T_DARK_YELLOW:T_GREEN));
            RenderTextL(372,y,col,FONT_NORMAL,0.8f,"%s",Sys_Text.stringTable[ItemStringIdx((i32)widx)]);/*Weapon text*/
            char b[64]; GetWeaponAmmoText(slot,b,sizeof(b)); RenderTextL(574,y,col,FONT_NORMAL,0.8f,"%s",b);/*Ammo text*/
            if(hov&&World.inventoryMode&&Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed){WeaponSelectSlot(slot);Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=false; World.uiIsBlocking=true;}
        }
    }
}

void SideMFDHeader(bool isRH) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex), tab = isRH ? World.Sys_UI.MFD_RightTab : World.Sys_UI.MFD_LefTab;
    u8 selected=tab?tab:World.Sys_UI.mfdSelected[isRH?2:1];
    RenderUIImage(isRH ? 1350 : -16,520,32,40,selected == 1 ? 1024 : 1022);/*Weapon side tab button*/

    RenderUIImage(isRH ? 1350 : -16,576,32,40,selected == 2 ? 1024 : 1022);/*Item side tab button*/

    RenderUIImage(isRH ? 1350 : -16,632,32,40,selected == 3 ? 1024 : 1022);/*Automap side tab button*/

    RenderUIImage(isRH ? 1350 : -16,688,32,40,selected == 4 ? 1024 : 1022);/*Data side tab button*/

    if ((World.invP1.hardwareIsActive & HW_SNS) && World.invP1.hwVers[HW_SNS_IDX] > 1) {/*TODO Sensaround Plane*/ }
     if (tab == 0){return;} 
/*RenderUIImage(isRH ? 1022 : 24,520,320,240,1025); // TODO REMOVE Test BG for ensuring fit into 320x240 to match 1:1 scale that Doom's 320x200 would map to after 4:3 scaling applied (since the CRT's had non-square pixels that stretched 320x200 into 320x240 space, ish) TODO gate by search active*/
    if (tab == 1) {/*WeaponTabLH: WepNameTextLH, WepIconLH, ClipBox, EnergyHeatTicks, ReloadButtons, EnergySlider*/
        i16 slot=World.invP1.weaponCurrent; if (slot<0 || slot>=7) return;
        i32 widx=World.invP1.weaponInventoryIndices[slot];
        if (widx >= 0) { RenderTextL(isRH ? 1342 : 24,520,T_RED,FONT_NORMAL,0.8f,"%s",Sys_Text.stringTable[ItemStringIdx((i32)widx)]);/*Weapon Name*/ if (wep16 >=0 && wep16 < 16){RenderUIImage(isRH ? 1207 : 24,548,270,100,wepIconTexIndices[wep16]);/*WepIconLH*/} }
    } else if (tab == 2 && World.Sys_UI.mfdItemReader[isRH]) {
        i16 x=isRH?1080:22; static const u16 labels[4]={42,39,43,885};
        RenderTextL(x+6,540,T_YELLOW,FONT_NORMAL,0.6,"%s",Sys_Text.stringTable[349]);
        for (u8 section=0;section<4;++section) { if (section==MM_NOTES && !World.diffMis) continue;
            bool sectionSelected=World.Sys_UI.MFD_MediaTab==section,unread=World.Sys_UI.highlightStatus[section];
            RenderUIImage(x+65*section,718,65,40,sectionSelected||unread?1087:1086);
            RenderTextL(x+65*section,718,sectionSelected?T_GREEN_MENU:T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"%s",Sys_Text.stringTable[labels[section]]);
        }
    }
    else if (tab == 3) {/*AutomapTab: AutomapMask, Overlays, PlayerIcon, ZoomIn/Out/Full/Side Buttons*/ }
}

static const u16 vmailStartFrames[6]={1579,1645,1713,1784,1864,1931}; static const u16 vmailEndFrames[6]={1644,1712,1783,1863,1930,1988}; double avgCPUt[AVG_CPU_TAPS]={0}; int avgCPUt_idx = 0;
void AppendTextWarning(i32 sidx, i32 sidx2, i32 sidx3, i32 col, i32 id) { World.Sys_UI.tWrnTextIdx[id]=sidx; World.Sys_UI.tWrnTextIdx2[id]=sidx2; World.Sys_UI.tWrnTextIdx3[id]=sidx3; World.Sys_UI.tWrnFinished[id]=World.Sys_UI.tWrnFinished[id] < World.pauseRelativeTime ? World.pauseRelativeTime + 0.1f : World.Sys_UI.tWrnFinished[id] + 0.1f; World.Sys_UI.tWrnColorIdx[id] = col; }
extern double game_actual_start_time; extern u16 editModeTestEntityDefinition;
/*Edit-mode info panel text editing (console-style entry)*/
enum { EF_POSX,EF_POSY,EF_POSZ,EF_ROTX,EF_ROTY,EF_ROTZ,EF_ROTW,EF_SCLX,EF_SCLY,EF_SCLZ,EF_TEX,EF_MODEL,EF_GLOW,EF_SPEC,EF_NORM,EF_LAST };
bool editFieldEditing=false; static u8 editFieldSlot=EF_LAST; static char editFieldBuffer[40]={0};
#define EF_LABELX 982
#define EF_VALUEX 1088
static const i16 efRowY[EF_LAST]={160,188,216,244,272,300,328,356,384,412,440,468,496,524,552};
static const char* efRowLabel[EF_LAST]={"position x","position y","position z","rotation x","rotation y","rotation z","rotation w","scale x","scale y","scale z","texIndex","modelIndex","glowIndex","specIndex","normIndex"};
extern Quaternion quat_normalize(Quaternion);
static void EditFieldValueText(u8 slot,u16 sel,char* out,size_t n){switch(slot){
    case EF_POSX:sFormat(out,n,"%.2f",World.position[sel].x);break; case EF_POSY:sFormat(out,n,"%.2f",World.position[sel].y);break; case EF_POSZ:sFormat(out,n,"%.2f",World.position[sel].z);break;
    case EF_ROTX:sFormat(out,n,"%.3f",World.rotation[sel].x);break; case EF_ROTY:sFormat(out,n,"%.3f",World.rotation[sel].y);break; case EF_ROTZ:sFormat(out,n,"%.3f",World.rotation[sel].z);break; case EF_ROTW:sFormat(out,n,"%.3f",World.rotation[sel].w);break;
    case EF_SCLX:sFormat(out,n,"%.2f",World.scale[sel].x);break; case EF_SCLY:sFormat(out,n,"%.2f",World.scale[sel].y);break; case EF_SCLZ:sFormat(out,n,"%.2f",World.scale[sel].z);break;
    case EF_TEX:sFormat(out,n,"%u",World.instances[sel].texIndex);break; case EF_MODEL:sFormat(out,n,"%u",World.instances[sel].modelIndex);break; case EF_GLOW:sFormat(out,n,"%u",World.instances[sel].glowIndex);break; case EF_SPEC:sFormat(out,n,"%u",World.instances[sel].specIndex);break; case EF_NORM:sFormat(out,n,"%u",World.instances[sel].normIndex);break;
    default:sFormat(out,n,"");break;}}
static bool EditSelIsActive(void){return Cheats.editMode&&editModeSelection<U16_MAX&&editModeSelection>=INSTS_1ST_IDX&&editModeSelection<World.instCount&&(World.instances[editModeSelection].entflags&EF_ACTIVE);}
bool EditPanelPointerHover(void){if(!EditSelIsActive()||!World.inventoryMode)return false;u16 sel=editModeSelection;char v[40];for(int i=0;i<EF_LAST;++i){EditFieldValueText((u8)i,sel,v,40);float w=MeasureLineAdvance(v,FONT_NORMAL);if(World.cursorPos_x>=EF_VALUEX&&World.cursorPos_x<=EF_VALUEX+w&&World.cursorPos_y>=efRowY[i]&&World.cursorPos_y<=efRowY[i]+26)return true;}return false;}
static void EditStop(void){editFieldEditing=false;editFieldSlot=EF_LAST;}
static void EditFieldWrite(u16 s,Entity* e,float f,i32 iv){switch(editFieldSlot){
    case EF_POSX:World.position[s].x=f;break; case EF_POSY:World.position[s].y=f;break; case EF_POSZ:World.position[s].z=f;break;
    case EF_ROTX:World.rotation[s].x=f;World.rotation[s]=quat_normalize(World.rotation[s]);break; case EF_ROTY:World.rotation[s].y=f;World.rotation[s]=quat_normalize(World.rotation[s]);break; case EF_ROTZ:World.rotation[s].z=f;World.rotation[s]=quat_normalize(World.rotation[s]);break; case EF_ROTW:World.rotation[s].w=f;World.rotation[s]=quat_normalize(World.rotation[s]);break;
    case EF_SCLX:World.scale[s].x=f;break; case EF_SCLY:World.scale[s].y=f;break; case EF_SCLZ:World.scale[s].z=f;break;
    case EF_TEX:e->texIndex=(u16)vclamp(iv,0,MAX_TXRS-1);break; case EF_MODEL:e->modelIndex=(u16)vclamp(iv,0,MAX_MDLS-1);break; case EF_GLOW:e->glowIndex=(u16)vclamp(iv,0,MAX_TXRS-1);break; case EF_SPEC:e->specIndex=(u16)vclamp(iv,0,MAX_TXRS-1);break; case EF_NORM:e->normIndex=(u16)vclamp(iv,0,MAX_TXRS-1);break;}
}

static void EditFieldCommitLive(void){if(!EditSelIsActive())return;u16 s=editModeSelection;Entity* e=&World.instances[s];float f=0;{const char* p=editFieldBuffer;f=fast_atof(&p);}EditFieldWrite(s,e,f,s2i32(editFieldBuffer));}
static void EditFieldStep(float delta){if(!editFieldEditing||!EditSelIsActive())return;float cur;{const char* p=editFieldBuffer;cur=fast_atof(&p);}
    if(editFieldSlot<EF_TEX){int steps=delta>0?(int)(delta+0.5f):-(int)((-delta)+0.5f);cur+=steps*0.01f;int prec=(editFieldSlot>=EF_ROTX&&editFieldSlot<=EF_ROTW)?3:2;char fmt[8];sFormat(fmt,8,"%%.%df",prec);sFormat(editFieldBuffer,40,fmt,cur);}
    else{int steps=delta>0?(int)(delta+0.5f):-(int)((-delta)+0.5f);i32 iv=(i32)cur+steps;iv=vclamp(iv,0,(editFieldSlot==EF_MODEL?MAX_MDLS-1:MAX_TXRS-1));sFormat(editFieldBuffer,40,"%d",iv);}
    EditFieldCommitLive();
}

void EditFieldKey(i32 keycode){if(!editFieldEditing)return;
    if(keycode==KEY_ESCAPE){EditStop();return;} if(keycode==KEY_ENTER||keycode==KEY_KP_ENTER){EditStop();return;}
    size_t len=slen(editFieldBuffer); bool changed=false;
    if(keycode>=KEY_0&&keycode<=KEY_9){if(len<39){editFieldBuffer[len]=(char)('0'+(keycode-KEY_0));editFieldBuffer[len+1]='\0';changed=true;}}
    else if(keycode>=KEY_KP_0&&keycode<=KEY_KP_9){if(len<39){editFieldBuffer[len]=(char)('0'+(keycode-KEY_KP_0));editFieldBuffer[len+1]='\0';changed=true;}}
    else if((keycode==KEY_MINUS||keycode==KEY_KP_SUBTRACT)&&len==0){if(len<39){editFieldBuffer[len]='-';editFieldBuffer[len+1]='\0';changed=true;}}
    else if(keycode==KEY_PERIOD||keycode==KEY_KP_DECIMAL){bool hasdot=false;for(size_t k=0;k<len&&!hasdot;++k)if(editFieldBuffer[k]=='.')hasdot=true;if(!hasdot&&len<39){editFieldBuffer[len]='.';editFieldBuffer[len+1]='\0';changed=true;}}
    else if(keycode==KEY_BACKSPACE){if(len>0){editFieldBuffer[len-1]='\0';changed=true;}}
    if(changed)EditFieldCommitLive();
}


bool UI_SoftwareInventory(void) { return false; }
bool InventoryPointerHover(void);
bool UI_HardwareInventory(void);
bool UIInteractions(void) {/*Loop UIRegions: over+active sets lmb/rmb on click, lastLMB/lastRMB=World.currentTime for dblclick*/
    if (!World.inventoryMode) return false;
    if (World.menuActive || World.paused || World.creditsActive || Cheats.consoleActive) return true;
    if (EditPanelPointerHover()) return true;
    if (Cheats.noHUD || World.Sys_UI.vmailActive) return false;
    if (CursorIsOverBounds(667,699,0,32)) return true;
    u32 hw=World.invP1.hasHardware;
    if (((hw&HW_BIO)&&CursorIsOverBounds(0,40,180,220)) || ((hw&HW_SNS)&&CursorIsOverBounds(0,40,240,280)) || ((hw&HW_LAN)&&CursorIsOverBounds(0,40,300,340)) || ((hw&HW_SHD)&&CursorIsOverBounds(0,40,360,400))) return true;
    if (((hw&HW_INF)&&CursorIsOverBounds(1326,1366,180,220)) || ((hw&HW_ERD)&&CursorIsOverBounds(1326,1366,240,280)) || ((hw&HW_BST)&&CursorIsOverBounds(1326,1366,300,340)) || ((hw&HW_JET)&&CursorIsOverBounds(1326,1366,360,400))) return true;
    if (CursorIsOverBounds(400,464,752,784) || CursorIsOverBounds(480,544,752,784) || CursorIsOverBounds(560,624,752,784) || CursorIsOverBounds(902,966,752,784)) return true;
    for (int side=0;side<2;++side) for (int tab=0;tab<4;++tab) if (CursorIsOverBounds(side?1350:-16,side?1382:16,520+56*tab,560+56*tab)) return true;
    if (World.invP1.holdingObject && CursorIsOverBounds(345,1021,460,768)) return true;
    if (InventoryPointerHover()) return true;
    return (World.Sys_UI.MFD_CenterTab && World.Sys_UI.MFD_CenterTab!=1 && World.Sys_UI.MFD_CenterTab!=3 && CursorIsOverBounds(345,1021,552,768)) || (World.Sys_UI.MFD_LefTab && World.Sys_UI.MFD_LefTab!=2 && CursorIsOverBounds(24,344,520,768)) || (World.Sys_UI.MFD_RightTab && World.Sys_UI.MFD_RightTab!=2 && CursorIsOverBounds(1022,1342,520,768));
}

int GeneralInvItem(int slot);
bool GeneralInvCanUse(int slot),GeneralInvCanVaporize(int slot),GeneralInvTake(int slot);
void GeneralInvClick(int slot,int custom),GeneralInvApply(int slot,int custom),VaporizeClick(void);
bool InventoryHasAccessCard(AccCardType card);
const char* AccessCardCodeForType(AccCardType card);
u16 GetItemFrobTexture(u16 index);
static const i16 generalRowY[7]={573,597,620,644,667,691,714};
static const char* GeneralInvLabel(int slot) { int item=GeneralInvItem(slot); return item<0?"":Sys_Text.stringTable[slot?item+326:597]; }
static float GeneralInvTextWidth(int slot) { return vmin(185.0f,MeasureLineAdvance(GeneralInvLabel(slot),FONT_NORMAL)*0.6f); }
static bool GeneralInvTextHover(int slot) {
    if (slot<0 || GeneralInvItem(slot)<0) return false;
    i16 x=slot<7?454:681,y=generalRowY[slot%7]; return CursorIsOverBounds(x,x+GeneralInvTextWidth(slot),y,y+20);
}
static const u8 grenadeItems[7]={7,9,13,8,11,12,10},patchSlots[7]={6,5,0,3,4,2,1};
static const i16 consumableRowY[7]={604,624,644,664,685,705,725};
void UseGrenade(int),PatchUse(int);
int ConsumableSlot(bool patch,int row) { return row<0 || row>=7?-1:patch?patchSlots[row]:row; }
int ConsumableItem(bool patch,int row) { int slot=ConsumableSlot(patch,row); return slot<0?-1:patch?14+slot:grenadeItems[row]; }
static u8 ConsumableCount(bool patch,int row) { int slot=ConsumableSlot(patch,row); return slot<0?0:patch?World.invP1.patchCounts[slot]:World.invP1.grenAmmo[slot]; }
static bool TextPointerHover(i16 x,i16 y,const char* text,float scale,float maxWidth) { return CursorIsOverBounds(x,x+vmin(maxWidth,MeasureLineAdvance(text,FONT_NORMAL)*scale),y,y+20); }
static bool ConsumableRowHover(bool patch,int row) {
    if (!ConsumableCount(patch,row)) return false;
    i16 x=patch?831:734,y=consumableRowY[row]; char count[4]; sFormat(count,sizeof(count),"%u",ConsumableCount(patch,row));
    return TextPointerHover(x,y,Sys_Text.stringTable[(patch?907:900)+row],0.6f,patch?62:59) || TextPointerHover(patch?895:706,y,count,0.6f,24) || CursorIsOverBounds(patch?920:795,patch?940:815,y+1,y+21);
}
void ConsumableSelect(bool patch,int row) {
    if (!ConsumableCount(patch,row)) return;
    if (patch) World.invP1.patchCur=(u8)ConsumableSlot(true,row); else World.invP1.grenCur=(u8)row;
    MFD_ShowGeneralItem(); World.Sys_UI.mfdGeneralItem=false; World.Sys_UI.mfdConsumable=patch?2:1;
}
void ConsumableUse(bool patch,int row) {
    if (!ConsumableCount(patch,row) || World.invP1.holdingObject) return;
    ConsumableSelect(patch,row); if (patch) PatchUse(ConsumableSlot(true,row)); else UseGrenade(ConsumableItem(false,row)+307);
    World.Sys_UI.consumableClickRow=-1;
}
static int ConsumableSelectedRow(void) {
    if (World.Sys_UI.mfdConsumable==1) return World.invP1.grenCur<7?World.invP1.grenCur:-1;
    if (World.Sys_UI.mfdConsumable==2) for (int row=0;row<7;++row) if (patchSlots[row]==World.invP1.patchCur) return row;
    return -1;
}
void ConsumableSetTimer(float fraction) {
    int row=ConsumableSelectedRow(); if (World.Sys_UI.mfdConsumable!=1 || row<5 || !ConsumableCount(false,row)) return;
    float min=row==5?2.0f:4.0f,value=min+vclamp(fraction,0.0f,1.0f)*(60.0f-min);
    if (row==5) World.invP1.nitroTimeSetting=value; else World.invP1.earthShakerTimeSetting=value;
}
static bool UI_Consumables(void) {
    bool left=Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed;
    if (World.Sys_UI.MFD_CenterTab==1 && !World.invP1.holdingObject) for (int kind=0;kind<2;++kind) for (int row=0;row<7;++row) {
        if (!ConsumableRowHover(kind,row)) continue;
        int id=kind*7+row; bool twice=World.Sys_UI.consumableClickRow==id && World.pauseRelativeTime-World.Sys_UI.consumableClickTime<=0.5;
        if (!left) return false;
        i16 y=consumableRowY[row]; bool use=CursorIsOverBounds(kind?920:795,kind?940:815,y+1,y+21);
        Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=false; World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true;
        ConsumableSelect(kind,row); if (use || twice) ConsumableUse(kind,row);
        else { World.Sys_UI.consumableClickRow=(i8)id; World.Sys_UI.consumableClickTime=World.pauseRelativeTime; }
        return true;
    }
    World.Sys_UI.consumableClickRow=-1;
    int row=ConsumableSelectedRow(); bool patch=World.Sys_UI.mfdConsumable==2;
    if (row<0 || !ConsumableCount(patch,row)) return false;
    for (int side=0;side<2;++side) {
        if ((side?World.Sys_UI.MFD_RightTab:World.Sys_UI.MFD_LefTab)!=2 || World.Sys_UI.mfdItemReader[side]) continue;
        i16 dx=side?1059:0;
        if (left && CursorIsOverBounds(dx+72,dx+232,691,731)) {
            World.Sys_UI.lastItemSideRH=side!=0; Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=false; World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true; ConsumableUse(patch,row); return true;
        }
        if (!patch && row>=5 && Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].down && CursorIsOverBounds(dx+40,dx+264,650,674)) {
            World.Sys_UI.lastItemSideRH=side!=0; ConsumableSetTimer((World.cursorPos_x-dx-40)/224.0f);
            Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=false; World.Sys_UI.mouseClickHeldOverGUI=World.uiIsBlocking=true; return true;
        }
    }
    return false;
}
void RenderConsumables(void) {
    for (int kind=0;kind<2;++kind) {
        i16 x=kind?831:734; RenderTextL(x,560,T_RED,FONT_NORMAL,0.8f,"%s",Sys_Text.stringTable[kind?873:872]);
        for (int row=0;row<7;++row) {
            u8 count=ConsumableCount(kind,row); if (!count) continue;
            int slot=ConsumableSlot(kind,row); u32 color=(kind?World.invP1.patchCur:World.invP1.grenCur)==slot?T_YELLOW:T_GREEN; i16 y=consumableRowY[row];
            const char* text=Sys_Text.stringTable[(kind?907:900)+row]; float width=MeasureLineAdvance(text,FONT_NORMAL),scale=width>0?vmin(0.6f,(kind?62.0f:59.0f)/width):0.6f;
            RenderTextL(x,y,color,FONT_NORMAL,scale,"%s",text); RenderTextL(kind?895:706,y,color,FONT_NORMAL,0.6f,"%u",count);
            RenderUIImage(kind?920:795,y+1,20,20,1086); RenderUIImage(kind?925:800,y+6,11,11,1079);
        }
    }
}
void RenderConsumableItem(bool isRH) {
    int row=ConsumableSelectedRow(); bool patch=World.Sys_UI.mfdConsumable==2; if (row<0 || !ConsumableCount(patch,row)) return;
    i16 dx=isRH?1059:0; int item=ConsumableItem(patch,row); const char* text=Sys_Text.stringTable[item+326]; float width=MeasureLineAdvance(text,FONT_NORMAL);
    RenderTextL(dx+28,540,T_YELLOW,FONT_NORMAL,width>0?vmin(0.6f,260.0f/width):0.6f,"%s",text);
    u16 tex=GetItemFrobTexture(item+307); if (tex<MAX_TXRS) RenderUIImage(dx+112,560,80,64,tex);
    RenderUIImage(dx+72,691,160,40,1087); RenderTextL(dx+72,691,T_GREEN_MENU,FONT_NORMAL,0.6f,"%s",Sys_Text.stringTable[736]);
    if (!patch && row>=5) {
        float min=row==5?2.0f:4.0f,value=row==5?World.invP1.nitroTimeSetting:World.invP1.earthShakerTimeSetting;
        RenderTextL(dx+112,626,T_GREEN,FONT_NORMAL,0.6f,"%.1f",(double)value);
        RenderUIImage(dx+40,650,224,24,1087); RenderUIImage(dx+40+(i16)(204.0f*vclamp((value-min)/(60.0f-min),0.0f,1.0f)),652,20,20,1086);
    }
}
bool InventoryPointerHover(void) {
    if (World.Sys_UI.MFD_CenterTab==3 && !World.invP1.holdingObject) {
        if (TextPointerHover(454,557,Sys_Text.stringTable[875],0.6f,260)) return true;
        for (int slot=0;slot<14;++slot) {
            if (GeneralInvTextHover(slot)) return true;
            i16 x=slot<7?454:681,y=generalRowY[slot%7]; if (GeneralInvCanUse(slot) && CursorIsOverBounds(x+187,x+207,y+2,y+22)) return true;
        }
    }
    if (World.Sys_UI.MFD_CenterTab==1) {
        for (int kind=0;kind<2;++kind) {
            if (TextPointerHover(kind?831:734,584,Sys_Text.stringTable[kind?873:872],0.6f,120)) return true;
            for (int row=0;row<7;++row) if (ConsumableRowHover(kind,row)) return true;
        }
        if (TextPointerHover(372,560,"WEAPONS",0.8f,200) || TextPointerHover(574,560,"SHOTS",0.8f,120)) return true;
        for (int slot=0;slot<7;++slot) if (World.invP1.weaponInventoryIndices[slot]>=0 && CursorIsOverBounds(372,712,577+22*slot,598+22*slot)) return true;
    }
    for (int side=0;side<2;++side) {
        if ((side?World.Sys_UI.MFD_RightTab:World.Sys_UI.MFD_LefTab)!=2) continue;
        i16 dx=side?1059:0;
        if (World.Sys_UI.mfdItemReader[side]) {
            i16 x=side?1080:22;
            if (TextPointerHover(x+6,540,Sys_Text.stringTable[349],0.6f,260)) return true;
            for (int section=0;section<4;++section) if ((section!=MM_NOTES || World.diffMis) && CursorIsOverBounds(x+65*section,x+65*(section+1),718,758)) return true;
            continue;
        }
        int row=ConsumableSelectedRow(),slot=World.invP1.generalInvCurrent; bool patch=World.Sys_UI.mfdConsumable==2;
        int item=World.Sys_UI.mfdConsumable?(ConsumableCount(patch,row)?ConsumableItem(patch,row):-1):World.Sys_UI.mfdGeneralItem?GeneralInvItem(slot):-1;
        if (item<0) continue;
        if (TextPointerHover(dx+28,540,World.Sys_UI.mfdConsumable?Sys_Text.stringTable[item+326]:GeneralInvLabel(slot),0.6f,260)) return true;
        if ((GetItemFrobTexture(item+307)<MAX_TXRS || (item>=92 && item<=94)) && CursorIsOverBounds(dx+112,dx+192,560,624)) return true;
        if ((World.Sys_UI.mfdConsumable || GeneralInvCanUse(slot)) && CursorIsOverBounds(dx+72,dx+232,691,731)) return true;
        if (!World.Sys_UI.mfdConsumable && GeneralInvCanVaporize(slot) && CursorIsOverBounds(dx+72,dx+232,628,668)) return true;
        if (World.Sys_UI.mfdConsumable==1 && row>=5 && (CursorIsOverBounds(dx+40,dx+264,650,674) || CursorIsOverBounds(dx+112,dx+170,626,646))) return true;
        if (!World.Sys_UI.mfdConsumable && !slot) {
            i16 x=dx+36,y=638;
            for (int card=ACC_Std;card<=ACC_Per5;++card) if (InventoryHasAccessCard(card)) {
                const char* code=AccessCardCodeForType(card); float w=MeasureLineAdvance(code,FONT_NORMAL)*0.6f;
                if (x+w>dx+280) { x=dx+36; y+=20; } if (CursorIsOverBounds(x,x+w,y,y+20)) return true; x+=(i16)(w+8);
            }
        }
    }
    return false;
}
static bool UI_GeneralInventory(void) {
    if (World.Sys_UI.generalClickSlot>=0 && (World.Sys_UI.MFD_CenterTab!=3 || World.invP1.holdingObject || !GeneralInvTextHover(World.Sys_UI.generalClickSlot) || GeneralInvItem(World.Sys_UI.generalClickSlot)!=World.Sys_UI.generalClickItem || World.invP1.generalInvCustIdx[World.Sys_UI.generalClickSlot]!=World.Sys_UI.generalClickCustom || World.pauseRelativeTime-World.Sys_UI.generalClickTime>0.5)) MFD_GeneralChanged();
    if (World.Sys_UI.MFD_CenterTab==3 && !World.invP1.holdingObject) for (u8 slot=0;slot<14;++slot) {
        int item=GeneralInvItem(slot); if (item<0) continue;
        i16 x=slot<7?454:681,y=generalRowY[slot%7];
        if (GeneralInvCanUse(slot) && HwBtnClick(x+187,x+207,y+2,y+22)) {
            MFD_GeneralChanged(); GeneralInvClick(slot,World.invP1.generalInvCustIdx[slot]); GeneralInvApply(slot,World.invP1.generalInvCustIdx[slot]); return true;
        }
        if (!GeneralInvTextHover(slot)) continue;
        bool right=Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed;
        if (!HwBtnClick(x,x+GeneralInvTextWidth(slot),y,y+20)) continue;
        bool twice=!right && World.Sys_UI.generalClickSlot==slot && World.Sys_UI.generalClickItem==item && World.Sys_UI.generalClickCustom==World.invP1.generalInvCustIdx[slot] && World.pauseRelativeTime-World.Sys_UI.generalClickTime<=0.5;
        MFD_GeneralChanged();
        if (right && slot) { GeneralInvTake(slot); return true; }
        GeneralInvClick(slot,World.invP1.generalInvCustIdx[slot]);
        if (twice) GeneralInvApply(slot,World.invP1.generalInvCustIdx[slot]);
        else if (!right) { World.Sys_UI.generalClickSlot=(i8)slot; World.Sys_UI.generalClickItem=(i16)item; World.Sys_UI.generalClickCustom=World.invP1.generalInvCustIdx[slot]; World.Sys_UI.generalClickTime=World.pauseRelativeTime; }
        return true;
    }
    if (World.Sys_UI.mfdGeneralItem) for (u8 side=0;side<2;++side) {
        if ((side?World.Sys_UI.MFD_RightTab:World.Sys_UI.MFD_LefTab)!=2 || World.Sys_UI.mfdItemReader[side]) continue;
        i16 dx=side?1059:0; int slot=World.invP1.generalInvCurrent;
        bool use=GeneralInvCanUse(slot),vapor=GeneralInvCanVaporize(slot);
        if ((use && HwBtnClick(dx+72,dx+232,691,731)) || (vapor && HwBtnClick(dx+72,dx+232,628,668))) {
            MFD_GeneralChanged(); if (use) GeneralInvApply(slot,World.invP1.generalInvCustIdx[slot]); else VaporizeClick(); return true;
        }
    }
    if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed || Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed) MFD_GeneralChanged();
    return false;
}
void RenderGeneralInventory(void) {
    RenderTextL(454,557,T_RED,FONT_NORMAL,0.6,"%s",Sys_Text.stringTable[875]);
    for (u8 slot=0;slot<14;++slot) {
        if (GeneralInvItem(slot)<0) continue;
        i16 x=slot<7?454:681,y=generalRowY[slot%7]; float width=MeasureLineAdvance(GeneralInvLabel(slot),FONT_NORMAL),scale=width>0?vmin(0.6f,185.0f/width):0.6f;
        RenderTextL(x,y,World.invP1.generalInvCurrent==slot?T_YELLOW:T_GREEN,FONT_NORMAL,scale,"%s",GeneralInvLabel(slot));
        if (GeneralInvCanUse(slot)) { RenderUIImage(x+187,y+2,20,20,1086); RenderUIImage(x+192,y+7,11,11,1079); }
    }
}
void RenderGeneralItem(bool isRH) {
    if (World.Sys_UI.mfdConsumable) { RenderConsumableItem(isRH); return; }
    if (!World.Sys_UI.mfdGeneralItem) return;
    int slot=World.invP1.generalInvCurrent,item=GeneralInvItem(slot); if (item<0) return;
    i16 dx=isRH?1059:0;
    float width=MeasureLineAdvance(GeneralInvLabel(slot),FONT_NORMAL),scale=width>0?vmin(0.6f,260.0f/width):0.6f;
    RenderTextL(dx+28,540,T_YELLOW,FONT_NORMAL,scale,"%s",GeneralInvLabel(slot));
    u16 tex=GetItemFrobTexture((u16)(item+307));
    if (item>=92 && item<=94) {
        static const u8 heads[19]={37,11,32,1,7,9,10,12,13,14,15,17,25,27,28,31,33,35,36};
        u16 custom=World.invP1.generalInvCustIdx[slot]; tex=(u16)(1272+heads[custom<19?custom:0]);
    }
    if (tex<MAX_TXRS) RenderUIImage(dx+112,560,80,64,tex);
    if (!slot) {
        i16 x=dx+36,y=638;
        for (int card=ACC_Std;card<=ACC_Per5;++card) if (InventoryHasAccessCard((AccCardType)card)) {
            const char* code=AccessCardCodeForType((AccCardType)card); float w=MeasureLineAdvance(code,FONT_NORMAL)*0.6f;
            if (x+w>dx+280) { x=dx+36; y+=20; }
            RenderTextL(x,y,T_YELLOW,FONT_NORMAL,0.6,"%s",code); x+=(i16)(w+8);
        }
    } else if (GeneralInvCanUse(slot) || GeneralInvCanVaporize(slot)) {
        bool use=GeneralInvCanUse(slot); i16 y=use?691:628;
        RenderUIImage(dx+72,y,160,40,1087); RenderTextL(dx+72,y,T_GREEN_MENU,FONT_NORMAL,0.6,"%s",Sys_Text.stringTable[use?736:883]);
    }
}
void UpdateSearchTether(void),CloseSearch(void); bool SearchTakeSlot(u8 slot);
void UI_ProcessNavigation(void) {
    UpdateSearchTether();
    if (World.menuActive || World.paused || World.creditsActive || Cheats.consoleActive || World.Sys_UI.vmailActive) { MFD_GeneralChanged(); return; }
    static const u16 keys[7]={KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F7,KEY_F8}; static const u8 tabs[7]={1,2,3,4,1,2,3};
    for (u8 i=0;i<7;++i) if (Sys_Input.keyStates[keys[i]].pressed) { Sys_Input.keyStates[keys[i]].pressed=false; MFD_SelectTab(i<4?1:2,tabs[i],true); }
    for (u8 up=0;up<2;++up) { u16 key=up?KEY_PAGE_UP:KEY_PAGE_DOWN; if (!Sys_Input.keyStates[key].pressed) continue;
        Sys_Input.keyStates[key].pressed=false; u8 tab=World.Sys_UI.MFD_CenterTab?World.Sys_UI.MFD_CenterTab:World.Sys_UI.mfdSelected[0];
        MFD_SelectTab(0,tab==5?1:1+(tab-1+(up?3:1))%4,false); World.Sys_UI.MFD_ReaderView=MFD_READER_CONTENTS;
    }
    if (!World.inventoryMode || Cheats.noHUD) { MFD_GeneralChanged(); return; }
    if (UI_Consumables() || UI_GeneralInventory() || UI_SoftwareInventory()) return;
    if (HwBtnClick(667,699,0,32)) { ForceShootMode(); return; }
    if ((World.invP1.hasHardware&HW_ERD) && HwBtnClick(1326,1366,240,280)) {
        MFD_ResetGeneral(); World.Sys_UI.MFD_CenterTab=5; World.Sys_UI.MFD_LefTab=2; World.Sys_UI.mfdItemReader[0]=true; World.Sys_UI.MFD_ReaderView=MFD_READER_CONTENTS;
        World.Sys_UI.MFD_MediaTab=World.Sys_UI.lastMultiMediaTabOpened; if (World.Sys_UI.MFD_MediaTab>MM_NOTES || (World.Sys_UI.MFD_MediaTab==MM_NOTES && !World.diffMis)) World.Sys_UI.MFD_MediaTab=MM_LOG_TABLE;
        play_wav(sounds[97],SfxVol(),(V3){0,0,0},false); return;
    }
    static const i16 centerX[4]={400,480,560,902};
    for (u8 i=0;i<4;++i) if (HwBtnClick(centerX[i],centerX[i]+64,752,784)) { MFD_SelectTab(0,i+1,true); return; }
    for (u8 side=0;side<2;++side) {
        for (u8 i=0;i<4;++i) if (HwBtnClick(side?1350:-16,side?1382:16,520+56*i,560+56*i)) { MFD_SelectTab(side+1,tabs[i],true); return; }
        if ((side?World.Sys_UI.MFD_RightTab:World.Sys_UI.MFD_LefTab)==4 && (side?World.Sys_UI.MFD_DataR:World.Sys_UI.MFD_DataL)==5 && World.Sys_UI.tetheredSearchable!=U16_MAX) {
            i16 dx=side?1059:0;
            i16 closeY=side?528:534;
            if (HwBtnClick(dx+259,dx+288,closeY,closeY+29)) { CloseSearch(); return; }
            for (u8 slot=0;slot<4;++slot) { i16 x=dx+84+90*(slot&1),y=584+90*(slot>>1);
                if (World.instances[World.Sys_UI.tetheredSearchable].contents[slot]<0) continue;
                if (HwBtnClick(x,x+64,y,y+64)) { World.Sys_UI.lastSearchSideRH=side!=0; SearchTakeSlot(slot); return; }
            }
        }
        if ((side?World.Sys_UI.MFD_RightTab:World.Sys_UI.MFD_LefTab)!=2 || !World.Sys_UI.mfdItemReader[side]) continue;
        for (u8 section=0;section<4;++section) { if (section==MM_NOTES && !World.diffMis) continue; i16 x=(side?1080:22)+65*section;
            if (!HwBtnClick(x,x+64,718,758)) continue;
            World.Sys_UI.MFD_CenterTab=5; World.Sys_UI.MFD_MediaTab=World.Sys_UI.lastMultiMediaTabOpened=section; World.Sys_UI.MFD_ReaderView=MFD_READER_CONTENTS;
            if (section>=MM_DATA_TABLE) { World.Sys_UI.highlightStatus[section]=false; World.Sys_UI.highlightTickCount[section]=0; }
            play_wav(sounds[97],SfxVol(),(V3){0,0,0},false); return;
        }
    }
}

void RenderSearchFX(void) {
    for (int side=0;side<2;++side) {
        if (!World.Sys_UI.searchFXActive[side]) continue;
        double elapsed = World.pauseRelativeTime - World.Sys_UI.searchFXStartTime[side];
        if (elapsed >= 1.0) { World.Sys_UI.searchFXActive[side] = false; continue; }
        if (Cheats.noHUD || (side?World.Sys_UI.MFD_RightTab:World.Sys_UI.MFD_LefTab)!=4 || (side?World.Sys_UI.MFD_DataR:World.Sys_UI.MFD_DataL)!=5 || World.Sys_UI.tetheredSearchable==U16_MAX) continue;
        float t = (float)elapsed / 1.0f;
        if (t > 1.0f) t = 1.0f;
        float p = t < 0.4f ? t / 0.4f : 1.0f;/*scale up first 0.4s, hold*/
        float ep = 1.0f - (1.0f - p) * (1.0f - p) * (1.0f - p);/*ease-out cubic*/
        float scale = 40.0f + ep * (263.0f - 40.0f);
        float w = scale, h = scale * 240.0f / 263.0f;
        float sx = World.Sys_UI.searchFXCursorX[side];
        float sy = World.Sys_UI.searchFXCursorY[side];
        float ex = (side ? 1210.5f : 151.5f);
        float ey = 648.0f;
        float cx = sx + (ex - sx) * ep;
        float cy = sy + (ey - sy) * ep;
        RenderUIImage((i16)(cx - w * 0.5f),(i16)(cy - h * 0.5f),(i16)(w + 0.5f),(i16)(h + 0.5f),1074);
    }
}

u16 GetItemFrobTexture(u16 index);
void RenderSearch(bool isRH) {
    u16 s=World.Sys_UI.tetheredSearchable;
    if (s<INSTS_1ST_IDX || s>=World.instCount || !(World.instances[s].entflags&EF_ACTIVE) || !World.instances[s].srchInUse) return;
    Entity* e=&World.instances[s]; i16 dx=isRH?1059:0; u16 label=0;
    switch (e->index) {
        case 464: label=895; break; case 531: label=896; break; case 530: label=898; break;
        case 465: case 466: case 467: case 468: case 469: case 470: case 471: label=897; break;
        case 472: case 473: case 474: case 475: case 476: label=899; break;
    }
    if (label) RenderTextL(dx+34,536,T_YELLOW,FONT_NORMAL,0.6,"%s",Sys_Text.stringTable[label]);
    else if (IdxIsNPC(e->index)) RenderTextL(dx+34,536,T_YELLOW,FONT_NORMAL,0.6,"%s",npcTable[e->index-419].name);
    bool any=false;
    for (u8 slot=0;slot<4;++slot) {
        i16 item=e->contents[slot]; if (item<0 || item>110) continue;
        any=true; u16 tex=GetItemFrobTexture((u16)(item+307));
        if (tex<MAX_TXRS) RenderUIImage(dx+84+90*(slot&1),584+90*(slot>>1),64,64,tex);
    }
    if (!any) RenderTextL(dx+24,633,T_YELLOW,FONT_NORMAL,0.6,"%s",Sys_Text.stringTable[891]);
    RenderUIImage(dx+259,isRH?528:534,29,29,899); RenderTextL(dx+259,isRH?531:534,T_STOPD_RED,FONT_NORMAL,0.6,"X");
}

void SideMFD(bool isRH) {
    i16 dx = isRH ? 1059 : 0; u8 tab = isRH ? World.Sys_UI.MFD_RightTab : World.Sys_UI.MFD_LefTab, data = isRH ? World.Sys_UI.MFD_DataR : World.Sys_UI.MFD_DataL;
    if (tab==2 && !World.Sys_UI.mfdItemReader[isRH?1:0]) RenderGeneralItem(isRH);
    if(tab==4){/*DataTabLH*/
        if(data==8){/*Blocked*/
            RenderUIImage(31+dx,535,227,209,1110);/*BlockedBySecurityLH UNMAPPED:[Resources/BlockedBySecurity/blocked_00. ImageSequenceTextureArrayUI.cs,PooledItemDestroy.cs*/
            RenderTextL(45+dx,542,T_YELLOW,FONT_NORMAL,0.6,"%s",890<1100?Sys_Text.stringTable[890]:"Blocked by SHODAN level Security.");/*BlockedBySecurityText UIPointerMask.cs*/
        }
        if(data==1){/*Elevator*/
            CreateUIElement((V2){132+dx,531},(V2){164+dx,563},isRH ? UI_ID_RMFD_ELEV_CURRENT_FLOOR_INDICATOR : UI_ID_LMFD_ELEV_CURRENT_FLOOR_INDICATOR); RenderUIImage(132+dx,531,32,32,929);/*CurrentFloorIndicator*/
            RenderUIImage(86+dx,578,45,168,0);/*ButtonBankLH QUAD:builtin-knob*/
            CreateUIElement((V2){86+dx,578},(V2){131+dx,617},isRH ? UI_ID_RMFD_ELEV_BUTTON_1 : UI_ID_LMFD_ELEV_BUTTON_1); RenderTextL(89+dx,580,T_GREEN,FONT_NORMAL,0.6,"R");/*Text (1)*/ RenderUIImage(86+dx,578,45,39,2133);/*ElevButton1 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png] UIButtonMask.cs*/ RenderUIImage(88+dx,583,40,34,2134);/*Keypad.Button (1) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Keypad.Button (1): ElevButtonClick() ElevatorButton.cs,UIButtonMask.cs*/
            RenderUIImage(86+dx,620,45,39,2135);/*ElevButton2 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png] UIButtonMask.cs*/
            RenderUIImage(88+dx,623,40,34,2134);/*Keypad.Button (2) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Keypad.Button (2): ElevButtonClick() ElevatorButton.cs,UIButtonMask.cs*/
            RenderTextL(89+dx,620,T_GREEN,FONT_NORMAL,0.6,"1");/*Text (2)*/
            RenderUIImage(86+dx,663,45,39,2135);/*ElevButton3 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png] UIButtonMask.cs*/
            RenderUIImage(88+dx,666,40,34,2134);/*Keypad.Button (3) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Keypad.Button (3): ElevButtonClick() ElevatorButton.cs,UIButtonMask.cs*/
            RenderTextL(89+dx,663,T_GREEN,FONT_NORMAL,0.6,"2");/*Text (3)*/
            RenderUIImage(86+dx,706,45,39,2133);/*ElevButton4 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png] UIButtonMask.cs*/
            RenderUIImage(88+dx,707,40,34,2134);/*Keypad.Button (4) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Keypad.Button (4): ElevButtonClick() ElevatorButton.cs,UIButtonMask.cs*/
            RenderTextL(89+dx,704,T_GREEN,FONT_NORMAL,0.6,"3");/*Text (4)*/
            RenderUIImage(164+dx,578,45,168,0);/*ButtonBankRH QUAD:builtin-knob*/
            RenderUIImage(164+dx,578,45,39,2133);/*ElevButton5 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png] UIButtonMask.cs*/
            RenderUIImage(167+dx,582,40,34,2134);/*Keypad.Button (5) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Keypad.Button (5): ElevButtonClick() ElevatorButton.cs,UIButtonMask.cs*/
            RenderTextL(168+dx,580,T_GREEN,FONT_NORMAL,0.6,"6");/*Text (5)*/
            RenderUIImage(164+dx,620,45,39,2135);/*ElevButton6 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png] UIButtonMask.cs*/
            RenderUIImage(167+dx,623,40,34,2134);/*Keypad.Button (6) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Keypad.Button (6): ElevButtonClick() ElevatorButton.cs,UIButtonMask.cs*/
            RenderTextL(168+dx,620,T_GREEN,FONT_NORMAL,0.6,"7");/*Text (6)*/
            RenderUIImage(164+dx,663,45,39,2135);/*ElevButton7 UNMAPPED:[Textures/UI/hudbuttons/keypad_mid.png] UIButtonMask.cs*/
            RenderUIImage(167+dx,666,40,34,2134);/*Keypad.Button (7) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Keypad.Button (7): ElevButtonClick() ElevatorButton.cs,UIButtonMask.cs*/
            RenderTextL(168+dx,663,T_GREEN,FONT_NORMAL,0.6,"8");/*Text (7)*/
            RenderUIImage(164+dx,706,45,39,2133);/*ElevButton8 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png] UIButtonMask.cs*/
            RenderUIImage(167+dx,707,40,34,2134);/*Keypad.Button (8) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Keypad.Button (8): ElevButtonClick() ElevatorButton.cs,UIButtonMask.cs*/
            RenderTextL(168+dx,704,T_GREEN,FONT_NORMAL,0.6,"9");/*Text (8)*/
            RenderUIImage(246+dx,528,29,29,899);/*CloseButton BTN CloseButton: MFDManager.CloseElevatorPad() UIButtonMask.cs*/
            RenderTextL(246+dx,528,T_STOPD_RED,FONT_NORMAL,0.6,"X");/*KeycodeUIControlLH: KeypadKeycodeButtons.cs*/
        }
        if(data==2){/*Keycode*/
            RenderUIImage(86+dx,577,42,38,2133);/*KeycodeButton1 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(88+dx,580,38,35,2134);/*Button (1) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (1): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(80+dx,572,T_GREEN,FONT_NORMAL,0.6,"1");
            RenderUIImage(127+dx,577,42,38,2133);/*KeycodeButton2 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(129+dx,580,38,35,2134);/*Button (2) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (2): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(121+dx,572,T_GREEN,FONT_NORMAL,0.6,"2");
            RenderUIImage(169+dx,577,42,38,2133);/*KeycodeButton3 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(169+dx,580,38,35,2134);/*Button (3) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (3): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(161+dx,572,T_GREEN,FONT_NORMAL,0.6,"3");
            RenderUIImage(86+dx,620,42,38,2133);/*KeycodeButton4 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(88+dx,621,38,35,2134);/*Button (4) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (4): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(80+dx,614,T_GREEN,FONT_NORMAL,0.6,"4");
            RenderUIImage(127+dx,620,42,38,2133);/*KeycodeButton5 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(129+dx,621,38,35,2134);/*Button (5) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (5): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(121+dx,614,T_GREEN,FONT_NORMAL,0.6,"5");
            RenderUIImage(169+dx,620,42,38,2133);/*KeycodeButton6 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(169+dx,621,38,35,2134);/*Button (6) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (6): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(162+dx,614,T_GREEN,FONT_NORMAL,0.6,"6");
            RenderUIImage(86+dx,663,42,38,2133);/*KeycodeButton7 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(88+dx,665,38,35,2134);/*Button (7) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (7): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(80+dx,657,T_GREEN,FONT_NORMAL,0.6,"7");
            RenderUIImage(127+dx,663,42,38,2133);/*KeycodeButton8 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(129+dx,665,38,35,2134);/*Button (8) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (8): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(121+dx,657,T_GREEN,FONT_NORMAL,0.6,"8");
            RenderUIImage(169+dx,663,42,38,2133);/*KeycodeButton9 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(169+dx,665,38,35,2134);/*Button (9) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (9): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(162+dx,657,T_GREEN,FONT_NORMAL,0.6,"9");
            RenderUIImage(86+dx,706,42,38,2133);/*KeycodeButtonBackSpace UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(88+dx,707,38,35,2134);/*Button (-) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (-): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(80+dx,700,T_GREEN,FONT_NORMAL,0.6,"-");
            RenderUIImage(127+dx,706,42,38,2133);/*KeycodeButton0 UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(129+dx,707,38,35,2134);/*Button (0) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (0): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(121+dx,700,T_GREEN,FONT_NORMAL,0.6,"0");
            RenderUIImage(169+dx,706,42,38,2133);/*KeycodeButtonC UNMAPPED:[Textures/UI/hudbuttons/keypad_end.png]*/
            RenderUIImage(169+dx,707,38,35,2134);/*Button (C) UNMAPPED:[Textures/UI/hudbuttons/keypad_inner_on. BTN Button (C): KeycodeButtonClick() KeycodeButton.cs*/
            RenderTextL(161+dx,700,T_GREEN,FONT_NORMAL,0.6,"C");
            RenderUIImage(173+dx,526,32,32,2132);/*KeycodeOnes UNMAPPED:[Textures/UI/elnum_null.png] KeycodeDigitImage.cs*/
            RenderUIImage(132+dx,526,32,32,2132);/*KeycodeTens UNMAPPED:[Textures/UI/elnum_null.png] KeycodeDigitImage.cs*/
            RenderUIImage(255+dx,525,29,29,899);/*CloseButton BTN CloseButton: MFDManager.CloseKeycodePad() UIButtonMask.cs*/
            RenderTextL(255+dx,525,T_STOPD_RED,FONT_NORMAL,0.6,"X");
            RenderUIImage(90+dx,526,32,32,2132);/*KeycodeHuns UNMAPPED:[Textures/UI/elnum_null.png] KeycodeDigitImage.cs*/
        }
        if (data==5) RenderSearch(isRH);
        if(data==6){/*AudioLog*/
            RenderUIImage(20+dx,528,263,240,1272);/*LogImage*/
            RenderTextL(29+dx,540,T_YELLOW,FONT_NORMAL,0.6,"HACKER IS AWESOME");/*LogName UIPointerMask.cs*/
            RenderTextL(29+dx,557,T_YELLOW,FONT_NORMAL,0.6,"Sender: SHODAN");/*SenderText UIPointerMask.cs*/
            RenderTextL(29+dx,701,T_YELLOW,FONT_NORMAL,0.6,"Subject:\n\nif only i had a sparq beam then all the world would be right");/*SubjectText UIPointerMask.cs PuzzleGridLH: PuzzleGrid.cs*/
        }
        if(data==3){/*GridPuzzle*/
            RenderUIImage(42+dx,555,221,163,2139);/*OuterColorBorder UNMAPPED:[Textures/UI/puzzle/gridcontainer_gray.p*/
            RenderUIImage(46+dx,558,214,157,2138);/*ContainerEdge UNMAPPED:[Textures/UI/puzzle/gridcontainer.png]*/
            RenderUIImage(25+dx,621,29,29,2141);/*NodeSource UNMAPPED:[Textures/UI/puzzle/node_source.png]*/
            RenderUIImage(250+dx,621,29,29,2140);/*Node UNMAPPED:[Textures/UI/puzzle/node_off.png]*/
            RenderUIImage(51+dx,565,29,29,2137);/*Button UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button: PuzzleGridLH.OnGridCellClick() UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(51+dx,565,29,29,2136);/*GeniusHighlight UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderTextL(51+dx,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(80+dx,565,29,29,2137);/*Button (1) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (1): PuzzleGridLH.OnGridCellClick(1) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(80+dx,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(80+dx,565,29,29,2136);/*GeniusHighlight (1) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(109+dx,565,29,29,2137);/*Button (2) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (2): PuzzleGridLH.OnGridCellClick(2) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(109+dx,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(109+dx,565,29,29,2136);/*GeniusHighlight (2) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(138+dx,565,29,29,2137);/*Button (3) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (3): PuzzleGridLH.OnGridCellClick(3) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(138+dx,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(138+dx,565,29,29,2136);/*GeniusHighlight (3) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(166+dx,565,29,29,2137);/*Button (4) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (4): PuzzleGridLH.OnGridCellClick(4) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(166+dx,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(166+dx,565,29,29,2136);/*GeniusHighlight (4) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(195+dx,565,29,29,2137);/*Button (5) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (5): PuzzleGridLH.OnGridCellClick(5) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(195+dx,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(195+dx,565,29,29,2136);/*GeniusHighlight (5) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(224+dx,565,29,29,2137);/*Button (6) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (6): PuzzleGridLH.OnGridCellClick(6) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(224+dx,565,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(224+dx,565,29,29,2136);/*GeniusHighlight (6) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(51+dx,594,29,29,2137);/*Button (7) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (7): PuzzleGridLH.OnGridCellClick(7) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(51+dx,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(51+dx,594,29,29,2136);/*GeniusHighlight (7) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(80+dx,594,29,29,2137);/*Button (8) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (8): PuzzleGridLH.OnGridCellClick(8) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(80+dx,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(80+dx,594,29,29,2136);/*GeniusHighlight (8) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(109+dx,594,29,29,2137);/*Button (9) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (9): PuzzleGridLH.OnGridCellClick(9) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(109+dx,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(109+dx,594,29,29,2136);/*GeniusHighlight (9) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(138+dx,594,29,29,2137);/*Button (10) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (10): PuzzleGridLH.OnGridCellClick(10) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(138+dx,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(138+dx,594,29,29,2136);/*GeniusHighlight (10) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(166+dx,594,29,29,2137);/*Button (11) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (11): PuzzleGridLH.OnGridCellClick(11) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(166+dx,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(166+dx,594,29,29,2136);/*GeniusHighlight (11) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(195+dx,594,29,29,2137);/*Button (12) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (12): PuzzleGridLH.OnGridCellClick(12) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(195+dx,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(195+dx,594,29,29,2136);/*GeniusHighlight (12) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(224+dx,594,29,29,2137);/*Button (13) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (13): PuzzleGridLH.OnGridCellClick(13) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(224+dx,594,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(224+dx,594,29,29,2136);/*GeniusHighlight (13) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(51+dx,622,29,29,2137);/*Button (14) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (14): PuzzleGridLH.OnGridCellClick(14) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(51+dx,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(51+dx,622,29,29,2136);/*GeniusHighlight (14) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(80+dx,622,29,29,2137);/*Button (15) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (15): PuzzleGridLH.OnGridCellClick(15) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(80+dx,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(80+dx,622,29,29,2136);/*GeniusHighlight (15) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(109+dx,622,29,29,2137);/*Button (16) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (16): PuzzleGridLH.OnGridCellClick(16) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(109+dx,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(109+dx,622,29,29,2136);/*GeniusHighlight (16) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(138+dx,622,29,29,2137);/*Button (17) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (17): PuzzleGridLH.OnGridCellClick(17) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(138+dx,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(138+dx,622,29,29,2136);/*GeniusHighlight (17) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(166+dx,622,29,29,2137);/*Button (18) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (18): PuzzleGridLH.OnGridCellClick(18) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(166+dx,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(166+dx,622,29,29,2136);/*GeniusHighlight (18) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(195+dx,622,29,29,2137);/*Button (19) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (19): PuzzleGridLH.OnGridCellClick(19) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(195+dx,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(195+dx,622,29,29,2136);/*GeniusHighlight (19) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(224+dx,622,29,29,2137);/*Button (20) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (20): PuzzleGridLH.OnGridCellClick(20) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(224+dx,622,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(224+dx,622,29,29,2136);/*GeniusHighlight (20) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(51+dx,651,29,29,2137);/*Button (21) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (21): PuzzleGridLH.OnGridCellClick(21) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(51+dx,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(51+dx,651,29,29,2136);/*GeniusHighlight (21) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(80+dx,651,29,29,2137);/*Button (22) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (22): PuzzleGridLH.OnGridCellClick(22) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(80+dx,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(80+dx,651,29,29,2136);/*GeniusHighlight (22) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(109+dx,651,29,29,2137);/*Button (23) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (23): PuzzleGridLH.OnGridCellClick(23) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(109+dx,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(109+dx,651,29,29,2136);/*GeniusHighlight (23) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(138+dx,651,29,29,2137);/*Button (24) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (24): PuzzleGridLH.OnGridCellClick(24) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(138+dx,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(138+dx,651,29,29,2136);/*GeniusHighlight (24) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(166+dx,651,29,29,2137);/*Button (25) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (25): PuzzleGridLH.OnGridCellClick(25) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(166+dx,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(166+dx,651,29,29,2136);/*GeniusHighlight (25) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(195+dx,651,29,29,2137);/*Button (26) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (26): PuzzleGridLH.OnGridCellClick(26) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(195+dx,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(195+dx,651,29,29,2136);/*GeniusHighlight (26) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(224+dx,651,29,29,2137);/*Button (27) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (27): PuzzleGridLH.OnGridCellClick(27) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(224+dx,651,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(224+dx,651,29,29,2136);/*GeniusHighlight (27) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(51+dx,680,29,29,2137);/*Button (28) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (28): PuzzleGridLH.OnGridCellClick(28) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(51+dx,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(51+dx,680,29,29,2136);/*GeniusHighlight (28) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(80+dx,680,29,29,2137);/*Button (29) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (29): PuzzleGridLH.OnGridCellClick(29) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(80+dx,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(80+dx,680,29,29,2136);/*GeniusHighlight (29) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(109+dx,680,29,29,2137);/*Button (30) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (30): PuzzleGridLH.OnGridCellClick(30) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(109+dx,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(109+dx,680,29,29,2136);/*GeniusHighlight (30) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(138+dx,680,29,29,2137);/*Button (31) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (31): PuzzleGridLH.OnGridCellClick(31) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(138+dx,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(138+dx,680,29,29,2136);/*GeniusHighlight (31) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(166+dx,680,29,29,2137);/*Button (32) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (32): PuzzleGridLH.OnGridCellClick(32) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(166+dx,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(166+dx,680,29,29,2136);/*GeniusHighlight (32) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(195+dx,680,29,29,2137);/*Button (33) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (33): PuzzleGridLH.OnGridCellClick(33) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(195+dx,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(195+dx,680,29,29,2136);/*GeniusHighlight (33) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(224+dx,680,29,29,2137);/*Button (34) UNMAPPED:[Textures/UI/puzzle/grid1_base.png] BTN Button (34): PuzzleGridLH.OnGridCellClick(34) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderTextL(224+dx,680,T_GREEN_MENU_SHADOW,FONT_NORMAL,0.6,"?");
            RenderUIImage(224+dx,680,29,29,2136);/*GeniusHighlight (34) UNMAPPED:[Textures/UI/puzzle/geniusgrid_highlight*/
            RenderUIImage(42+dx,720,221,26,2139);/*ProgressContainer UNMAPPED:[Textures/UI/puzzle/gridcontainer_gray.p*/
            RenderUIImage(45+dx,726,225,13,0);/*Background QUAD:builtin-knob*/
            RenderUIImage(48+dx,726,6,13,2142);/*Fill UNMAPPED:[Textures/UI/puzzle/puzzlesliderwire.png*/
            RenderUIImage(45+dx,720,22,26,1078);/*Handle*/
            RenderUIImage(259+dx,527,29,29,899);/*CloseButton BTN CloseButton: MFDManager.ClosePuzzleGrid() UIButtonMask.cs*/
            RenderTextL(259+dx,527,T_STOPD_RED,FONT_NORMAL,0.6,"X");/*PuzzleWireLH: PuzzleWire.cs*/
        }
        if(data==4){/*WirePuzzle*/
            RenderUIImage(82+dx,570,139,192,2143);/*ContainerCenter UNMAPPED:[Textures/UI/puzzle/wire_center.png]*/
            RenderUIImage(34+dx,521,235,44,2144);/*LevelsBox UNMAPPED:[Textures/UI/puzzle/wire_levelsbox.png]*/
            RenderUIImage(40+dx,526,235,34,0);/*Background QUAD:builtin-knob*/
            RenderUIImage(43+dx,526,6,34,2142);/*Fill UNMAPPED:[Textures/UI/puzzle/puzzlesliderwire.png*/
            RenderUIImage(40+dx,509,22,69,1078);/*Handle*/
            RenderUIImage(204+dx,522,66,42,2145);/*TargetLine UNMAPPED:[Textures/UI/puzzle/wire_levelstargetlin*/
            RenderUIImage(57+dx,566,26,29,2146);/*NodeBase UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase: PuzzleWireLH.ClickLHNode() UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(61+dx,572,16,16,0);/*SelectedIndicator QUAD:none*/
            RenderUIImage(58+dx,569,22,22,0);/*GeniusHint QUAD:none*/
            RenderUIImage(57+dx,594,26,29,2146);/*NodeBase (1) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (1): PuzzleWireLH.ClickLHNode(1) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(61+dx,600,16,16,0);/*SelectedIndicator (1) QUAD:none*/
            RenderUIImage(58+dx,597,22,22,0);/*GeniusHint (1) QUAD:none*/
            RenderUIImage(57+dx,623,26,29,2146);/*NodeBase (2) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (2): PuzzleWireLH.ClickLHNode(2) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(61+dx,629,16,16,0);/*SelectedIndicator (2) QUAD:none*/
            RenderUIImage(58+dx,626,22,22,0);/*GeniusHint (2) QUAD:none*/
            RenderUIImage(57+dx,651,26,29,2146);/*NodeBase (3) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (3): PuzzleWireLH.ClickLHNode(3) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(61+dx,657,16,16,0);/*SelectedIndicator (3) QUAD:none*/
            RenderUIImage(58+dx,654,22,22,0);/*GeniusHint (3) QUAD:none*/
            RenderUIImage(57+dx,679,26,29,2146);/*NodeBase (4) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (4): PuzzleWireLH.ClickLHNode(4) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(61+dx,685,16,16,0);/*SelectedIndicator (4) QUAD:none*/
            RenderUIImage(58+dx,682,22,22,0);/*GeniusHint (4) QUAD:none*/
            RenderUIImage(57+dx,707,26,29,2146);/*NodeBase (5) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (5): PuzzleWireLH.ClickLHNode(5) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(61+dx,713,16,16,0);/*SelectedIndicator (5) QUAD:none*/
            RenderUIImage(58+dx,710,22,22,0);/*GeniusHint (5) QUAD:none*/
            RenderUIImage(57+dx,737,26,29,2146);/*NodeBase (6) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (6): PuzzleWireLH.ClickLHNode(6) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(61+dx,743,16,16,0);/*SelectedIndicator (6) QUAD:none*/
            RenderUIImage(58+dx,740,22,22,0);/*GeniusHint (6) QUAD:none*/
            RenderUIImage(222+dx,566,26,29,2146);/*NodeBase UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase: PuzzleWireLH.ClickRHNode() UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(227+dx,572,16,16,0);/*SelectedIndicator QUAD:none*/
            RenderUIImage(223+dx,569,22,22,0);/*GeniusHint QUAD:none*/
            RenderUIImage(222+dx,594,26,29,2146);/*NodeBase (1) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (1): PuzzleWireLH.ClickRHNode(1) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(227+dx,600,16,16,0);/*SelectedIndicator (1) QUAD:none*/
            RenderUIImage(223+dx,597,22,22,0);/*GeniusHint (1) QUAD:none*/
            RenderUIImage(222+dx,623,26,29,2146);/*NodeBase (2) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (2): PuzzleWireLH.ClickRHNode(2) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(227+dx,629,16,16,0);/*SelectedIndicator (2) QUAD:none*/
            RenderUIImage(223+dx,626,22,22,0);/*GeniusHint (2) QUAD:none*/
            RenderUIImage(222+dx,651,26,29,2146);/*NodeBase (3) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (3): PuzzleWireLH.ClickRHNode(3) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(227+dx,657,16,16,0);/*SelectedIndicator (3) QUAD:none*/
            RenderUIImage(223+dx,654,22,22,0);/*GeniusHint (3) QUAD:none*/
            RenderUIImage(222+dx,679,26,29,2146);/*NodeBase (4) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (4): PuzzleWireLH.ClickRHNode(4) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(227+dx,685,16,16,0);/*SelectedIndicator (4) QUAD:none*/
            RenderUIImage(223+dx,682,22,22,0);/*GeniusHint (4) QUAD:none*/
            RenderUIImage(222+dx,707,26,29,2146);/*NodeBase (5) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (5): PuzzleWireLH.ClickRHNode(5) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(227+dx,713,16,16,0);/*SelectedIndicator (5) QUAD:none*/
            RenderUIImage(223+dx,710,22,22,0);/*GeniusHint (5) QUAD:none*/
            RenderUIImage(222+dx,736,26,29,2146);/*NodeBase (6) UNMAPPED:[Textures/UI/puzzle/wire_node.png] BTN NodeBase (6): PuzzleWireLH.ClickRHNode(6) UIButtonMask.cs,PuzzleUIButton.cs*/
            RenderUIImage(227+dx,743,16,16,0);/*SelectedIndicator (6) QUAD:none*/
            RenderUIImage(223+dx,740,22,22,0);/*GeniusHint (6) QUAD:none*/
            RenderUIImage(259+dx,736,29,29,899);/*CloseButton BTN CloseButton: MFDManager.ClosePuzzleWire() UIButtonMask.cs*/
            RenderTextL(259+dx,736,T_STOPD_RED,FONT_NORMAL,0.6,"X");/*SystemAnalyzerDisplayLH: SystemAnalyzer.cs*/
        }
        if(data==7){/*SysAnalyzer*/
            RenderTextL(24+dx,523,T_YELLOW,FONT_NORMAL,0.6,"%s",892<1100?Sys_Text.stringTable[892]:"SYSTEM ANALYZER");/*Header UIPointerMask.cs*/
            RenderTextL(24+dx,547,T_GREEN,FONT_NORMAL,0.6,"Current level security:");/*DescriptionLevelSecurity UIPointerMask.cs*/
            RenderTextL(180+dx,547,T_GREEN,FONT_NORMAL,0.6,"100%%");/*TextLevelSecurity UIPointerMask.cs*/
            RenderTextL(24+dx,566,T_GREEN,FONT_NORMAL,0.6,"Mining laser status:");/*DescriptionMiningLaser UIPointerMask.cs*/
            RenderTextL(180+dx,566,T_GREEN,FONT_NORMAL,0.6,"Charging");/*TextLaserStatus UIPointerMask.cs*/
            RenderTextL(24+dx,585,T_GREEN,FONT_NORMAL,0.6,"Lifepod status:");/*DescriptionLifepods UIPointerMask.cs*/
            RenderTextL(180+dx,585,T_GREEN,FONT_NORMAL,0.6,"Disabled");/*TextLifepodStatus UIPointerMask.cs*/
            RenderTextL(24+dx,605,T_GREEN,FONT_NORMAL,0.6,"Station shield status:");/*DescriptionShield UIPointerMask.cs*/
            RenderTextL(180+dx,605,T_GREEN,FONT_NORMAL,0.6,"Off");/*TextShieldStatus UIPointerMask.cs*/
            RenderTextL(24+dx,624,T_GREEN,FONT_NORMAL,0.6,"Reactor status:");/*DescriptionReactor UIPointerMask.cs*/
            RenderTextL(180+dx,624,T_GREEN,FONT_NORMAL,0.6,"Normal");/*TextReactorStatus UIPointerMask.cs*/
            RenderTextL(24+dx,643,T_GREEN,FONT_NORMAL,0.6,"Processor nodes:");/*DescriptionProcessors UIPointerMask.cs*/
            RenderTextL(180+dx,643,T_GREEN,FONT_NORMAL,0.6,"99");/*TextProcessors UIPointerMask.cs*/
            RenderTextL(24+dx,662,T_GREEN,FONT_NORMAL,0.6,"Main Program:");/*DescriptionMainProgram UIPointerMask.cs*/
            RenderTextL(179+dx,662,T_GREEN,FONT_NORMAL,0.6,"Downloading to earth");/*TextMainProgram UIPointerMask.cs*/
            RenderTextL(24+dx,681,T_GREEN,FONT_NORMAL,0.6,"Alpha Grove status:");/*DescriptionGroveAlphaStatus UIPointerMask.cs*/
            RenderTextL(180+dx,681,T_GREEN,FONT_NORMAL,0.6,"normal");/*TextGroveAlpha UIPointerMask.cs*/
            RenderTextL(24+dx,701,T_GREEN,FONT_NORMAL,0.6,"Beta Grove status:");/*DescriptionGroveBetaStatus UIPointerMask.cs*/
            RenderTextL(180+dx,701,T_GREEN,FONT_NORMAL,0.6,"normal");/*TextGroveBeta UIPointerMask.cs*/
            RenderTextL(24+dx,720,T_GREEN,FONT_NORMAL,0.6,"Gamma Grove status:");/*DescriptionGroveGammaStatus UIPointerMask.cs*/
            RenderTextL(180+dx,720,T_GREEN,FONT_NORMAL,0.6,"launched");/*TextGroveGamma UIPointerMask.cs*/
            RenderTextL(24+dx,739,T_GREEN,FONT_NORMAL,0.6,"Delta Grove status:");/*DescriptionGroveDeltaStatus UIPointerMask.cs*/
            RenderTextL(180+dx,739,T_GREEN,FONT_NORMAL,0.6,"launched");/*TextGroveDelta UIPointerMask.cs*/
            RenderUIImage(259+dx,527,29,29,899);/*CloseButton BTN CloseButton: SystemAnalyzerDisplayLH.Close() UIButtonMask.cs*/
            RenderTextL(259+dx,527,T_STOPD_RED,FONT_NORMAL,0.6,"X");
        }
        if(data==9){/*Minigames*/
            RenderUIImage(21+dx,501,262,262,1025);/*MinigamesContainer UIPointerMask.cs*/
            RenderTextL(28+dx,503,T_RED,FONT_NORMAL,0.6,"TRIOPTIMUM FUNPACK");/*Header*/
            RenderUIImage(32+dx,540,115,24,0);/*MiniGameButton0_Ping QUAD:builtin-white BTN MiniGameButton0_Ping: MFDManager.MinigameStart_Ping() UIButtonMask.cs*/
            RenderTextL(37+dx,541,T_GREEN,FONT_NORMAL,0.6,"Ping");
            RenderUIImage(32+dx,575,115,24,0);/*MiniGameButton1_15 QUAD:builtin-white BTN MiniGameButton1_15: MFDManager.MinigameStart_15() UIButtonMask.cs*/
            RenderTextL(37+dx,577,T_GREEN,FONT_NORMAL,0.6,"15");
            RenderUIImage(32+dx,610,115,24,0);/*MiniGameButton2_Wing0 QUAD:builtin-white BTN MiniGameButton2_Wing0: MFDManager.MinigameStart_Wing0() UIButtonMask.cs*/
            RenderTextL(37+dx,612,T_GREEN,FONT_NORMAL,0.6,"Wing 0");
            RenderUIImage(32+dx,646,115,24,0);/*MiniGameButton3_Botbounce QUAD:builtin-white BTN MiniGameButton3_Botbounce: MFDManager.MinigameStart_Botbounce() UIButtonMask.cs*/
            RenderTextL(37+dx,647,T_GREEN,FONT_NORMAL,0.6,"Botbounce");
            RenderUIImage(156+dx,540,115,24,0);/*MiniGameButton4_EelZapper QUAD:builtin-white BTN MiniGameButton4_EelZapper: MFDManager.MinigameStart_EelZapper() UIButtonMask.cs*/
            RenderTextL(161+dx,541,T_GREEN,FONT_NORMAL,0.6,"Eel Zapper");
            RenderUIImage(156+dx,575,115,24,0);/*MiniGameButton5_Road QUAD:builtin-white BTN MiniGameButton5_Road: MFDManager.MinigameStart_Road() UIButtonMask.cs*/
            RenderTextL(161+dx,577,T_GREEN,FONT_NORMAL,0.6,"Road");
            RenderUIImage(156+dx,610,115,24,0);/*MiniGameButton6_TriopToe QUAD:builtin-white BTN MiniGameButton6_TriopToe: MFDManager.MinigameStart_TriopToe() UIButtonMask.cs*/
            RenderTextL(161+dx,612,T_GREEN,FONT_NORMAL,0.6,"TriopToe");
            RenderUIImage(156+dx,646,115,24,0);/*MiniGameButton7_CorporateConquer QUAD:builtin-white BTN MiniGameButton7_CorporateConquer: MFDManager.MinigameStart_CorporateConquer() UIButtonMask.cs*/
            RenderTextL(161+dx,647,T_GREEN,FONT_NORMAL,0.6,"Corp Conq");
            RenderUIImage(32+dx,681,115,24,0);/*MiniGameButton8_Chess QUAD:builtin-white BTN MiniGameButton8_Chess: MFDManager.MinigameStart_Chess() UIButtonMask.cs*/
            RenderTextL(37+dx,682,T_GREEN,FONT_NORMAL,0.6,"Chess");
            RenderTextL(97+dx,726,T_RED,FONT_NORMAL,0.6,"Don't Play on\n\nCompany Time");/*Footer*/
            RenderUIImage(261+dx,504,19,19,0);/*MinigameClose QUAD:none BTN MinigameClose: MFDManager.TabReset() UIButtonMask.cs*/
            RenderUIImage(259+dx,502,22,22,899);/*Border*/
            RenderUIImage(21+dx,501,262,262,0);/*MinigameView QUAD:none UIPointerMask.cs*/
            RenderUIImage(21+dx,501,262,262,0);/*PingGameOver QUAD:builtin-white BTN PingGameOver: Ping.ResetOnGameOver()|Fifteen.Reset()*/
            RenderTextL(30+dx,545,T_WHITE,FONT_NORMAL,0.6,"PUZZLE SOLVED!");/*gameOverText*/
            RenderTextL(91+dx,710,T_WHITE,FONT_NORMAL,0.6,"YOU LOSE");/*winText*/
            RenderUIImage(261+dx,504,19,19,0);/*MinigameBack QUAD:none BTN MinigameBack: MFDManager.OpenMinigames() UIButtonMask.cs*/
            RenderUIImage(259+dx,502,22,22,899);/*Border*/
        }
    }
}

void CenterMFD() {
    if (Cheats.noHUD) return;
    CenterMFDHeader();
    if(World.Sys_UI.MFD_CenterTab==1){/*Main*/
        RenderConsumables();
    }
    if(World.Sys_UI.MFD_CenterTab==2){/*Hardware*/
        RenderTextL(454,557,T_RED,FONT_NORMAL,0.8f,"%s",874<1100?Sys_Text.stringTable[874]:"HARDWARE");
        for (int i=0;i<HW_COUNT;++i) { int ref=World.invP1.hardwareInvReferenceIndex[i]; if (ref<0 || World.invP1.hwVers[i] <= 0) continue;
            int row=i/6; int col=i%6; i16 x=458+col*223, y=575+row*23;
            const char* label=Sys_Text.stringTable[ref+326]; float w=MeasureLineAdvance(label,FONT_NORMAL); float sc=w>0?vmin(0.6f,210.0f/w):0.6f;
            RenderTextL(x,y,World.invP1.hardwareInvCurrent==i?T_YELLOW:(World.invP1.hasHardware&(1u<<i)?T_GREEN_MENU:T_GREEN_MENU_SHADOW),FONT_NORMAL,sc,"%s",label);
            RenderTextL(x+195,y,World.invP1.hardwareInvCurrent==i?T_YELLOW:T_GREEN_MENU,FONT_NORMAL,0.5f,"v%d",(int)World.invP1.hwVers[i]);
        }
    }
    if (World.Sys_UI.MFD_CenterTab==3) RenderGeneralInventory();
    static const char* swLabels[7]={"ICE DRILL","PULSER/DRILL","SHIELD","TURBO","DECOY","RECALL","GAMES"};
    static const u8 swVersions[3]={0,1,2};
    if(World.Sys_UI.MFD_CenterTab==4){/*Software*/
        RenderTextL(454,557,T_RED,FONT_NORMAL,0.8f,"%s",876<1100?Sys_Text.stringTable[876]:"SOFTWARE");
        for (int i=0;i<7;++i) { bool owned=false; int ver=0; int count=0;
            if (i<=2) { bool owned=World.invP1.hasSoft&(1u<<(i+3)); if (!owned) continue; } else if (i>=3 && i<=5) { int count=World.invP1.softVersions[i]; bool owned=count>0 || (World.invP1.hasSoft&(1u<<(i+3)))!=0; if (!owned) continue; count=(count>0?count:0); } else { bool owned=World.invP1.hasMinigame; if (!owned) continue; }
            int y=588+i*32; const char* label=swLabels[i]; bool selected=i==World.invP1.cyberItemIndex; float sc=0.6f; float w=MeasureLineAdvance(label,FONT_NORMAL); sc=w>0?vmin(sc,210.0f/w):sc; RenderTextL(454,y,selected?T_YELLOW:(owned?T_GREEN_MENU:T_GREEN_MENU_SHADOW),FONT_NORMAL,sc,"%s",label);
            if (i<=2) RenderTextL(680,y,selected?T_YELLOW:T_GREEN_MENU,FONT_NORMAL,0.5f,"v%d",World.invP1.softVersions[i]+1); else if (i<=5) RenderTextL(680,y,selected?T_YELLOW:T_GREEN_MENU,FONT_NORMAL,0.5f,"x%d",(World.invP1.softVersions[i]>0?World.invP1.softVersions[i]:0)); else RenderTextL(680,y,selected?T_YELLOW:T_GREEN_MENU,FONT_NORMAL,0.5f,"%d",World.invP1.hasMinigame?1:0);
        } 
    }
    if(World.Sys_UI.MFD_CenterTab==5){/*EReader*/
        RenderTextL(454,557,T_RED,FONT_NORMAL,0.6,"%s",877<1100?Sys_Text.stringTable[877]:"LOGS");/*MultiMediaHeaderLabel UIPointerMask.cs*/
        if(World.Sys_UI.MFD_MediaTab==MM_LOG_TABLE){/*LogTable*/
            if(World.Sys_UI.MFD_ReaderView==MFD_READER_CONTENTS){
                RenderUIImage(454,573,453,191,0);/*LogTableofContents QUAD:builtin-knob LogTableContentsButtonsManager.cs*/
                RenderUIImage(454,573,226,24,0);/*Button QUAD:builtin-white BTN Button: LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs*/
                RenderTextL(454,573,T_GREEN,FONT_NORMAL,0.6,"Level R Logs");
                RenderTextL(531,573,T_GREEN,FONT_NORMAL,0.6,"3");/*CountText LogCountsText.cs*/
                RenderUIImage(454,597,226,24,0);/*Button (1) QUAD:builtin-white BTN Button (1): LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs*/
                RenderTextL(454,597,T_GREEN,FONT_NORMAL,0.6,"Level 1 Logs");
                RenderTextL(531,597,T_GREEN,FONT_NORMAL,0.6,"3");/*CountText (1) LogCountsText.cs*/
                RenderUIImage(454,620,226,24,0);/*Button (2) QUAD:builtin-white BTN Button (2): LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs*/
                RenderTextL(454,620,T_GREEN,FONT_NORMAL,0.6,"Level 2 Logs");
                RenderTextL(531,620,T_GREEN,FONT_NORMAL,0.6,"3");/*CountText (2) LogCountsText.cs*/
                RenderUIImage(454,644,226,24,0);/*Button (3) QUAD:builtin-white BTN Button (3): LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs*/
                RenderTextL(454,644,T_GREEN,FONT_NORMAL,0.6,"Level 3 Logs");
                RenderTextL(531,644,T_GREEN,FONT_NORMAL,0.6,"3");/*CountText (3) LogCountsText.cs*/
                RenderUIImage(454,667,226,24,0);/*Button (4) QUAD:builtin-white BTN Button (4): LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs*/
                RenderTextL(454,667,T_GREEN,FONT_NORMAL,0.6,"Level 4 Logs");
                RenderTextL(531,667,T_GREEN,FONT_NORMAL,0.6,"3");/*CountText (4) LogCountsText.cs*/
                RenderUIImage(454,691,226,24,0);/*Button (5) QUAD:builtin-white BTN Button (5): LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs*/
                RenderTextL(454,691,T_GREEN,FONT_NORMAL,0.6,"Level 5 Logs");
                RenderTextL(531,691,T_GREEN,FONT_NORMAL,0.6,"3");/*CountText (5) LogCountsText.cs*/
                RenderUIImage(454,714,226,24,0);/*Button (6) QUAD:builtin-white BTN Button (6): LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs*/
                RenderTextL(454,714,T_GREEN,FONT_NORMAL,0.6,"Level 6 Logs");
                RenderTextL(531,714,T_GREEN,FONT_NORMAL,0.6,"3");/*CountText (6) LogCountsText.cs*/
                RenderUIImage(681,573,226,24,0);/*Button (7) QUAD:builtin-white BTN Button (7): LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs*/
                RenderTextL(681,573,T_GREEN,FONT_NORMAL,0.6,"Level 7 Logs");
                RenderTextL(751,573,T_GREEN,FONT_NORMAL,0.6,"3");/*CountText (7) LogCountsText.cs*/
                RenderUIImage(681,597,226,24,0);/*Button (8) QUAD:builtin-white BTN Button (8): LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs*/
                RenderTextL(681,597,T_GREEN,FONT_NORMAL,0.6,"Level 8 Logs");
                RenderTextL(751,597,T_GREEN,FONT_NORMAL,0.6,"3");/*CountText (8) LogCountsText.cs*/
                RenderUIImage(681,620,226,24,0);/*Button (9) QUAD:builtin-white BTN Button (9): LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs*/
                RenderTextL(681,620,T_GREEN,FONT_NORMAL,0.6,"Level 9 Logs");
                RenderTextL(751,620,T_GREEN,FONT_NORMAL,0.6,"3");/*CountText (9) LogCountsText.cs*/
            }else if(World.Sys_UI.MFD_ReaderView==MFD_READER_FOLDER){
                RenderUIImage(458,570,445,188,0);/*LogsLevelFolder QUAD:builtin-knob LogContentsButtonsManager.cs*/
                RenderUIImage(458,570,222,21,0);/*Button QUAD:builtin-white BTN Button: LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(458,570,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text0*/
                RenderUIImage(458,591,222,21,0);/*Button (1) QUAD:builtin-white BTN Button (1): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(458,591,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text1*/
                RenderUIImage(458,612,222,21,0);/*Button (2) QUAD:builtin-white BTN Button (2): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(458,612,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text2*/
                RenderUIImage(458,633,222,21,0);/*Button (3) QUAD:builtin-white BTN Button (3): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(458,633,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text3*/
                RenderUIImage(458,654,222,21,0);/*Button (4) QUAD:builtin-white BTN Button (4): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(458,654,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text4*/
                RenderUIImage(458,675,222,21,0);/*Button (5) QUAD:builtin-white BTN Button (5): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(458,675,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text5*/
                RenderUIImage(458,696,222,21,0);/*Button (6) QUAD:builtin-white BTN Button (6): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(458,696,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text6*/
                RenderUIImage(458,717,222,21,0);/*Button (7) QUAD:builtin-white BTN Button (7): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(458,717,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text7*/
                RenderUIImage(681,570,222,21,0);/*Button (8) QUAD:builtin-white BTN Button (8): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(681,570,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text8*/
                RenderUIImage(681,591,222,21,0);/*Button (9) QUAD:builtin-white BTN Button (9): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(681,591,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text9*/
                RenderUIImage(681,612,222,21,0);/*Button (10) QUAD:builtin-white BTN Button (10): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(681,612,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text10*/
                RenderUIImage(681,633,222,21,0);/*Button (11) QUAD:builtin-white BTN Button (11): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(681,633,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text11*/
                RenderUIImage(681,654,222,21,0);/*Button (12) QUAD:builtin-white BTN Button (12): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(681,654,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text12*/
                RenderUIImage(681,675,222,21,0);/*Button (13) QUAD:builtin-white BTN Button (13): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(681,675,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text13*/
                RenderUIImage(681,696,222,21,0);/*Button (14) QUAD:builtin-white BTN Button (14): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
                RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Log");/*Text14*/
            }else if(World.Sys_UI.MFD_ReaderView==MFD_READER_TEXT){
                /*LogTextReader: LogTextReaderManager.cs*/
                RenderTextL(449,576,T_GREEN,FONT_NORMAL,0.6,"\"abc def ghi jkl mno pqrs tuv wxyz ABC DEF GHI JKL MNO PQRS TUV WXYZ !\"\\xA7\n$%%& /() =?* '<> #|; \\xB2\\xB3~ @`\\xB4 \\xA9\\xAB\\xBB \\xA4\\xBC\\x...");/*LogTextOutput UIPointerMask.cs*/
                RenderUIImage(454,576,456,174,0);/*MoreButton QUAD:builtin-white BTN MoreButton: LogMoreButtonClick() UIButtonMask.cs,LogMoreButton.cs*/
                RenderTextL(654,647,T_YELLOW,FONT_NORMAL,0.6,"%s",26<1100?Sys_Text.stringTable[26]:"[MORE]");/*Text0*/
                RenderUIImage(453,718,69,31,0);/*BackButton QUAD:builtin-white BTN BackButton: LogBackButtonClick() UIButtonMask.cs,LogBackButton.cs*/
                RenderTextL(453,718,T_YELLOW,FONT_NORMAL,0.6,"%s",879<1100?Sys_Text.stringTable[879]:"[BACK]");/*Text0*/
            }
        }
        if(World.Sys_UI.MFD_MediaTab==MM_EMAIL_TABLE){/*Email*/
            RenderUIImage(458,570,445,188,0);/*EmailTab QUAD:builtin-knob EmailContentsButtonsManager.cs*/
            RenderUIImage(458,570,223,21,0);/*Button QUAD:builtin-white BTN Button: LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,570,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text0*/
            RenderUIImage(458,591,223,21,0);/*Button (1) QUAD:builtin-white BTN Button (1): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,591,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text1*/
            RenderUIImage(458,612,223,21,0);/*Button (2) QUAD:builtin-white BTN Button (2): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,612,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text2*/
            RenderUIImage(458,633,223,21,0);/*Button (3) QUAD:builtin-white BTN Button (3): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,633,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text3*/
            RenderUIImage(458,654,223,21,0);/*Button (4) QUAD:builtin-white BTN Button (4): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,654,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text4*/
            RenderUIImage(458,675,223,21,0);/*Button (5) QUAD:builtin-white BTN Button (5): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,675,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text5*/
            RenderUIImage(458,696,223,21,0);/*Button (6) QUAD:builtin-white BTN Button (6): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text6*/
            RenderUIImage(458,717,223,21,0);/*Button (7) QUAD:builtin-white BTN Button (7): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,717,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text7*/
            RenderUIImage(681,570,223,21,0);/*Button (8) QUAD:builtin-white BTN Button (8): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,570,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text8*/
            RenderUIImage(681,591,223,21,0);/*Button (9) QUAD:builtin-white BTN Button (9): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,591,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text9*/
            RenderUIImage(681,612,223,21,0);/*Button (10) QUAD:builtin-white BTN Button (10): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,612,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text10*/
            RenderUIImage(681,633,223,21,0);/*Button (11) QUAD:builtin-white BTN Button (11): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,633,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text11*/
            RenderUIImage(681,654,223,21,0);/*Button (12) QUAD:builtin-white BTN Button (12): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,654,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text12*/
            RenderUIImage(681,675,223,21,0);/*Button (13) QUAD:builtin-white BTN Button (13): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,675,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text13*/
            RenderUIImage(681,696,223,21,0);/*Button (14) QUAD:builtin-white BTN Button (14): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (15) QUAD:builtin-white BTN Button (15): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (16) QUAD:builtin-white BTN Button (16): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (17) QUAD:builtin-white BTN Button (17): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (18) QUAD:builtin-white BTN Button (18): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (19) QUAD:builtin-white BTN Button (19): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (20) QUAD:builtin-white BTN Button (20): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (21) QUAD:builtin-white BTN Button (21): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (22) QUAD:builtin-white BTN Button (22): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (23) QUAD:builtin-white BTN Button (23): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (24) QUAD:builtin-white BTN Button (24): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
            RenderUIImage(681,696,223,21,0);/*Button (25) QUAD:builtin-white BTN Button (25): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,696,T_GREEN,FONT_NORMAL,0.6,"Email");/*Text14*/
        }
        if(World.Sys_UI.MFD_MediaTab==MM_DATA_TABLE){/*DataTab*/
            RenderUIImage(458,570,445,188,0);/*DataTab QUAD:builtin-knob EmailContentsButtonsManager.cs*/
            RenderUIImage(458,570,223,21,0);/*Button QUAD:builtin-white BTN Button: LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,570,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text0*/
            RenderUIImage(458,591,223,21,0);/*Button (1) QUAD:builtin-white BTN Button (1): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,591,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text1*/
            RenderUIImage(458,612,223,21,0);/*Button (2) QUAD:builtin-white BTN Button (2): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,612,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text2*/
            RenderUIImage(458,633,223,21,0);/*Button (3) QUAD:builtin-white BTN Button (3): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,633,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text3*/
            RenderUIImage(458,654,223,21,0);/*Button (4) QUAD:builtin-white BTN Button (4): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,654,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text4*/
            RenderUIImage(458,675,223,21,0);/*Button (5) QUAD:builtin-white BTN Button (5): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,675,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text5*/
            RenderUIImage(458,696,223,21,0);/*Button (6) QUAD:builtin-white BTN Button (6): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,696,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text6*/
            RenderUIImage(458,717,223,21,0);/*Button (7) QUAD:builtin-white BTN Button (7): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(458,717,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text7*/
            RenderUIImage(681,570,223,21,0);/*Button (8) QUAD:builtin-white BTN Button (8): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,570,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text8*/
            RenderUIImage(681,591,223,21,0);/*Button (9) QUAD:builtin-white BTN Button (9): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,591,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text9*/
            RenderUIImage(681,612,223,21,0);/*Button (10) QUAD:builtin-white BTN Button (10): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,612,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text10*/
            RenderUIImage(681,633,223,21,0);/*Button (11) QUAD:builtin-white BTN Button (11): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,633,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text11*/
            RenderUIImage(681,654,223,21,0);/*Button (12) QUAD:builtin-white BTN Button (12): LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs*/
            RenderTextL(681,654,T_GREEN,FONT_NORMAL,0.6,"Data");/*Text12*/
        }
        if(World.Sys_UI.MFD_MediaTab==MM_NOTES){/*Notes*/
            RenderUIImage(453,570,166,39,0);/*NoteToggle QUAD:none UIPointerMask.cs*/
            RenderUIImage(453,572,19,18,910);/*Background*/
            RenderTextL(474,573,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],1,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev1SecCode);/*Label*/
            RenderUIImage(453,599,166,39,0);/*NoteToggle1 QUAD:none UIPointerMask.cs*/
            RenderUIImage(453,600,19,18,910);/*Background*/
            RenderTextL(474,602,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],2,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev2SecCode);/*Label1*/
            RenderUIImage(453,628,166,39,0);/*NoteToggle2 QUAD:none UIPointerMask.cs*/
            RenderUIImage(453,629,19,18,910);/*Background*/
            RenderTextL(474,631,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],3,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev3SecCode);/*Label2*/
            RenderUIImage(453,657,166,39,0);/*NoteToggle3 QUAD:none UIPointerMask.cs*/
            RenderUIImage(453,658,19,18,910);/*Background*/
            RenderTextL(474,660,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],4,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev4SecCode);/*Label3*/
            RenderUIImage(453,686,166,39,0);/*NoteToggle4 QUAD:none UIPointerMask.cs*/
            RenderUIImage(453,687,19,18,910);/*Background*/
            RenderTextL(474,689,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],5,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev5SecCode);/*Label4*/
            RenderUIImage(453,715,166,39,0);/*NoteToggle5 QUAD:none UIPointerMask.cs*/
            RenderUIImage(453,716,19,18,910);/*Background*/
            RenderTextL(474,718,T_GREEN,FONT_NORMAL,0.6,"%s%d%s%s%u.",Sys_Text.stringTable[556],6,Sys_Text.stringTable[557],Sys_Text.stringTable[558],World.lev6SecCode);/*Label5*/
            RenderUIImage(620,570,166,39,0);/*NoteToggle6 QUAD:none UIPointerMask.cs*/
            RenderUIImage(620,572,19,18,910);/*Background*/
            RenderTextL(641,573,T_GREEN,FONT_NORMAL,0.6,"Escape neurosurgery suite.  Keycode is 451.");/*Label6*/
            RenderUIImage(620,599,166,39,0);/*NoteToggle7 QUAD:none UIPointerMask.cs*/
            RenderUIImage(620,600,19,18,910);/*Background*/
            RenderTextL(641,602,T_GREEN,FONT_NORMAL,0.6,"Disengage laser safety override.");/*Label7*/
            RenderUIImage(620,628,166,39,0);/*NoteToggle8 QUAD:none UIPointerMask.cs*/
            RenderUIImage(620,629,19,18,910);/*Background*/
            RenderTextL(641,631,T_GREEN,FONT_NORMAL,0.6,"Activate the station energy shield.");/*Label8*/
            RenderUIImage(620,657,166,39,0);/*NoteToggle9 QUAD:none UIPointerMask.cs*/
            RenderUIImage(620,658,19,18,910);/*Background*/
            RenderTextL(641,660,T_GREEN,FONT_NORMAL,0.6,"Destroy the mining laser.");/*Label9*/
            RenderUIImage(620,686,166,39,0);/*NoteToggle10 QUAD:none UIPointerMask.cs*/
            RenderUIImage(620,687,19,18,910);/*Background*/
            RenderTextL(641,689,T_GREEN,FONT_NORMAL,0.6,"Enable master jettison.");/*Label10*/
            RenderUIImage(620,715,166,39,0);/*NoteToggle11 QUAD:none UIPointerMask.cs*/
            RenderUIImage(620,716,19,18,910);/*Background*/
            RenderTextL(641,718,T_GREEN,FONT_NORMAL,0.6,"Diagnose and repair broken relay: 428.");/*Label11*/
            RenderUIImage(787,570,166,39,0);/*NoteToggle12 QUAD:none UIPointerMask.cs*/
            RenderUIImage(787,572,19,18,910);/*Background*/
            RenderTextL(808,573,T_GREEN,FONT_NORMAL,0.6,"Jettison Beta Grove.");/*Label12*/
            RenderUIImage(787,599,166,39,0);/*NoteToggle13 QUAD:none UIPointerMask.cs*/
            RenderUIImage(787,600,19,18,910);/*Background*/
            RenderTextL(808,602,T_GREEN,FONT_NORMAL,0.6,"Destroy the four relay antennae.");/*Label13*/
            RenderUIImage(787,628,166,39,0);/*NoteToggle14 QUAD:none UIPointerMask.cs*/
            RenderUIImage(787,629,19,18,910);/*Background*/
            RenderTextL(808,631,T_GREEN,FONT_NORMAL,0.6,"Engage reactor self-destruct.");/*Label14*/
            RenderUIImage(787,657,166,39,0);/*NoteToggle15 QUAD:none UIPointerMask.cs*/
            RenderUIImage(787,658,19,18,910);/*Background*/
            RenderTextL(808,660,T_GREEN,FONT_NORMAL,0.6,"Escape on escape pod.");/*Label15*/
            RenderUIImage(787,686,166,39,0);/*NoteToggle16 QUAD:none UIPointerMask.cs*/
            RenderUIImage(787,687,19,18,910);/*Background*/
            RenderTextL(808,689,T_GREEN,FONT_NORMAL,0.6,"Access the bridge.");/*Label16*/
            RenderUIImage(787,715,166,39,0);/*NoteToggle17 QUAD:none UIPointerMask.cs*/
            RenderUIImage(787,716,19,18,910);/*Background*/
            RenderTextL(808,718,T_GREEN,FONT_NORMAL,0.6,"Destroy SHODAN.");/*Label17*/
        }
    }
}


static double RenderUI() {
    drawCallsNormal=drawCalls; World.uiIsBlocking=false;
    if (!EditSelIsActive()) editFieldEditing=false;
    if (World.creditsActive) {
        if (Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed) { ++World.creditsPageIndex; if (World.creditsPageIndex>CREDITS_PAGES) { World.creditsActive=false; return get_time(); } }
        if (World.creditsPageIndex==1) { CreditsStats(); RenderTextL(300,10,T_WHITE,FONT_NORMAL,1.0f,(const char*)&creditStats); }
        else RenderTextL(300,10,T_WHITE,FONT_NORMAL,1.0f,creditPages[World.creditsPageIndex]);
        return get_time();
    }
    if (World.menuActive) RenderMenu(); else if (World.paused) RenderPausedUI();
    if (World.menuActive || World.paused) {
        if (Sys_Input.keyStates[KEY_DOWN].pressed) currentMenuItem=(currentMenuItem+1)>=menuItemCount?0:currentMenuItem+1;
        else if (Sys_Input.keyStates[KEY_UP].pressed) currentMenuItem=(currentMenuItem-1)<0?menuItemCount-1:currentMenuItem-1;
    } else if (!World.Sys_UI.vmailActive) {
        if (!Cheats.noHUD) {
            TickBar(false); TickBar(true); HardwareButtons(); RenderUIImage(667,0,32,32,1020);
            for (u8 i=0;i<10;++i) if (World.Sys_UI.tWrnFinished[i]>World.pauseRelativeTime) {
                char flt[6]; if (World.Sys_UI.tWrnTextIdx[i]==185) sFormat(flt,6,"%.1f",(double)World.instances[PLAYER1].radiation);
                RenderTextL(340,72+i*18,World.Sys_UI.tWrnColorIdx[i],FONT_NORMAL,0.8f,"%s%s%s",Sys_Text.stringTable[World.Sys_UI.tWrnTextIdx[i]],World.Sys_UI.tWrnTextIdx[i]==185?flt:World.Sys_UI.tWrnTextIdx2[i]>=0?Sys_Text.stringTable[World.Sys_UI.tWrnTextIdx2[i]]:"",World.Sys_UI.tWrnTextIdx3[i]>=0?Sys_Text.stringTable[World.Sys_UI.tWrnTextIdx3[i]]:"");
            }
            SideMFDHeader(false); SideMFD(false); CenterMFD(); SideMFDHeader(true); SideMFD(true); 
            if(World.diffMis>=3){RenderTextL(43,2,T_YELLOW,FONT_NORMAL,0.6,"%s",World.misTimerMission<1100?Sys_Text.stringTable[World.misTimerMission]:"");/*MissionTimerT*/ {char misT[8]; if(World.misTimerTimesUP) sFormat(misT,sizeof(misT),"%s",869<1100?Sys_Text.stringTable[869]:""); else {float mt=World.misTimerT<0.0f?0.0f:World.misTimerT; int mm=(int)(mt/60.0f),ss=(int)(mt-(float)(mm*60)); sFormat(misT,sizeof(misT),"%02d:%02d",mm,ss);} RenderTextL(258,2,T_YELLOW,FONT_NORMAL,0.6,"%s",misT);}/*MissionTimer*/}
            if (World.curLev==LEVEL_CYBERSPACE) { RenderTextL(28,530,T_WHITE,FONT_NORMAL,0.6,"T -"); RenderTextL(68,530,T_WHITE,FONT_NORMAL,0.6,"99:99"); }
        }
RenderTextL(1137,570,T_YELLOW,FONT_NORMAL,0.6,"level 1 elevator taken off line - SHODAN security block established 04.NOV.72");/*CyberSPrint UIPointerMask.cs,PooledItemDestroy.cs BioMonitorContainer: BioMonitor.cs BioMonitorContainer: BiomonitorGraphSystem.cs*/
if((World.invP1.hardwareIsActive & HW_BIO)!=0){/*BioMonitor*/
RenderUIImage(0,0,480,80,0);/*Graph QUAD:none*/
RenderTextL(4,83,T_YELLOW,FONT_NORMAL,0.6,"%s",895<1100?Sys_Text.stringTable[895]:"Biomonitor:");/*BiomonitorHeader*/
RenderTextL(4,99,T_GREEN,FONT_NORMAL,0.6,"%s",896<1100?Sys_Text.stringTable[896]:"Heart Rate:");/*BiomonitorTextHeart*/
RenderTextL(70,99,T_GREEN,FONT_NORMAL,0.6,"100");/*BiomonitorTextHeartRate*/
RenderTextL(122,99,T_GREEN,FONT_NORMAL,0.6,"BPM");/*BiomonitorTextBPM*/
RenderTextL(4,131,T_GREEN,FONT_NORMAL,0.6,"%s",897<1100?Sys_Text.stringTable[897]:"Patches Active:");/*BiomonitorTextPatch*/
RenderTextL(119,131,T_GREEN,FONT_NORMAL,0.6,"MEDI STAMINUP SIGHT GENIUS BERSERK REFLEX");/*BiomonitorTextPatchEffects*/
RenderTextL(4,115,T_GREEN,FONT_NORMAL,0.6,"%s",898<1100?Sys_Text.stringTable[898]:"Fatigue:");/*BiomonitorTextFatigueDetail*/
RenderTextL(66,115,T_GREEN,FONT_NORMAL,0.6,"Moderate");/*BiomonitorTextFatigue*/
}
RenderTextL(1270,78,T_WHITE,FONT_NORMAL,0.6,"0");
RenderTextL(1308,78,T_WHITE,FONT_NORMAL,0.6,"0");
RenderSearchFX();
        if (EditSelIsActive()) {/*Edit mode selection highlight + object info panel*/
            u16 sel=editModeSelection; Entity* e=&World.instances[sel];
            V3 f=World.instances[PLAYER1].forward,rt=World.instances[PLAYER1].right,ff=(V3){-f.x,-f.y,-f.z},up=V3_Normalize(V3_Cross(rt,ff)),d=V3_AsubB(World.position[sel],World.position[PLAYER1]); float bz=V3_dot(d,f);
            if (bz > 0.01f) { float tanFov=vtan((float)Sys_Settings.FOV*0.5f*PI/180.0f),k=384.0f/(bz*tanFov); float sx=683.0f+V3_dot(d,rt)*k, sy=384.0f-V3_dot(d,up)*k; if (sx > -48.0f && sx < 1414.0f && sy > -48.0f && sy < 816.0f) RenderUIImage((i16)(sx-24.0f),(i16)(sy-24.0f),48,48,1051); }
            RenderUIImage(966,84,400,600,1025);/*Edit object info panel bg*/
            RenderTextL(EF_LABELX,104,T_YELLOW,FONT_NORMAL,1.0f,"EDIT OBJECT #%u",sel); {char v[40];sFormat(v,40,"%u",e->index);RenderTextL(EF_LABELX,132,T_GREEN,FONT_NORMAL,1.0f,"const index");RenderTextL(EF_VALUEX,132,T_GREEN,FONT_NORMAL,1.0f,"%s",v);} bool caretOn=((u32)(get_time()*2.0f)&1)!=0;
            for(int i=0;i<EF_LAST;++i){u8 slot=(u8)i;i16 y=efRowY[i];char v[40];EditFieldValueText(slot,sel,v,40);
                bool editingThis=editFieldEditing&&editFieldSlot==slot; RenderTextL(EF_LABELX,y,editingThis?T_RED:T_GREEN,FONT_NORMAL,1.0f,"%s",efRowLabel[i]); 
                if(editingThis){char buf[44];sFormat(buf,44,"%s%s",editFieldBuffer,caretOn?"|":"");RenderTextL(EF_VALUEX,y,T_RED,FONT_NORMAL,1.0f,"%s",buf);}
                else {
                    float w=MeasureLineAdvance(v,FONT_NORMAL);bool hov=World.inventoryMode&&World.cursorPos_x>=EF_VALUEX&&World.cursorPos_x<=EF_VALUEX+w&&World.cursorPos_y>=y&&World.cursorPos_y<=y+26; 
                    if(hov){World.uiIsBlocking=true;if(!editFieldEditing&&(Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed||Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed)){sFormat(editFieldBuffer,40,"%s",v);editFieldSlot=slot;editFieldEditing=true;Sys_Input.mouseButtons[MOUSE_BUTTON_LEFT].pressed=Sys_Input.mouseButtons[MOUSE_BUTTON_RIGHT].pressed=false;}} 
                    RenderTextL(EF_VALUEX,y,hov?T_YELLOW:(i<EF_TEX?T_WHITE:T_GREEN),FONT_NORMAL,1.0f,"%s",v);
                }
            }
            if(editFieldEditing&&Sys_Input.scrollDelta!=0.0f){EditFieldStep((float)Sys_Input.scrollDelta);Sys_Input.scrollDelta=0.0f;}
            if(sel==World.weaponVModelIndex){int wep16=Get16WeaponIndexFromConstIndex(e->index);if(wep16>=0&&wep16<16){V3 offs=vWepOfs[wep16];offs.y+=wfx.reloadContainerPos.y;RenderTextL(EF_LABELX,580,T_GREEN,FONT_NORMAL,1.0f,"vm offset");RenderTextL(EF_VALUEX,580,T_YELLOW,FONT_NORMAL,1.0f,"%.2f %.2f %.2f",offs.x,offs.y,offs.z);}}
        }
    }
    if (World.Sys_UI.vmailActive) {
        if (World.Sys_UI.vmailFrameFinished < World.pauseRelativeTime && World.Sys_UI.vmailFrame < vmailEndFrames[World.Sys_UI.vmailActive]) {
            if (World.Sys_UI.vmailFrame == (vmailStartFrames[World.Sys_UI.vmailActive]+11)) play_wav(sounds[99],1.0f,(V3){0,0,0},false);
            World.Sys_UI.vmailFrameFinished=World.pauseRelativeTime + 0.1; World.Sys_UI.vmailFrame++; if (World.Sys_UI.vmailFrame > vmailEndFrames[World.Sys_UI.vmailActive]) World.Sys_UI.vmailFrame = vmailEndFrames[World.Sys_UI.vmailActive];
        }
        RenderUIImage(283,184,800,400,World.Sys_UI.vmailFrame);/*Vmail viewer*/
    }
    i16 debugTextStartY = 48;/*Diagnostics / Debugging*/
    if (Cheats.showLocation && !World.menuActive) RenderTextL(16, debugTextStartY, T_WHITE, FONT_NORMAL,1.0f, "x: %.4f, y: %.4f, z: %.4f, rx: %.4f, ry: %.4f, rz: %.4f, rw: %.4f",World.position[PLAYER1].x,World.position[PLAYER1].y,World.position[PLAYER1].z,World.rotation[PLAYER1].x,World.rotation[PLAYER1].y,World.rotation[PLAYER1].z,World.rotation[PLAYER1].w);
    i16 lineSpacing = 18;
    if (!World.menuActive && !Cheats.noHUD && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 1),T_WHITE,FONT_NORMAL,1.0f,"GPU ms::All:%.2f, Shad:%.2f, Pre:%.2f, Main:%.2f, SSR:%.2f, Comp:%.2f",World.gpuFrameMs,World.gpuShadowMs,World.gpuPreMs,World.gpuMainMs,World.gpuSsrMs,World.gpuCompMs);
    if (!World.menuActive && !Cheats.noHUD && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 2),T_WHITE,FONT_NORMAL,1.0f,"CPU ms::Shad:%.3f, Phys:%.3f, Subs:%u, Rend:%.3f, Pre Phys:%.3f, Logic:%.3f",shadowTime * 1000,physTime * 1000,World.substeps,renderTime * 1000,prePhys * 1000,gameTime * 1000);
    if (!World.menuActive && !Cheats.noHUD && !World.paused && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 3),T_WHITE,FONT_NORMAL,1.0f,"Grounded: %u  weaponCurrent: %d  weaponIndex: %d  pendingIdx: %d  wep16: %d  viewModel: %u  reloadDone: %.2f",(World.instances[PLAYER1].entflags & EF_GROUNDED) > 0,(int)World.invP1.weaponCurrent,(int)World.invP1.weaponIndex,(int)World.invP1.weaponIndexPending,Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex),((Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==5||Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==6)?49u:((Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==0||Get16WeaponIndexFromConstIndex((int)World.invP1.weaponIndex)==1)?50u:0u)),World.invP1.reloadFinished);
    if (!World.menuActive && !Cheats.noHUD && Cheats.showFPS) RenderTextL(16,debugTextStartY + (lineSpacing * 4),T_WHITE,FONT_NORMAL,1.0f,"Test Edx: %u, Time Elapsed: %.3f, Fatigue: %.2f, Sprinting: %u",editModeTestEntityDefinition,World.pauseRelativeTime - game_actual_start_time,World.invP1.fatigue,Sprint());
    if (!Cheats.noHUD && Cheats.showLocation) RenderTextL(16,debugTextStartY + (lineSpacing * 5),T_WHITE,FONT_NORMAL,1.0f,"Cursor: %d, %d  dx:%d dy:%d",World.cursorPos_x,World.cursorPos_y,World.currentMouse_dx,World.currentMouse_dy);
    if (Cheats.consoleActive) RenderTextL(16,0,T_WHITE,FONT_NORMAL,1.0f, "] %s",consoleEntryText);
    if (World.statusTextDecayFinished > World.current_time) RenderTextC(683,164,T_WHITE,FONT_NORMAL,1.0f, "%s",statusText);
    double time_now = get_time();
    if (Cheats.showFPS) {
        World.thisFrameTime = (time_now - World.last_time) * 1000.0; World.last_time = time_now; World.cpuFrameTime = World.cpuTime * 1000.0;
        u8 timingColor = (get_time() - World.current_time) > (World.thisFrameTime - 0.2) ? T_RED : T_WHITE;
        double avgCPU = 0.0;
        avgCPUt[avgCPUt_idx] = World.cpuFrameTime; avgCPUt_idx++; if (avgCPUt_idx >= AVG_CPU_TAPS) avgCPUt_idx = 0;
        u32 avgmax = globalframe > AVG_CPU_TAPS ? AVG_CPU_TAPS : globalframe;
        for (u32 i=0;i<avgmax;++i) avgCPU += avgCPUt[i];
        avgCPU /= (double)avgmax;
        RenderTextL(16 + 100, debugTextStartY - lineSpacing,timingColor,FONT_NORMAL,1.0f,"CPU avg %.2f",avgCPU); avgCPU = 0;
        RenderTextL(16, debugTextStartY - lineSpacing, timingColor,FONT_NORMAL,1.0f,"ms: %.2f",World.thisFrameTime);
        RenderTextL(16 + 250, debugTextStartY - lineSpacing,T_WHITE,FONT_NORMAL,1.0f,"(FPS:%d),Drwclls:%d [G:%d UI:%d Sh:%d] Vrt:%d E:%u|M:%u|P:%u",globalframesPerLastSecond,drawCalls,drawCallsNormal,uiDrawCalls,shadDrawCalls,vertsRendered,Cheats.editMode,World.menuActive,World.paused);
    }
    if ((World.inventoryMode && !Cheats.noHUD) || World.menuActive || World.paused){RenderUIImage((i16)(World.cursorPos_x) - 20,(i16)(World.cursorPos_y) - 20,40,40,GetCursorTexture());}else if (!Cheats.noHUD){RenderUIImage(663,364,40,40,GetCursorTexture());}/*Centered on UI fixed resolution 1366x768 FBO*/
    return time_now;
}
