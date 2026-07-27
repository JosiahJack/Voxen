// audio.c - Audio System supporting .mp3 + .wav filetypes only, uses Windows WASAPI or Linux ALSA("default" to work on PulseAudio and PipeWire or ALSA+dmix, w/ raw ioctl fallback).  Mixes synthesized sounds/music.
#include "common.h"
#include "lib.h"
enum{AUDIO_RATE=48000,AUDIO_CHANNELS=2,AUDIO_PERIOD_MS=10,AUDIO_PERIODS=4,AUDIO_FRAMES=((AUDIO_RATE*AUDIO_PERIOD_MS)/1000),AUDBUF_SIZE=(AUDIO_FRAMES*AUDIO_PERIODS)};
const char* sounds[SOUNDS_COUNT] = {
    "null"/*0*/, "ambient/ambient_frogs"/*1*/, "ambient/clicker"/*2*/, "ambient/compressor"/*3*/, "ambient/dishwasher"/*4*/, "ambient/drip_amb"/*5*/, "ambient/fan1"/*6*/, "ambient/generator_gas"/*7*/,
    "ambient/gurgle"/*8*/, "ambient/icemaker"/*9*/, "ambient/ind_lift1"/*10*/, "ambient/ind_lift2"/*11*/, "ambient/intake"/*12*/, "ambient/lathe"/*13*/, "ambient/lava2"/*14*/, "ambient/lev3loop1"/*15*/,
    "ambient/lev3loop2"/*16*/, "ambient/lev3loop3"/*17*/, "ambient/lev3loop4"/*18*/, "ambient/liquid_bubble"/*19*/, "ambient/machgear_loop"/*20*/, "ambient/machine_ambience"/*21*/, "ambient/machine_go"/*22*/, "ambient/machine_humamb7"/*23*/,
    "ambient/machine_humlonoise"/*24*/, "ambient/machine_loop1"/*25*/, "ambient/machine_loop2"/*26*/, "ambient/machinea1"/*27*/, "ambient/machinevat_loop"/*28*/, "ambient/pipewater_loop"/*29*/, "ambient/powerloom"/*30*/, "ambient/pump"/*31*/,
    "ambient/pump2"/*32*/, "ambient/rain"/*33*/, "ambient/sparks1"/*34*/, "ambient/sparks2"/*35*/, "ambient/sparks3"/*36*/, "ambient/steam_loop"/*37*/, "ambient/washing_machine"/*38*/, "buttons/button_beep"/*39*/,
    "buttons/button_chonk"/*40*/, "buttons/button_chuck"/*41*/, "buttons/button_clickclocktuck"/*42*/, "buttons/button_deny"/*43*/, "buttons/button_lswitch"/*44*/, "buttons/button_swipe"/*45*/, "buttons/keycard_success"/*46*/, "cyborgs/cyborg_die"/*47*/,
    "cyborgs/cyborg_die2"/*48*/, "cyborgs/cyborg_idle2"/*49*/, "cyborgs/cyborg_shoot"/*50*/, "cyborgs/cyborgwarrior_die"/*51*/, "cyborgs/diego_blubber"/*52*/, "cyborgs/ediego_dontkeepmewaiting"/*53*/, "cyborgs/ediego_faceme"/*54*/, "cyborgs/ediego_facexecutioner"/*55*/,
    "cyborgs/ediego_finishexecution"/*56*/, "cyborgs/ediego_wewillmeetagain"/*57*/, "cyborgs/yourlevelsareterrible"/*58*/, "cyborgs/yourweaponsareshoddybadweapons"/*59*/, "destroy/attack1_explode"/*60*/, "destroy/camera_destroy"/*61*/, "destroy/crate_break"/*62*/, "destroy/explode_minor"/*63*/,
    "destroy/explosion1"/*64*/, "destroy/explosion2"/*65*/, "destroy/explosion3"/*66*/, "destroy/hit2"/*67*/, "destroy/hit3"/*68*/, "destroy/screen_destroy"/*69*/, "doors/door_platform"/*70*/, "doors/doorbulkhead_open"/*71*/,
    "doors/doorbulkhead_open2"/*72*/, "doors/doorcompartment_open"/*73*/, "doors/doormech_open"/*74*/, "doors/doorpnuematic_open"/*75*/, "doors/doorwall_move"/*76*/, "doors/doorwall_stop"/*77*/, "hud/activate"/*78*/, "hud/batteryuse"/*79*/,
    "hud/changeweapon"/*80*/, "hud/cyber"/*81*/, "hud/deactivate"/*82*/, "hud/emailalert"/*83*/, "hud/energy_gone"/*84*/, "hud/envirosuit_on"/*85*/, "hud/frob_hardware"/*86*/, "hud/frob_item"/*87*/,
    "hud/jumpjets_off"/*88*/, "hud/patchuse"/*89*/, "hud/radiation"/*90*/, "hud/searchsound"/*91*/, "hud/select"/*92*/, "hud/sensaround"/*93*/, "hud/shield_absorb"/*94*/, "hud/shield_off"/*95*/,
    "hud/shield_on"/*96*/, "hud/tab"/*97*/, "hud/targetacquire"/*98*/, "hud/vmailalert"/*99*/, "misc/chargingstation"/*100*/, "misc/energy_hum"/*101*/, "misc/forcebridge"/*102*/, "misc/healstation"/*103*/,
    "misc/klaxon_station_alarm"/*104*/, "misc/machine_on"/*105*/, "misc/teleport"/*106*/, "misc/wing-o"/*107*/, "mutants/avianmut_attack"/*108*/, "mutants/avianmut_die"/*109*/, "mutants/avianmut_die2"/*110*/, "mutants/avianmut_sightsound"/*111*/,
    "mutants/footstep1"/*112*/, "mutants/footstep2"/*113*/, "mutants/footstep3"/*114*/, "mutants/footstep4"/*115*/, "mutants/footstep5"/*116*/, "mutants/gortiger_die"/*117*/, "mutants/gortiger_idle1"/*118*/, "mutants/gortiger_sightsound"/*119*/,
    "mutants/invisiblemut_die"/*120*/, "mutants/invisiblemut_idle1"/*121*/, "mutants/mutant_attack"/*122*/, "mutants/mutant_die"/*123*/, "mutants/plantmut_die2"/*124*/, "mutants/plantmut_throw"/*125*/, "mutants/plantmutstep1"/*126*/, "mutants/plantmutstep2"/*127*/,
    "mutants/plantmutstep3"/*128*/, "mutants/steplarge1"/*129*/, "mutants/steplarge2"/*130*/, "mutants/steplarge3"/*131*/, "mutants/steplarge4"/*132*/, "mutants/steplarge5"/*133*/, "mutants/virusmut_die"/*134*/, "player/jump"/*135*/,
    "player/jumpland"/*136*/, "player/ladder"/*137*/, "player/painalarm"/*138*/, "player/painalarmfast"/*139*/, "player/playerpain1"/*140*/, "robots/bot_destroy1"/*141*/, "robots/bot_destroy2"/*142*/, "robots/bot_destroy3"/*143*/,
    "robots/bot_destroy4"/*144*/, "robots/bot_destroybig"/*145*/, "robots/bot_pincherattack"/*146*/, "robots/bot_shoot1"/*147*/, "robots/bot_shoot2"/*148*/, "robots/bot_shoot3"/*149*/, "robots/bot_sight1"/*150*/, "robots/bot_sight2"/*151*/,
    "robots/bot_sight3"/*152*/, "robots/footfall"/*153*/, "robots/footstep1"/*154*/, "robots/footstep2"/*155*/, "robots/footstep3"/*156*/, "robots/footstephuge1"/*157*/, "robots/footstephuge2"/*158*/, "robots/footstephuge3"/*159*/,
    "robots/footstepsmall1"/*160*/, "robots/footstepsmall2"/*161*/, "robots/hopper_die"/*162*/, "robots/servo"/*163*/, "vox/vox_abortingprogram"/*164*/, "vox/vox_accesspanellocked"/*165*/, "vox/vox_accesspanelunlocked"/*166*/, "vox/vox_armoryaccessoverriden"/*167*/,
    "vox/vox_armoryaccessreinstituted"/*168*/, "vox/vox_baydoor3locked"/*169*/, "vox/vox_betagrovejettisoned"/*170*/, "vox/vox_betagrovelvatrunlocked"/*171*/, "vox/vox_biocontaminantdetected"/*172*/, "vox/vox_blastdoorlocked"/*173*/, "vox/vox_blastdoorunlocked"/*174*/, "vox/vox_bridgesepdone"/*175*/,
    "vox/vox_bridgesepsoon"/*176*/, "vox/vox_bridgesepstage1"/*177*/, "vox/vox_bridgesepstage2"/*178*/, "vox/vox_cameractivsecdoor"/*179*/, "vox/vox_chargeraccessgranted"/*180*/, "vox/vox_chargeroff"/*181*/, "vox/vox_corelocksdisengaged"/*182*/, "vox/vox_cybconvcancelled"/*183*/,
    "vox/vox_cybconvenabled"/*184*/, "vox/vox_demodsuccess"/*185*/, "vox/vox_destructcancelled"/*186*/, "vox/vox_destructnotenabled"/*187*/, "vox/vox_destructsecaccess"/*188*/, "vox/vox_doorclosedisabled"/*189*/, "vox/vox_ediegostorageclosetunlocked"/*190*/, "vox/vox_elevatordisabled"/*191*/,
    "vox/vox_emeraccesslocked"/*192*/, "vox/vox_entercode"/*193*/, "vox/vox_fallingairpressure"/*194*/, "vox/vox_flightbayarmoryunlocked"/*195*/, "vox/vox_forcedoor1opened"/*196*/, "vox/vox_forcedoor2opened"/*197*/, "vox/vox_forcedoor3opened"/*198*/, "vox/vox_gammajettisoned"/*199*/,
    "vox/vox_grovejettisoned"/*200*/, "vox/vox_grovejettisonstage1"/*201*/, "vox/vox_groveunlocked"/*202*/, "vox/vox_hallaccess"/*203*/, "vox/vox_hospitalsecdoorsopened"/*204*/, "vox/vox_jettisonalreadyenabled"/*205*/, "vox/vox_jettisonenabled"/*206*/, "vox/vox_jettisonfailure"/*207*/,
    "vox/vox_lifepodcancelled"/*208*/, "vox/vox_lifepodlaunchstart"/*209*/, "vox/vox_lifepodsdisabled"/*210*/, "vox/vox_maintdoorlocked"/*211*/, "vox/vox_needisotope"/*212*/, "vox/vox_nomasterjettisonenabled"/*213*/, "vox/vox_nosecaccess"/*214*/, "vox/vox_powerdivertedtor"/*215*/,
    "vox/vox_programinstalled"/*216*/, "vox/vox_radiationshielddeacticated"/*217*/, "vox/vox_radiationtreatdone"/*218*/, "vox/vox_reactorcountdown"/*219*/, "vox/vox_reactoroveloadstart"/*220*/, "vox/vox_reactoroverloadaccess"/*221*/, "vox/vox_reactoroverloadbypassed"/*222*/, "vox/vox_receptacleisolinearchipset"/*223*/,
    "vox/vox_relay428failure"/*224*/, "vox/vox_relay428faulty"/*225*/, "vox/vox_relayfixed"/*226*/, "vox/vox_replacedemodulator"/*227*/, "vox/vox_robotprodcancelled"/*228*/, "vox/vox_robotsactivated"/*229*/, "vox/vox_safetyinterlockdisabled"/*230*/, "vox/vox_safetyinterlockengaged"/*231*/,
    "vox/vox_safetyinterlocksenabled"/*232*/, "vox/vox_safetyoverridesonolaser"/*233*/, "vox/vox_secompoverrideneeded"/*234*/, "vox/vox_shieldgeneratorsready"/*235*/, "vox/vox_shieldson"/*236*/, "vox/vox_thelaserisdestroyed"/*237*/, "weapons/noammo"/*238*/, "weapons/wblaster"/*239*/,
    "weapons/wdartgun"/*240*/, "weapons/wdrill"/*241*/, "weapons/wearthshake"/*242*/, "weapons/wflechette"/*243*/, "weapons/wgrenade_arm"/*244*/, "weapons/wion"/*245*/, "weapons/wlaserrapier_hit"/*246*/, "weapons/wlaserrapier_swing"/*247*/,
    "weapons/wlocknload"/*248*/, "weapons/wmagnum"/*249*/, "weapons/wmagpulse"/*250*/, "weapons/wmarksman"/*251*/, "weapons/wpipe_dmg"/*252*/, "weapons/wpipe_hit"/*253*/, "weapons/wpipe_swing"/*254*/, "weapons/wpistol"/*255*/,
    "weapons/wpistolsilenced"/*256*/, "weapons/wplasma"/*257*/, "weapons/wpulser"/*258*/, "weapons/wrailgun"/*259*/, "weapons/wreload"/*260*/, "weapons/wricoshet"/*261*/, "weapons/wriotgun"/*262*/, "weapons/wskorpion"/*263*/,
    "weapons/wsparq"/*264*/, "weapons/wstungun"/*265*/, "weapons/wwoosh"/*266*/, "ambient/airhiss"/*267*/, "footsteps/Carpet/carpet_step1"/*268*/, "footsteps/Carpet/carpet_step2"/*269*/, "footsteps/Carpet/carpet_step3"/*270*/, "footsteps/Carpet/carpet_step4"/*271*/,
    "footsteps/Carpet/carpet_step5"/*272*/, "footsteps/Carpet/carpet_step6"/*273*/, "footsteps/Carpet/carpet_step7"/*274*/, "footsteps/Carpet/carpet_step8"/*275*/, "footsteps/Concrete/concrete_step1"/*276*/, "footsteps/Concrete/concrete_step2"/*277*/, "footsteps/Concrete/concrete_step3"/*278*/, "footsteps/Concrete/concrete_step4"/*279*/,
    "footsteps/Concrete/concrete_step5"/*280*/, "footsteps/Concrete/concrete_step6"/*281*/, "footsteps/Concrete/concrete_step7"/*282*/, "footsteps/Concrete/concrete_step8"/*283*/, "footsteps/Concrete Gritty/concrete_grit_step1"/*284*/, "footsteps/Concrete Gritty/concrete_grit_step2"/*285*/, "footsteps/Concrete Gritty/concrete_grit_step3"/*286*/, "footsteps/Concrete Gritty/concrete_grit_step4"/*287*/,
    "footsteps/Concrete Gritty/concrete_grit_step5"/*288*/, "footsteps/Concrete Gritty/concrete_grit_step6"/*289*/, "footsteps/Concrete Gritty/concrete_grit_step7"/*290*/, "footsteps/Concrete Gritty/concrete_grit_step8"/*291*/, "footsteps/Earth/earth_step1"/*292*/, "footsteps/Earth/earth_step2"/*293*/, "footsteps/Earth/earth_step3"/*294*/, "footsteps/Earth/earth_step4"/*295*/,
    "footsteps/Earth/earth_step5"/*296*/, "footsteps/Earth/earth_step6"/*297*/, "footsteps/Earth/earth_step7"/*298*/, "footsteps/Earth/earth_step8"/*299*/, "footsteps/Earth/gravel_step1"/*300*/, "footsteps/Earth/gravel_step2"/*301*/, "footsteps/Earth/gravel_step3"/*302*/, "footsteps/Earth/gravel_step4"/*303*/,
    "footsteps/Earth/gravel_step5"/*304*/, "footsteps/Earth/gravel_step6"/*305*/, "footsteps/Earth/gravel_step7"/*306*/, "footsteps/Earth/gravel_step8"/*307*/, "footsteps/Earth/rock_step1"/*308*/, "footsteps/Earth/rock_step2"/*309*/, "footsteps/Earth/rock_step3"/*310*/, "footsteps/Earth/rock_step4"/*311*/,
    "footsteps/Earth/rock_step5"/*312*/, "footsteps/Earth/rock_step6"/*313*/, "footsteps/Earth/rock_step7"/*314*/, "footsteps/Earth/rock_step8"/*315*/, "footsteps/Glass/glasssolid_step1"/*316*/, "footsteps/Glass/glasssolid_step2"/*317*/, "footsteps/Glass/glasssolid_step3"/*318*/, "footsteps/Glass/glasssolid_step4"/*319*/,
    "footsteps/Glass/glasssolid_step5"/*320*/, "footsteps/Glass/glasssolid_step6"/*321*/, "footsteps/Glass/glasssolid_step7"/*322*/, "footsteps/Glass/glasssolid_step8"/*323*/, "footsteps/Marble/marble_step1"/*324*/, "footsteps/Marble/marble_step2"/*325*/, "footsteps/Marble/marble_step3"/*326*/, "footsteps/Marble/marble_step4"/*327*/,
    "footsteps/Marble/marble_step5"/*328*/, "footsteps/Marble/marble_step6"/*329*/, "footsteps/Marble/marble_step7"/*330*/, "footsteps/Marble/marble_step8"/*331*/, "footsteps/Metal/metal_step1"/*332*/, "footsteps/Metal/metal_step2"/*333*/, "footsteps/Metal/metal_step3"/*334*/, "footsteps/Metal/metal_step4"/*335*/,
    "footsteps/Metal/metal_step5"/*336*/, "footsteps/Metal/metal_step6"/*337*/, "footsteps/Metal/metal_step7"/*338*/, "footsteps/Metal/metal_step8"/*339*/, "footsteps/Metal/metalgrate_step1"/*340*/, "footsteps/Metal/metalgrate_step2"/*341*/, "footsteps/Metal/metalgrate_step3"/*342*/, "footsteps/Metal/metalgrate_step4"/*343*/,
    "footsteps/Metal/metalgrate_step5"/*344*/, "footsteps/Metal/metalgrate_step6"/*345*/, "footsteps/Metal/metalgrate_step7"/*346*/, "footsteps/Metal/metalgrate_step8"/*347*/, "footsteps/Metal/metalsolid_step1"/*348*/, "footsteps/Metal/metalsolid_step2"/*349*/, "footsteps/Metal/metalsolid_step3"/*350*/, "footsteps/Metal/metalsolid_step4"/*351*/,
    "footsteps/Metal/metalsolid_step5"/*352*/, "footsteps/Metal/metalsolid_step6"/*353*/, "footsteps/Metal/metalsolid_step7"/*354*/, "footsteps/Metal/metalsolid_step8"/*355*/, "footsteps/Metal/metalthin_step1"/*356*/, "footsteps/Metal/metalthin_step2"/*357*/, "footsteps/Metal/metalthin_step3"/*358*/, "footsteps/Metal/metalthin_step4"/*359*/,
    "footsteps/Metal/metalthin_step5"/*360*/, "footsteps/Metal/metalthin_step6"/*361*/, "footsteps/Metal/metalthin_step7"/*362*/, "footsteps/Metal/metalthin_step8"/*363*/, "footsteps/Panel/panel_step1"/*364*/, "footsteps/Panel/panel_step2"/*365*/, "footsteps/Panel/panel_step3"/*366*/, "footsteps/Panel/panel_step4"/*367*/,
    "footsteps/Panel/panel_step5"/*368*/, "footsteps/Panel/panel_step6"/*369*/, "footsteps/Panel/panel_step7"/*370*/, "footsteps/Panel/panel_step8"/*371*/, "footsteps/Plaster/plaster_step1"/*372*/, "footsteps/Plaster/plaster_step2"/*373*/, "footsteps/Plaster/plaster_step3"/*374*/, "footsteps/Plaster/plaster_step4"/*375*/,
    "footsteps/Plaster/plaster_step5"/*376*/, "footsteps/Plaster/plaster_step6"/*377*/, "footsteps/Plaster/plaster_step7"/*378*/, "footsteps/Plaster/plaster_step8"/*379*/, "footsteps/Plastic/plastic_step1"/*380*/, "footsteps/Plastic/plastic_step2"/*381*/, "footsteps/Plastic/plastic_step3"/*382*/, "footsteps/Plastic/plastic_step4"/*383*/,
    "footsteps/Plastic/plastic_step5"/*384*/, "footsteps/Plastic/plastic_step6"/*385*/, "footsteps/Plastic/plastic_step7"/*386*/, "footsteps/Plastic/plastic_step8"/*387*/, "footsteps/Plastic/plasticsolid_step1"/*388*/, "footsteps/Plastic/plasticsolid_step2"/*389*/, "footsteps/Plastic/plasticsolid_step3"/*390*/, "footsteps/Plastic/plasticsolid_step4"/*391*/,
    "footsteps/Plastic/plasticsolid_step5"/*392*/, "footsteps/Plastic/plasticsolid_step6"/*393*/, "footsteps/Plastic/plasticsolid_step7"/*394*/, "footsteps/Plastic/plasticsolid_step8"/*395*/, "footsteps/Rubber/rubber_step1"/*396*/, "footsteps/Rubber/rubber_step2"/*397*/, "footsteps/Rubber/rubber_step3"/*398*/, "footsteps/Rubber/rubber_step4"/*399*/,
    "footsteps/Rubber/rubber_step5"/*400*/, "footsteps/Rubber/rubber_step6"/*401*/, "footsteps/Rubber/rubber_step7"/*402*/, "footsteps/Rubber/rubber_step8"/*403*/, "footsteps/Sand/sand_step1"/*404*/, "footsteps/Sand/sand_step2"/*405*/, "footsteps/Sand/sand_step3"/*406*/, "footsteps/Sand/sand_step4"/*407*/,
    "footsteps/Sand/sand_step5"/*408*/, "footsteps/Sand/sand_step6"/*409*/, "footsteps/Sand/sand_step7"/*410*/, "footsteps/Sand/sand_step8"/*411*/, "footsteps/Squish/squish_step1"/*412*/, "footsteps/Squish/squish_step2"/*413*/, "footsteps/Squish/squish_step3"/*414*/, "footsteps/Squish/squish_step4"/*415*/,
    "footsteps/Squish/squish_step5"/*416*/, "footsteps/Squish/squish_step6"/*417*/, "footsteps/Squish/squish_step7"/*418*/, "footsteps/Squish/squish_step8"/*419*/, "footsteps/Squish/squish_step9"/*420*/, "footsteps/Squish/squish_step10"/*421*/, "footsteps/Squish/squish_step11"/*422*/, "footsteps/Squish/squish_step12"/*423*/,
    "footsteps/Squish/squish_step13"/*424*/, "footsteps/Squish/squish_step14"/*425*/, "footsteps/Squish/squish_step15"/*426*/, "footsteps/Squish/squish_step16"/*427*/, "footsteps/Vent/vent_step1"/*428*/, "footsteps/Vent/vent_step2"/*429*/, "footsteps/Vent/vent_step3"/*430*/, "footsteps/Vent/vent_step4"/*431*/,
    "footsteps/Vent/vent_step5"/*432*/, "footsteps/Vent/vent_step6"/*433*/, "footsteps/Vent/vent_step7"/*434*/, "footsteps/Vent/vent_step8"/*435*/, "footsteps/Vent/vent_step9"/*436*/, "footsteps/Vent/vent_step10"/*437*/, "footsteps/Water/water_foot_step1"/*438*/, "footsteps/Water/water_foot_step2"/*439*/,
    "footsteps/Water/water_foot_step3"/*440*/, "footsteps/Water/water_foot_step4"/*441*/, "footsteps/Water/water_foot_step5"/*442*/, "footsteps/Wood/wood_step1"/*443*/, "footsteps/Wood/wood_step2"/*444*/, "footsteps/Wood/wood_step3"/*445*/, "footsteps/Wood/wood_step4"/*446*/, "footsteps/Wood/wood_step5"/*447*/,
    "footsteps/Wood/wood_step6"/*448*/, "footsteps/Wood/wood_step7"/*449*/, "footsteps/Wood/wood_step8"/*450*/, "footsteps/Wood/woodcrate_step1"/*451*/, "footsteps/Wood/woodcrate_step2"/*452*/, "footsteps/Wood/woodcrate_step3"/*453*/, "footsteps/Wood/woodcrate_step4"/*454*/, "footsteps/Wood/woodcrate_step5"/*455*/,
    "footsteps/Wood/woodcrate_step6"/*456*/, "footsteps/Wood/woodcrate_step7"/*457*/, "footsteps/Wood/woodcrate_step8"/*458*/, "footsteps/Clothes/rustle01"/*459*/, "footsteps/Clothes/rustle02"/*460*/, "footsteps/Clothes/rustle03"/*461*/, "footsteps/Clothes/rustle04"/*462*/, "footsteps/Clothes/rustle05"/*463*/,
    "footsteps/Clothes/rustle06"/*464*/, "footsteps/Clothes/rustle07"/*465*/, "buttons/keycard_wrong"/*466*/, "buttons/locked_deny"/*467*/, "buttons/blocked_by_security"/*468*/, "shodan/shodan1"/*469*/, "shodan/shodan_beyondcomprehension"/*470*/, "shodan/shodan_ceaseimmediately"/*471*/,
    "shodan/shodan_ceasepestering"/*472*/, "shodan/shodan_ceaseyourmeddling"/*473*/, "shodan/shodan_cyborg65v"/*474*/, "shodan/shodan_cyborg77e"/*475*/, "shodan/shodan_destroyitmychildren"/*476*/, "shodan/shodan_destroymycameras"/*477*/, "shodan/shodan_didyouthinkididntknow"/*478*/, "shodan/shodan_directivetocyborgf71"/*479*/,
    "shodan/shodan_doyouthinkshecanhelp"/*480*/, "shodan/shodan_drunkwithvisioniamgod"/*481*/, "shodan/shodan_energydrainmines"/*482*/, "shodan/shodan_enjoyyourvictory"/*483*/, "shodan/shodan_enterroomgrave"/*484*/, "shodan/shodan_gaurdthrone"/*485*/, "shodan/shodan_grovesteps"/*486*/, "shodan/shodan_imonthebridge"/*487*/,
    "shodan/shodan_insectloose_plansforearth"/*488*/, "shodan/shodan_irulehere"/*489*/, "shodan/shodan_iwilldownloadmyself"/*490*/, "shodan/shodan_laserisbeingreadied"/*491*/, "shodan/shodan_learnchildren"/*492*/, "shodan/shodan_level8layout"/*493*/, "shodan/shodan_lookatyouhacker"/*494*/, "shodan/shodan_makeyouselfcomfy"/*495*/,
    "shodan/shodan_morrisbrocailisadolt"/*496*/, "shodan/shodan_nicejump"/*497*/, "shodan/shodan_nomoretransmissions"/*498*/, "shodan/shodan_prematurefruition"/*499*/, "shodan/shodan_quietstation"/*500*/, "shodan/shodan_releasemyinfectedchildren"/*501*/, "shodan/shodan_removeyourself"/*502*/, "shodan/shodan_shecanthelp"/*503*/,
    "shodan/shodan_stepintomytrap"/*504*/, "shodan/shodan_thankyou"/*505*/, "shodan/shodan_throneofgod"/*506*/, "shodan/shodan_tocyborg43s"/*507*/, "shodan/shodan_toolatetosavefriends"/*508*/, "shodan/shodan_welcomedeathmachine"/*509*/, "shodan/shodan_whoareyou"/*510*/, "shodan/shodan_youdestroyedmystation"/*511*/,
    "shodan/shodan_youmychildren"/*512*/, "physics/impact_barrel"/*513*/, "physics/impact_canister"/*514*/, "physics/impact_canistersmall"/*515*/, "physics/impact_ceramic_light"/*516*/, "physics/impact_cloth"/*517*/, "physics/impact_crate_break"/*518*/, "physics/impact_electronics"/*519*/,
    "physics/impact_glass_break"/*520*/, "physics/impact_glass_small"/*521*/, "physics/impact_keycard"/*522*/, "physics/impact_lightweight"/*523*/, "physics/impact_medium"/*524*/, "physics/impact_metal_medium"/*525*/, "physics/impact_metal_tiny"/*526*/, "physics/impact_pipe"/*527*/,
    "physics/impact_rapier"/*528*/, "physics/impact_rifle1"/*529*/, "physics/impact_rifle2"/*530*/, "physics/impact_soda"/*531*/, "physics/impact_soft"/*532*/, "physics/impact_stone"/*533*/, "physics/impact_wood"/*534*/, "physics/impact_metal_large"/*535*/,
    "physics/impact_wood_large"/*536*/, "footsteps/Carpet/carpet_land1"/*537*/, "footsteps/Carpet/carpet_land2"/*538*/, "footsteps/Carpet/carpet_land3"/*539*/, "footsteps/Carpet/carpet_jump1"/*540*/, "footsteps/Carpet/carpet_jump2"/*541*/, "footsteps/Carpet/carpet_jump3"/*542*/, "footsteps/Concrete/concrete_land1"/*543*/,
    "footsteps/Concrete/concrete_land2"/*544*/, "footsteps/Concrete/concrete_land3"/*545*/, "footsteps/Concrete/concrete_jump1"/*546*/, "footsteps/Concrete/concrete_jump2"/*547*/, "footsteps/Concrete/concrete_jump3"/*548*/, "footsteps/Concrete Gritty/concrete_grit_land1"/*549*/, "footsteps/Concrete Gritty/concrete_grit_land2"/*550*/, "footsteps/Concrete Gritty/concrete_grit_land3"/*551*/,
    "footsteps/Concrete Gritty/concrete_grit_jump1"/*552*/, "footsteps/Concrete Gritty/concrete_grit_jump2"/*553*/, "footsteps/Concrete Gritty/concrete_grit_jump3"/*554*/, "footsteps/Earth/earth_land1"/*555*/, "footsteps/Earth/earth_land2"/*556*/, "footsteps/Earth/earth_land3"/*557*/, "footsteps/Earth/earth_jump1"/*558*/, "footsteps/Earth/earth_jump2"/*559*/,
    "footsteps/Earth/earth_jump3"/*560*/, "footsteps/Earth/gravel_land1"/*561*/, "footsteps/Earth/gravel_land2"/*562*/, "footsteps/Earth/gravel_land3"/*563*/, "footsteps/Earth/gravel_jump1"/*564*/, "footsteps/Earth/gravel_jump2"/*565*/, "footsteps/Earth/gravel_jump3"/*566*/, "footsteps/Earth/rock_land1"/*567*/,
    "footsteps/Earth/rock_land2"/*568*/, "footsteps/Earth/rock_land3"/*569*/, "footsteps/Earth/rock_jump1"/*570*/, "footsteps/Earth/rock_jump2"/*571*/, "footsteps/Earth/rock_jump3"/*572*/, "footsteps/Glass/glasssolid_land1"/*573*/, "footsteps/Glass/glasssolid_land2"/*574*/, "footsteps/Glass/glasssolid_land3"/*575*/,
    "footsteps/Glass/glasssolid_jump1"/*576*/, "footsteps/Glass/glasssolid_jump2"/*577*/, "footsteps/Glass/glasssolid_jump3"/*578*/, "footsteps/Marble/marble_land1"/*579*/, "footsteps/Marble/marble_land2"/*580*/, "footsteps/Marble/marble_land3"/*581*/, "footsteps/Marble/marble_jump1"/*582*/, "footsteps/Marble/marble_jump2"/*583*/,
    "footsteps/Marble/marble_jump3"/*584*/, "footsteps/Metal/metal_land1"/*585*/, "footsteps/Metal/metal_land2"/*586*/, "footsteps/Metal/metal_land3"/*587*/, "footsteps/Metal/metal_jump1"/*588*/, "footsteps/Metal/metal_jump2"/*589*/, "footsteps/Metal/metal_jump3"/*590*/, "footsteps/Metal/metalgrate_land1"/*591*/,
    "footsteps/Metal/metalgrate_land2"/*592*/, "footsteps/Metal/metalgrate_land3"/*593*/, "footsteps/Metal/metalgrate_jump1"/*594*/, "footsteps/Metal/metalgrate_jump2"/*595*/, "footsteps/Metal/metalgrate_jump3"/*596*/, "footsteps/Metal/metalsolid_land1"/*597*/, "footsteps/Metal/metalsolid_land2"/*598*/, "footsteps/Metal/metalsolid_land3"/*599*/,
    "footsteps/Metal/metalsolid_jump1"/*600*/, "footsteps/Metal/metalsolid_jump2"/*601*/, "footsteps/Metal/metalsolid_jump3"/*602*/, "footsteps/Metal/metalthin_land1"/*603*/, "footsteps/Metal/metalthin_land2"/*604*/, "footsteps/Metal/metalthin_land3"/*605*/, "footsteps/Metal/metalthin_jump1"/*606*/, "footsteps/Metal/metalthin_jump2"/*607*/,
    "footsteps/Metal/metalthin_jump3"/*608*/, "footsteps/Panel/panel_land1"/*609*/, "footsteps/Panel/panel_land2"/*610*/, "footsteps/Panel/panel_land3"/*611*/, "footsteps/Panel/panel_jump1"/*612*/, "footsteps/Panel/panel_jump2"/*613*/, "footsteps/Panel/panel_jump3"/*614*/, "footsteps/Plaster/plaster_land1"/*615*/,
    "footsteps/Plaster/plaster_land2"/*616*/, "footsteps/Plaster/plaster_land3"/*617*/, "footsteps/Plaster/plaster_jump1"/*618*/, "footsteps/Plaster/plaster_jump2"/*619*/, "footsteps/Plaster/plaster_jump3"/*620*/, "footsteps/Plastic/plastic_land1"/*621*/, "footsteps/Plastic/plastic_land2"/*622*/, "footsteps/Plastic/plastic_land3"/*623*/,
    "footsteps/Plastic/plastic_jump1"/*624*/, "footsteps/Plastic/plastic_jump2"/*625*/, "footsteps/Plastic/plastic_jump3"/*626*/, "footsteps/Plastic/plasticsolid_land1"/*627*/, "footsteps/Plastic/plasticsolid_land2"/*628*/, "footsteps/Plastic/plasticsolid_land3"/*629*/, "footsteps/Plastic/plasticsolid_jump1"/*630*/, "footsteps/Plastic/plasticsolid_jump2"/*631*/,
    "footsteps/Plastic/plasticsolid_jump3"/*632*/, "footsteps/Rubber/rubber_land1"/*633*/, "footsteps/Rubber/rubber_land2"/*634*/, "footsteps/Rubber/rubber_land3"/*635*/, "footsteps/Rubber/rubber_jump1"/*636*/, "footsteps/Rubber/rubber_jump2"/*637*/, "footsteps/Rubber/rubber_jump3"/*638*/, "footsteps/Sand/sand_land1"/*639*/,
    "footsteps/Sand/sand_land2"/*640*/, "footsteps/Sand/sand_land3"/*641*/, "footsteps/Sand/sand_jump1"/*642*/, "footsteps/Sand/sand_jump2"/*643*/, "footsteps/Sand/sand_jump3"/*644*/, "footsteps/Squish/squish_land1"/*645*/, "footsteps/Squish/squish_land2"/*646*/, "footsteps/Squish/squish_land3"/*647*/,
    "footsteps/Squish/squish_jump1"/*648*/, "footsteps/Squish/squish_jump2"/*649*/, "footsteps/Squish/squish_jump3"/*650*/, "footsteps/Water/water_wade1"/*651*/, "footsteps/Water/water_wade2"/*652*/, "footsteps/Water/water_wade3"/*653*/, "footsteps/Water/water_wade4"/*654*/, "footsteps/Water/water_step6"/*655*/,
    "footsteps/Water/water_step7"/*656*/, "footsteps/Water/water_step8"/*657*/, "footsteps/Wood/wood_land1"/*658*/, "footsteps/Wood/wood_land2"/*659*/, "footsteps/Wood/wood_land3"/*660*/, "footsteps/Wood/wood_jump1"/*661*/, "footsteps/Wood/wood_jump2"/*662*/, "footsteps/Wood/wood_jump3"/*663*/,
    "footsteps/Wood/woodcrate_land1"/*664*/, "footsteps/Wood/woodcrate_land2"/*665*/, "footsteps/Wood/woodcrate_land3"/*666*/, "footsteps/Wood/woodcrate_jump1"/*667*/, "footsteps/Wood/woodcrate_jump2"/*668*/, "footsteps/Wood/woodcrate_jump3"/*669*/};
const char* audioLogs[LOGCNT] = {"logs/ghiran-2"/*0*/,"logs/steinberg-1"/*1*/,"logs/raines-1"/*2*/,"logs/sigmund-1"/*3*/,"logs/stevens-1"/*4*/,"shodan/shodan_youmychildren"/*5*/,"null"/*6*/,"logs/oconnel-1"/*7*/,"logs/honig-1"/*8*/,"logs/honig-2"/*9*/,
                                 "logs/stackhouse-1"/*10*/,"shodan/shodan_directivetocyborgf71"/*11*/,"logs/kirby-1"/*12*/,"logs/ozark-1"/*13*/,"logs/ghiran-1"/*14*/,"logs/darcy-1"/*15*/,"logs/blankenship-1"/*16*/,"logs/grossman-1"/*17*/,"logs/grossman-2"/*18*/,
                                 "logs/grossman-3"/*19*/,"shodan/shodan_laserisbeingreadied"/*20*/,"logs/stannek-2"/*21*/,"logs/anderczyk"/*22*/,"logs/endicott-1"/*23*/,"logs/wong-2"/*24*/,"logs/melville-1"/*25*/,"logs/baerga-1"/*26*/,
                                 "shodan/shodan_tocyborg43s"/*27*/,"logs/darcy-3"/*28*/,"logs/darcy-2"/*29*/,"logs/stannek-1"/*30*/,"logs/baerga-2"/*31*/,"shodan/shodan_energydrainmines"/*32*/,"shodan/shodan_drunkwithvisioniamgod"/*33*/,"logs/hayes-1"/*34*/,
                                 "logs/fortier-1"/*35*/,"logs/fortier-2"/*36*/,"logs/harvey-1"/*37*/,"logs/ghiran-3"/*38*/,"logs/ghiran-4"/*39*/,"logs/ghiran-5"/*40*/,"shodan/shodan_cyborg77e"/*41*/,"vox/vox_relay428failure"/*42*/,"logs/aubrey-1"/*43*/,
                                 "logs/aubrey-2"/*44*/,"logs/wong-1"/*45*/,"logs/macleod-1"/*46*/,"logs/sabo-1"/*47*/,"logs/macleod-2"/*48*/,"logs/macleod-3"/*49*/,"logs/diego-1"/*50*/,"logs/schuler-1"/*51*/,"logs/travers-3"/*52*/,"logs/travers-4"/*53*/,
                                 "logs/travers-2"/*54*/,"logs/kell-1"/*55*/, "logs/kell-2"/*56*/,"logs/travers-1"/*57*/, "shodan/shodan_cyborg65v"/*58*/,"logs/mcdaniel-1"/*59*/,"logs/mcdaniel-2"/*60*/,"logs/parovski-3"/*61*/,"logs/perry-1"/*62*/,
                                 "logs/koufax-1"/*63*/,"logs/wilkinson-1"/*64*/,"logs/diego-2"/*65*/,"shodan/shodan_grovesteps"/*66*/, "logs/aaron-1"/*67*/,"logs/aaron-2"/*68*/,"logs/diego-3"/*69*/,"logs/aaron-3"/*70*/,"logs/hessman-1"/*71*/,
                                 "logs/richie-1"/*72*/,"logs/schuler-2"/*73*/,"logs/schuler-3"/*74*/,"logs/schuler-4"/*75*/,"logs/hessman-2"/*76*/,"shodan/shodan_learnchildren"/*77*/,"shodan/shodan_level8layout"/*78*/,"logs/stevens-2"/*79*/,"logs/diego-4"/*80*/,
                                 "shodan/shodan_gaurdthrone"/*81*/,"logs/ghiran-6"/*82*/,"shodan/shodan1"/*83*/,"logs/rebecca-1"/*84*/,"shodan/shodan_whoareyou"/*85*/,"shodan/shodan_insectloose_plansforearth"/*86*/,"logs/parovski-1"/*87*/,"logs/parovski-2"/*88*/,
                                 "shodan/shodan_quietstation"/*89*/,"logs/rebecca-3"/*90*/,"shodan/shodan_iwilldownloadmyself"/*91*/,"shodan/shodan_imonthebridge"/*92*/,"logs/rebecca-4"/*93*/, "shodan/shodan_enjoyyourvictory"/*94*/,
                                 "cyborgs/ediego_dontkeepmewaiting"/*95*/,"shodan/shodan_youdestroyedmystation"/*96*/,"shodan/shodan_nomoretransmissions"/*97*/,"shodan/shodan_doyouthinkshecanhelp"/*98*/,"shodan/shodan_ceasepestering"/*99*/,
                                 "shodan/shodan_morrisbrocailisadolt"/*100*/,"shodan/shodan_shecanthelp"/*101*/,"null"/*102*/,"null"/*103*/,"null"/*104*/,"null"/*105*/,"null"/*106*/,"null"/*107*/,"null"/*108*/,"hud/vmailalert"/*109*/,"null"/*110*/,"null"/*111*/,
                                 "shodan/shodan_thankyou"/*112*/,"hud/vmailalert"/*113*/,"null"/*114*/,"hud/vmailalert"/*115*/,"hud/vmailalert"/*116*/,"null"/*117*/,"hud/vmailalert"/*118*/,"hud/vmailalert"/*119*/,"null"/*120*/,"null"/*121*/,"null"/*122*/,
                                 "null"/*123*/,"null"/*124*/,"null"/*125*/,"null"/*126*/,"null"/*127*/,"null"/*128*/,"null"/*129*/,"null"/*130*/,"null"/*131*/,"null"/*132*/};
static const char* GetRandomSound(FootStepType fstep,const int* starts,const int* counts,int size) { int idx=(int)fstep; if (idx<=0 || idx>=size) return sounds[0]; return sounds[random_range_u32(starts[idx],starts[idx]+counts[idx])]; }
FootStepType GetFootstepTypeForPrefab(int pid) {
    static FootStepType table[530]; static int initialized=0;
    if (!initialized) {
        for (int i=0; i<530; ++i) table[i]=FSTP_Plastic;
        typedef struct {int min,max;FootStepType type;} Range;
        const Range ranges[]={{0,0,FSTP_None},{21,22,FSTP_None},{2,10,FSTP_Squish},{98,100,FSTP_Squish},{110,110,FSTP_Squish},{13,18,FSTP_Metal2},{82,88,FSTP_Metal2},{23,40,FSTP_Plastic2},{48,49,FSTP_Plastic2},{53,53,FSTP_Plastic2},{41,47,FSTP_Plastic},
                              {149,159,FSTP_Plastic},{64,66,FSTP_Sand},{94,96,FSTP_Grass},{101,109,FSTP_GrittyCrete},{111,111,FSTP_GrittyCrete},{144,148,FSTP_Vent},{131,134,FSTP_Metal},{1,1,FSTP_Glass},{19,19,FSTP_Glass},{77,77,FSTP_Glass},{93,93,FSTP_Glass},
                              {122,122,FSTP_Glass},{126,126,FSTP_Glass},{128,128,FSTP_Glass},{187,187,FSTP_Glass},{221,221,FSTP_Glass},{235,237,FSTP_Glass},{260,261,FSTP_Glass},{270,271,FSTP_Glass},{279,279,FSTP_Glass},{11,11,FSTP_Metpanel},{51,52,FSTP_Metpanel},
                              {56,57,FSTP_Metpanel},{71,71,FSTP_Metpanel},{116,117,FSTP_Metpanel},{120,121,FSTP_Metpanel},{135,136,FSTP_Metpanel},{139,140,FSTP_Metpanel},{204,207,FSTP_Metpanel},{458,460,FSTP_Metpanel},{477,479,FSTP_Metpanel},{12,12,FSTP_Marble},
                              {61,61,FSTP_Marble},{72,73,FSTP_Marble},{76,76,FSTP_Marble},{280,287,FSTP_Marble},{20,20,FSTP_Wood2},{464,464,FSTP_Wood2},{472,476,FSTP_Wood2},{50,50,FSTP_Carpet},{70,70,FSTP_Carpet},{75,75,FSTP_Carpet},{54,55,FSTP_Gravel},
                              {62,63,FSTP_Metal},{78,78,FSTP_Metal},{89,89,FSTP_Metal},{112,112,FSTP_Metal},{127,127,FSTP_Metal},{129,129,FSTP_Metal},{137,138,FSTP_Metal},{141,143,FSTP_Metal},{189,189,FSTP_Metal},{196,196,FSTP_Metal},{208,220,FSTP_Metal},
                              {222,230,FSTP_Metal},{238,240,FSTP_Metal},{292,301,FSTP_Metal},{305,305,FSTP_Metal},{461,461,FSTP_Metal},{463,463,FSTP_Metal},{500,500,FSTP_Metal},{516,516,FSTP_Metal},{525,526,FSTP_Metal},{74,74,FSTP_Plaster},{306,306,FSTP_Plaster},
                              {79,79,FSTP_Grate},{130,130,FSTP_Grate},{231,231,FSTP_Grate},{262,265,FSTP_Grate},{527,529,FSTP_Grate},{80,81,FSTP_Rubber},{124,125,FSTP_Rubber},{302,304,FSTP_Rubber},{97,97,FSTP_Water},{113,115,FSTP_Panel},{118,119,FSTP_Panel},
                              {123,123,FSTP_Panel},{160,161,FSTP_Panel},{169,177,FSTP_Panel},{253,255,FSTP_Panel},{515,515,FSTP_Panel}};
        int num_ranges=(int)(sizeof(ranges)/sizeof(ranges[0])); for (int r=0; r<num_ranges; ++r) {  for (int i=ranges[r].min; i<=ranges[r].max && i<530; ++i) { if(i>=0){table[i]=ranges[r].type;} }  }
        initialized=1;
    }
    if (pid<0 || pid>=530) return FSTP_Plastic;
    return table[pid];
}

const char* FootStepSound(FootStepType fstep) { static const int starts[]={0,268,276,284,292,300,308,316,324,332,340,348,356,364,372,380,388,396,404,412,428,438,443,451}; static const int counts[]={0,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,16,10,5,8,8}; return GetRandomSound(fstep,starts,counts,(int)(sizeof(starts)/sizeof(starts[0]))); }
const char* JumpSound(FootStepType fstep) { static const int starts[]={0,540,546,552,558,564,570,576,582,588,594,600,606,612,618,624,630,636,642,648,429,651,661,667}; static const int counts[]={0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,4,3,3}; return GetRandomSound(fstep,starts,counts,(int)(sizeof(starts)/sizeof(starts[0]))); }
const char* JumpLandSound(FootStepType fstep) { static const int starts[]={0,537,543,549,555,561,567,573,579,585,591,597,603,609,615,621,627,633,639,645,428,655,658,664}; static const int counts[]={0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,10,3,3,3}; return GetRandomSound(fstep,starts,counts,(int)(sizeof(starts)/sizeof(starts[0]))); }
#if defined(_WIN32)
    #define FAILED(hr)    ((i32)(hr) <  0)
    #define PCM_NONBLOCK (1<<1)
    #define PCM_FORMAT_S16_LE 2
    #define REFIID const GUID *const
    typedef struct IMMDevice IMMDevice; typedef struct IMMDeviceEnumerator IMMDeviceEnumerator;
    typedef struct{ i32(__stdcall*q)(void*,const void*,void**); u32(__stdcall*a)(void*); u32(__stdcall*Release)(void*); i32(__stdcall* Activate)(void*,const void*,u32,void*,void**);} IMMDeviceVtbl; struct IMMDevice{IMMDeviceVtbl*lpVtbl;};
    typedef struct{ i32(__stdcall*q)(void*,const void*,void**); u32(__stdcall*a)(void*); u32(__stdcall*Release)(void*); i32(__stdcall*e)(void*,int,u32,void**); i32(__stdcall*GetDefaultAudioEndpoint)(void*,int,int,IMMDevice**);}IMMDeviceEnumeratorVtbl;struct IMMDeviceEnumerator{IMMDeviceEnumeratorVtbl*lpVtbl;};
    typedef struct IAudioClient IAudioClient; typedef struct IAudioRenderClient IAudioRenderClient; typedef struct { u16 t,n; u32 s, a; u16 b,w,c; } WAVEFORMATEX;
    typedef struct IAudioClientVtbl { i32 (__stdcall *QueryInterface)(void*, const void*,void**); u32 (__stdcall *AddRef)(void*); u32 (__stdcall *Release)(void*); i32 (__stdcall *Initialize)(void*,int,u32,i64,i64,const WAVEFORMATEX*,const void*); i32 (__stdcall *GetBufferSize)(void*,u32*); i32 (__stdcall *GetStreamLength)(void*,i64*); i32 (__stdcall *GetCurrentPadding)(void*,u32*);
        i32 (__stdcall *IsFormatSupported)(void*, int, const WAVEFORMATEX*, WAVEFORMATEX**); i32 (__stdcall *GetMixFormat)(void*,WAVEFORMATEX**); i32 (__stdcall *GetDevicePeriod)(void*,i64*,i64*); i32 (__stdcall *Start)(void*); i32 (__stdcall *Stop)(void*); i32 (__stdcall *Reset)(void*); i32 (__stdcall *SetEventHandle)(void*,void*); i32 (__stdcall *GetService)(void*,const void*,void**); } IAudioClientVtbl;
    struct IAudioClient { IAudioClientVtbl* lpVtbl; }; typedef struct IAudioRenderClientVtbl { i32 (__stdcall *QueryInterface)(void*,const void*,void**); u32 (__stdcall *AddRef)(void*); u32 (__stdcall *Release)(void*); i32 (__stdcall *GetBuffer)(void*,u32,u8**); i32 (__stdcall *ReleaseBuffer)(void*,u32,u32); } IAudioRenderClientVtbl;
    struct IAudioRenderClient { IAudioRenderClientVtbl* lpVtbl; }; typedef struct IUnknown IUnknown; typedef struct IUnknownVtbl { i32 (__stdcall *QueryInterface)(IUnknown* This, const GUID* riid, void** ppvObject); u32 (__stdcall *AddRef)(IUnknown* This); u32 (__stdcall *Release)(IUnknown* This); } IUnknownVtbl; struct IUnknown { const IUnknownVtbl* lpVtbl; };
    typedef u32 snd_pcm_uframes_t; typedef struct { int format,access,rate,channels,period_frames,periods; } pcm_params_t; typedef struct { snd_pcm_uframes_t hw_ptr; }  pcm_status_t; typedef struct { snd_pcm_uframes_t appl_ptr; } pcm_control_t; typedef struct { pcm_status_t status; pcm_control_t control; } pcm_sync_t; typedef struct {IAudioClient *client; IAudioRenderClient *render; u32 buffer_frames; i32 rate,channels,period_frames; bool open; } wasapi_dev_t;
    i32 WINAPI CoInitializeEx(void*,u32); i32 WINAPI CoCreateInstance(const GUID*,IUnknown*,u32,REFIID,void**); static wasapi_dev_t wasapi_devs[8]; static int wasapi_dev_count = 0;
    #define FD_TO_IDX(fd) ((int)(intptr_t)(fd)-100)
    #define IDX_TO_FD(i)  ((FHandle)(intptr_t)((i)+100))
    static const GUID IID_IAudioClient = {0x1CB9AD4C,0xDBFA,0x4C32,{0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2}}; static const GUID IID_IAudioRenderClient = {0xF294ACFC,0x3146,0x4483,{0xA7,0xBF,0xAD,0xDC,0xA7,0xC2,0x60,0xE2}};
    static int wasapi_init_device(IMMDevice *dev,int r,int ch,int period_frames,int p) {
        if(wasapi_dev_count>=8){return -1;}
        wasapi_dev_t *w=&wasapi_devs[wasapi_dev_count]; i32 hr=dev->lpVtbl->Activate(dev,&IID_IAudioClient,23,NULL,(void**)&w->client); if(FAILED(hr)){DualLogError("WASAPI Activate failed, %u\n",hr); return -1;}
        WAVEFORMATEX fmt = {1,(u16)ch,(u32)r,(u32)(r*ch*2),(u16)(ch*2),16,0}; i64 buf_dur = (i64)(period_frames*p)*10000000LL/r; hr = w->client->lpVtbl->Initialize(w->client,0,524288,buf_dur,0,&fmt,NULL); if(FAILED(hr)){w->client->lpVtbl->Release(w->client); return -1;}
        w->client->lpVtbl->GetBufferSize(w->client,&w->buffer_frames); hr = w->client->lpVtbl->GetService(w->client,&IID_IAudioRenderClient,(void**)&w->render); if(FAILED(hr)){ w->client->lpVtbl->Release(w->client); return -1;}
        w->client->lpVtbl->Start(w->client);w->rate=r; w->channels=ch; w->period_frames=period_frames; w->open=true; return wasapi_dev_count++;
    }
    
    static const GUID CLSID_MMDeviceEnumerator_ = {0xBCDE0395,0xE52F,0x467C,{0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E}}; static const GUID IID_IMMDeviceEnumerator_ = {0xA95664D2,0x9614,0x4F35,{0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6}};
    FHandle pcm_open_all(int rate,int channels,int period_frames,int periods) {
        CoInitializeEx(NULL,0); IMMDeviceEnumerator *en = NULL; if (FAILED(CoCreateInstance(&CLSID_MMDeviceEnumerator_,NULL,23,&IID_IMMDeviceEnumerator_,(void**)&en))) { DualLogError("CoCreateInstance fail\n"); return INVALID_FHANDLE; }
        IMMDevice *dev = NULL; i32 hr = en->lpVtbl->GetDefaultAudioEndpoint(en,0,0,&dev); en->lpVtbl->Release(en); if (FAILED(hr)||!dev) return INVALID_FHANDLE;
        int idx = wasapi_init_device(dev,rate,channels,period_frames,periods); dev->lpVtbl->Release(dev); if (idx<0) return INVALID_FHANDLE;
        return IDX_TO_FD(0);
    }

    int pcm_sync(FHandle fd, pcm_sync_t *sync) { int idx=FD_TO_IDX(fd); if(idx<0||idx>=wasapi_dev_count||!wasapi_devs[idx].open){return -1;} wasapi_dev_t *w=&wasapi_devs[idx]; u32 padding=0; w->client->lpVtbl->GetCurrentPadding(w->client,&padding); snd_pcm_uframes_t base = (w->buffer_frames>(u32)(w->period_frames*4)) ? w->buffer_frames-(u32)(w->period_frames*4) : 0; sync->status.hw_ptr=base; sync->control.appl_ptr=base+padding; return 0; }
    int pcm_prepare(FHandle fd) { int i = FD_TO_IDX(fd); if (i < 0 || i >= wasapi_dev_count) return -1; wasapi_dev_t *w = &wasapi_devs[i]; return w->client->lpVtbl->Stop(w->client),w->client->lpVtbl->Reset(w->client),w->client->lpVtbl->Start(w->client), 0; }
    int pcm_write(void *buf, int frames) { for (int i=0;i<wasapi_dev_count;i++) { wasapi_dev_t *w=&wasapi_devs[i]; if(!w->open){continue;} u8* data = NULL; if(FAILED(w->render->lpVtbl->GetBuffer(w->render,(u32)frames,&data))){pcm_prepare(IDX_TO_FD(i)); continue;} mcpy(data,buf,frames*w->channels*2); w->render->lpVtbl->ReleaseBuffer(w->render,(u32)frames,0); } return frames; }
#else
    int ioctl(int fd, u64 request, ...);
    #define _IOC(dir, type, nr, size) (((dir) << 30) | ((type) << 8) | ((nr) << 0) | ((size) << 16))
    #define _IO(type, nr) _IOC(0U, (type), (nr), 0)
    #define _IOR(type, nr, size) _IOC(2U, (type), (nr), sizeof(size))
    #define _IOW(type, nr, size) _IOC(1U, (type), (nr), sizeof(size))
    #define _IOWR(type, nr, size) _IOC(2U | 1U, (type), (nr), sizeof(size))
    typedef u64 snd_pcm_uframes_t; typedef i64 snd_pcm_sframes_t; struct snd_mask { u32 bits[8]; }; struct snd_interval { u32 min,max,openmin:1, openmax:1, integer:1, empty:1; };
    struct snd_pcm_hw_params { u32 flags; struct snd_mask masks[3]; struct snd_mask mres[5]; struct snd_interval intervals[12]; struct snd_interval ires[9]; u32 rmask,cmask,info,msbits,rate_num,rate_den; snd_pcm_uframes_t fifo_size; u8 reserved[64]; };
    struct snd_pcm_sw_params { int tstamp_mode; u32 period_step,sleep_min; snd_pcm_uframes_t avail_min,xfer_align,start_threshold,stop_threshold,silence_threshold,silence_size,boundary; u32 proto,tstamp_type; u8 reserved[56]; };
    struct snd_pcm_mmap_status { int state,pad1; snd_pcm_uframes_t hw_ptr; struct timespec tstamp; int suspended_state; struct timespec audio_tstamp; };
    struct snd_pcm_mmap_control { snd_pcm_uframes_t appl_ptr; snd_pcm_uframes_t avail_min; };
    struct snd_pcm_sync_ptr { u32 flags; union { struct snd_pcm_mmap_status  status; u8 reserved[64]; } s; union { struct snd_pcm_mmap_control control; u8 reserved[64]; } c; };
    struct snd_pcm_status { int state; struct timespec trigger_tstamp; struct timespec tstamp; snd_pcm_uframes_t appl_ptr,hw_ptr; snd_pcm_sframes_t delay; snd_pcm_uframes_t avail,avail_max,overrange; int suspended_state; u32 audio_tstamp_data; struct timespec audio_tstamp; struct timespec driver_tstamp; u32 audio_tstamp_accuracy; u8 reserved[20]; };
    typedef struct snd_pcm_mmap_status  pcm_status_t; typedef struct snd_pcm_mmap_control pcm_control_t; typedef struct snd_pcm_hw_params pcm_hw_params_t; typedef struct snd_pcm_sw_params pcm_sw_params_t;
    struct pcm_params { pcm_hw_params_t hw_params; pcm_sw_params_t sw_params; }; typedef struct pcm_params pcm_params_t;
    typedef enum pcm_param {PCM_ACCESS=0,PCM_FORMAT=1,PCM_RATE=11,PCM_CHANNELS=10,PCM_PERIOD_SIZE=13,PCM_BUFFER_SIZE=17,PCM_PERIODS=15,PCM_INTERRUPT=20,PCM_TSTAMP_TYPE=21,PCM_AVAIL_MIN=22,PCM_START_THRESHOLD=23,PCM_XRUN_THRESHOLD=24,PCM_SILENCE_THRESHOLD=25,PCM_SILENCE_SIZE=26} pcm_param_t;
    INLINE struct snd_mask* get_mask_struct(struct snd_pcm_hw_params *p, u32 parameter) { return &p->masks[parameter - 0]; }
    INLINE struct snd_interval* get_interval_struct(struct snd_pcm_hw_params *p, u32 parameter) { return &p->intervals[parameter - 8]; }
    static void hw_params_set_mask(struct snd_pcm_hw_params *p, int parameter, u32 value) { struct snd_mask *m = get_mask_struct(p,parameter); if (m->bits[((value) / 32)] & (1 << ((value) % 32))) {mset(m, 0x00, sizeof(*m));} m->bits[((value) / 32)] |= (1 << ((value) % 32)); }
    static void hw_params_set_interval(struct snd_pcm_hw_params *p, int parameter, u32 min, u32 max) { struct snd_interval *i = get_interval_struct(p,parameter); i->openmin = i->openmax = 0; i->integer = 1; i->min = min; i->max = max; }
    static void hw_params_set(struct snd_pcm_hw_params *p, int parameter, u32 value) { if ((parameter >= 0 && parameter <= 2)) hw_params_set_mask(p,parameter,value); else if ((parameter >= 8 && parameter <= 19)) hw_params_set_interval(p,parameter,value,value); }
    static u32 hw_params_get_mask(struct snd_pcm_hw_params *p, int parameter, u32 value) { struct snd_mask *m=get_mask_struct(p,parameter); return m->bits[((value) / 32)] & (1 << ((value) % 32)); }
    static void hw_params_get_interval(struct snd_pcm_hw_params *p, int parameter, u32 *min, u32 *max) { struct snd_interval *i = get_interval_struct(p,parameter); *min = i->min + i->openmin; *max = i->max - i->openmax; }
    static u32 hw_params_get(struct snd_pcm_hw_params *p, int parameter, u32 value) { u32 r, t; return (parameter >= 0 && parameter <= 2) ? hw_params_get_mask(p,parameter,value) : ((parameter >= 8 && parameter <= 19) ? (hw_params_get_interval(p,parameter,&r,&t),r) : 0); }
    static void hw_params_fill(struct snd_pcm_hw_params *p) { mset(p,0,sizeof(*p)); mset(p->masks,0xff,sizeof(p->masks)); p->rmask = p->info = 0xffffffffU; for (int i=0;i<=11;i++) { p->intervals[i].min = 0; p->intervals[i].max = 0xffffffffU; } }
    unsigned long pcm_gethw(pcm_params_t *p, pcm_param_t param, u32 val) { return hw_params_get(&p->hw_params,param,val); }
    unsigned long pcm_getsw(pcm_params_t *p, pcm_param_t param) { pcm_sw_params_t *sw = &p->sw_params; return ((u64*)&sw->avail_min)[param - 22]; }
    int pcm_params_setup(int fd, pcm_params_t *p) {
        if (ioctl(fd,_IOWR('A',0x11,struct snd_pcm_hw_params),&p->hw_params) == -1) return -1;
        if (!pcm_getsw(p,22)) ((u64*)&p->sw_params.avail_min)[0] = pcm_gethw(p,13,0);
        if (!pcm_getsw(p,24)) ((u64*)&p->sw_params.avail_min)[2] = pcm_gethw(p,17,0);
        if (ioctl(fd,_IOW('A',0x03,int),&p->sw_params.tstamp_type) == -1) return -1; // Support ancient kernels
        if (ioctl(fd,_IOWR('A',0x13,struct snd_pcm_sw_params),&p->sw_params) == -1) return -1;
        return ioctl(fd,_IO('A',0x40));
    }

    int pcm_open(int card, int device, int flags) { char path[4096]; sFormat(path,sizeof(path),"/dev/snd/pcmC%uD%u%c",card,device,(flags & 1) == 0 ? 'c' : 'p'); return OS_Open(path,00000002 | (flags & (1 << 1) ? 00004000 : 0),0); }
#endif
#define MP3_HDR_IS_MONO(h)             (((h[3]) & 0xC0) == 0xC0)
#define MP3_HDR_IS_MS_STEREO(h)        (((h[3]) & 0xE0) == 0x60)
#define MP3_HDR_IS_CRC(h)              (!((h[1]) & 1))
#define MP3_HDR_TEST_PADDING(h)        ((h[2]) & 0x2)
#define MP3_HDR_TEST_MPEG1(h)          ((h[1]) & 0x8)
#define MP3_HDR_TEST_NOT_MPEG25(h)     ((h[1]) & 0x10)
#define MP3_HDR_TEST_I_STEREO(h)       ((h[3]) & 0x10)
#define MP3_HDR_TEST_MS_STEREO(h)      ((h[3]) & 0x20)
#define MP3_HDR_GET_STEREO_MODE(h)     (((h[3]) >> 6) & 3)
#define MP3_HDR_GET_STEREO_MODE_EXT(h) (((h[3]) >> 4) & 3)
#define MP3_HDR_GET_LAYER(h)           (((h[1]) >> 1) & 3)
#define MP3_HDR_GET_BITRATE(h)         ((h[2]) >> 4)
#define MP3_HDR_GET_SAMPLE_RATE(h)     (((h[2]) >> 2) & 3)
#define MP3_HDR_GET_SAMPLE_RATEHDR(h)  (MP3_HDR_GET_SAMPLE_RATE(h) + (((h[1]>>3)&1)+((h[1]>>4)&1))*3)
#define MP3_HDR_IS_FRAME_576(h)        ((h[1] & 14) == 2)
#define MP3_HDR_IS_LAYER_1(h)          ((h[1] & 6) == 6)
#define MP3_OFFSET_PTR(p,offset) ((void*)((u8*)(p)+(offset)))
static u8 g_halfrate[2][3][15]={ {{0,4,8,12,16,20,24,28,32,40,48,56,64,72,80},{0,4,8,12,16,20,24,28,32,40,48,56,64,72,80},{0,16,24,28,32,40,48,56,64,72,80,88,96,112,128}},{{0,16,20,24,28,32,40,48,56,64,80,96,112,128,160},{0,16,24,28,32,40,48,56,64,80,96,112,128,160,192},{0,16,32,48,64,80,96,112,128,144,160,176,192,208,224}} };
static u8 g_scf_long[8][23]={{0},{12,12,12,12,12,12,16,20,24,28,32,40,48,56,64,76,90,2,2,2,2,2,0},{0},{6,6,6,6,6,6,8,10,12,14,16,18,22,26,32,38,46,54,62,70,76,36,0},{0},{4,4,4,4,4,4,6,6,8,8,10,12,16,20,24,28,34,42,50,54,76,158,0},{4,4,4,4,4,4,6,6,6,8,10,12,16,18,22,28,34,40,46,54,54,192,0},{4,4,4,4,4,4,6,6,8,10,12,16,20,24,30,38,46,56,68,84,102,26,0}};
static u8 g_scf_short[8][40]={{4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0},{8,8,8,8,8,8,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0},{4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0 },{4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0 }, { 4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },{4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0},{4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0},{4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0}};
static u8 g_scf_mixed[8][40]={{6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0},{12,12,12,4,4,4,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0},{6,6,6,6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0},{6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0},{6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0},{4,4,4,4,4,4,6,6,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0},{4,4,4,4,4,4,6,6,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0},{4,4,4,4,4,4,6,6,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0}};
static const u8 g_sfc_long_024[23] = { 6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54,0 };
static const u8 g_scf_partitions[3][28]={{6,5,5,5,6,5,5,5,6,5,7,3,11,10,0,0,7,7,7,0,6,6,6,3,8,8,5,0}, {8,9,6,12,6,9,9,9,6,9,12,6,15,18,0,0,6,15,12,0,6,12,9,6,6,18,9,0}, {9,9,6,12,9,9,9,9,9,9,12,6,18,18,0,0,12,12,12,0,12,9,9,6,15,12,9,0}};
static const float g_mp3_pow43[129+16]={ 0,-1,-2.519842f,-4.326749f,-6.349604f,-8.549880f,-10.902724f,-13.390518f,-16.000000f,-18.720754f,-21.544347f,-24.463781f,-27.473142f,-30.567351f,-33.741992f,-36.993181f,0,1,2.519842f,4.326749f,6.349604f,8.549880f,10.902724f,13.390518f,16.000000f,18.720754f,21.544347f,24.463781f,27.473142f,30.567351f,33.741992f,36.993181f,40.317474f,43.711787f,47.173345f,50.699631f,54.288352f,57.937408f,61.644865f,65.408941f,69.227979f,73.100443f,77.024898f,81.000000f,85.024491f,89.097188f,93.216975f,97.382800f,101.593667f,105.848633f,110.146801f,114.487321f,118.869381f,123.292209f,127.755065f,132.257246f,136.798076f,141.376907f,145.993119f,150.646117f,155.335327f,160.060199f,164.820202f,169.614826f,174.443577f,179.305980f,184.201575f,189.129918f,194.090580f,199.083145f,204.107210f,209.162385f,214.248292f,219.364564f,224.510845f,229.686789f,234.892058f,240.126328f,245.389280f,250.680604f,256.000000f,261.347174f,266.721841f,272.123723f,277.552547f,283.008049f,288.489971f,293.998060f,299.532071f,305.091761f,310.676898f,316.287249f,321.922592f,327.582707f,333.267377f,338.976394f,344.709550f,350.466646f,356.247482f,362.051866f,367.879608f,373.730522f,379.604427f,385.501143f,391.420496f,397.362314f,403.326427f,409.312672f,415.320884f,421.350905f,427.402579f,433.475750f,439.570269f,445.685987f,451.822757f,457.980436f,464.158883f,470.357960f,476.577530f,482.817459f,489.077615f,495.357868f,501.658090f,507.978156f,514.317941f,520.677324f,527.056184f,533.454404f,539.871867f,546.308458f,552.764065f,559.238575f,565.731879f,572.243870f,578.774440f,585.323483f,591.890898f,598.476581f,605.080431f,611.702349f,618.342238f,625.000000f,631.675540f,638.368763f,645.079578f };
static const i16 tabs[]={ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,785,785,785,785,784,784,784,784,513,513,513,513,513,513,513,513,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-255,1313,1298,1282,785,785,785,785,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,290,288,-255,1313,1298,1282,769,769,769,769,529,529,529,529,529,529,529,529,528,528,528,528,528,528,528,528,512,512,512,512,512,512,512,512,290,288,-253,-318,-351,-367,785,785,785,785,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,819,818,547,547,275,275,275,275,561,560,515,546,289,274,288,258,
    -254,-287,1329,1299,1314,1312,1057,1057,1042,1042,1026,1026,784,784,784,784,529,529,529,529,529,529,529,529,769,769,769,769,768,768,768,768,563,560,306,306,291,259,-252,-413,-477,-542,1298,-575,1041,1041,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-383,-399,1107,1092,1106,1061,849,849,789,789,1104,1091,773,773,1076,1075,341,340,325,309,834,804,577,577,532,532,516,516,832,818,803,816,561,561,531,531,515,546,289,289,288,258,-252,-429,-493,-559,1057,1057,1042,1042,529,529,529,529,529,529,529,529,784,784,784,784,769,769,769,769,512,512,512,512,512,512,512,512,-382,1077,-415,1106,1061,1104,849,849,789,789,1091,1076,1029,1075,834,834,597,581,340,340,339,324,804,833,532,532,832,772,818,803,817,787,816,771,290,290,290,290,288,258,
    -253,-349,-414,-447,-463,1329,1299,-479,1314,1312,1057,1057,1042,1042,1026,1026,785,785,785,785,784,784,784,784,769,769,769,769,768,768,768,768,-319,851,821,-335,836,850,805,849,341,340,325,336,533,533,579,579,564,564,773,832,578,548,563,516,321,276,306,291,304,259,-251,-572,-733,-830,-863,-879,1041,1041,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-511,-527,-543,1396,1351,1381,1366,1395,1335,1380,-559,1334,1138,1138,1063,1063,1350,1392,1031,1031,1062,1062,1364,1363,1120,1120,1333,1348,881,881,881,881,375,374,359,373,343,358,341,325,791,791,1123,1122,-703,1105,1045,-719,865,865,790,790,774,774,1104,1029,338,293,323,308,-799,-815,833,788,772,818,803,816,322,292,307,320,561,531,515,546,289,274,288,258,
    -251,-525,-605,-685,-765,-831,-846,1298,1057,1057,1312,1282,785,785,785,785,784,784,784,784,769,769,769,769,512,512,512,512,512,512,512,512,1399,1398,1383,1367,1382,1396,1351,-511,1381,1366,1139,1139,1079,1079,1124,1124,1364,1349,1363,1333,882,882,882,882,807,807,807,807,1094,1094,1136,1136,373,341,535,535,881,775,867,822,774,-591,324,338,-671,849,550,550,866,864,609,609,293,336,534,534,789,835,773,-751,834,804,308,307,833,788,832,772,562,562,547,547,305,275,560,515,290,290,-252,-397,-477,-557,-622,-653,-719,-735,-750,1329,1299,1314,1057,1057,1042,1042,1312,1282,1024,1024,785,785,785,785,784,784,784,784,769,769,769,769,-383,1127,1141,1111,1126,1140,1095,1110,869,869,883,883,1079,1109,882,882,375,374,807,868,838,881,791,-463,867,822,368,263,852,837,836,-543,610,610,550,550,352,336,534,534,865,774,851,821,850,805,593,533,579,564,773,832,578,578,548,548,577,577,307,276,306,291,516,560,259,259,
    -250,-2107,-2507,-2764,-2909,-2974,-3007,-3023,1041,1041,1040,1040,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-767,-1052,-1213,-1277,-1358,-1405,-1469,-1535,-1550,-1582,-1614,-1647,-1662,-1694,-1726,-1759,-1774,-1807,-1822,-1854,-1886,1565,-1919,-1935,-1951,-1967,1731,1730,1580,1717,-1983,1729,1564,-1999,1548,-2015,-2031,1715,1595,-2047,1714,-2063,1610,-2079,1609,-2095,1323,1323,1457,1457,1307,1307,1712,1547,1641,1700,1699,1594,1685,1625,1442,1442,1322,1322,-780,-973,-910,1279,1278,1277,1262,1276,1261,1275,1215,1260,1229,-959,974,974,989,989,-943,735,478,478,495,463,506,414,-1039,1003,958,1017,927,942,987,957,431,476,1272,1167,1228,-1183,1256,-1199,895,895,941,941,1242,1227,1212,1135,1014,1014,490,489,503,487,910,1013,985,925,863,894,970,955,1012,847,-1343,831,755,755,984,909,428,366,754,559,-1391,752,486,457,924,997,698,698,983,893,740,740,908,877,739,739,667,667,953,938,497,287,271,271,683,606,590,712,726,574,302,302,738,736,481,286,526,725,605,711,636,724,696,651,589,681,666,710,364,467,573,695,466,466,301,465,379,379,709,604,665,679,316,316,634,633,436,436,464,269,424,394,452,332,438,363,347,408,393,448,331,422,362,407,392,421,346,406,391,376,375,359,1441,1306,-2367,1290,-2383,1337,-2399,-2415,1426,1321,-2431,1411,1336,-2447,-2463,-2479,1169,1169,1049,1049,1424,1289,1412,1352,1319,-2495,1154,1154,1064,1064,1153,1153,416,390,360,404,403,389,344,374,373,343,358,372,327,357,342,311,356,326,1395,1394,1137,1137,1047,1047,1365,1392,1287,1379,1334,1364,1349,1378,1318,1363,792,792,792,792,1152,1152,1032,1032,1121,1121,1046,1046,1120,1120,1030,1030,-2895,1106,1061,1104,849,849,789,789,1091,1076,1029,1090,1060,1075,833,833,309,324,532,532,832,772,818,803,561,561,531,560,515,546,289,274,288,258,
    -250,-1179,-1579,-1836,-1996,-2124,-2253,-2333,-2413,-2477,-2542,-2574,-2607,-2622,-2655,1314,1313,1298,1312,1282,785,785,785,785,1040,1040,1025,1025,768,768,768,768,-766,-798,-830,-862,-895,-911,-927,-943,-959,-975,-991,-1007,-1023,-1039,-1055,-1070,1724,1647,-1103,-1119,1631,1767,1662,1738,1708,1723,-1135,1780,1615,1779,1599,1677,1646,1778,1583,-1151,1777,1567,1737,1692,1765,1722,1707,1630,1751,1661,1764,1614,1736,1676,1763,1750,1645,1598,1721,1691,1762,1706,1582,1761,1566,-1167,1749,1629,767,766,751,765,494,494,735,764,719,749,734,763,447,447,748,718,477,506,431,491,446,476,461,505,415,430,475,445,504,399,460,489,414,503,383,474,429,459,502,502,746,752,488,398,501,473,413,472,486,271,480,270,-1439,-1455,1357,-1471,-1487,-1503,1341,1325,-1519,1489,1463,1403,1309,-1535,1372,1448,1418,1476,1356,1462,1387,-1551,1475,1340,1447,1402,1386,-1567,1068,1068,1474,1461,455,380,468,440,395,425,410,454,364,467,466,464,453,269,409,448,268,432,1371,1473,1432,1417,1308,1460,1355,1446,1459,1431,1083,1083,1401,1416,1458,1445,1067,1067,1370,1457,1051,1051,1291,1430,1385,1444,1354,1415,1400,1443,1082,1082,1173,1113,1186,1066,1185,1050,-1967,1158,1128,1172,1097,1171,1081,-1983,1157,1112,416,266,375,400,1170,1142,1127,1065,793,793,1169,1033,1156,1096,1141,1111,1155,1080,1126,1140,898,898,808,808,897,897,792,792,1095,1152,1032,1125,1110,1139,1079,1124,882,807,838,881,853,791,-2319,867,368,263,822,852,837,866,806,865,-2399,851,352,262,534,534,821,836,594,594,549,549,593,593,533,533,848,773,579,579,564,578,548,563,276,276,577,576,306,291,516,560,305,305,275,259,
    -251,-892,-2058,-2620,-2828,-2957,-3023,-3039,1041,1041,1040,1040,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-511,-527,-543,-559,1530,-575,-591,1528,1527,1407,1526,1391,1023,1023,1023,1023,1525,1375,1268,1268,1103,1103,1087,1087,1039,1039,1523,-604,815,815,815,815,510,495,509,479,508,463,507,447,431,505,415,399,-734,-782,1262,-815,1259,1244,-831,1258,1228,-847,-863,1196,-879,1253,987,987,748,-767,493,493,462,477,414,414,686,669,478,446,461,445,474,429,487,458,412,471,1266,1264,1009,1009,799,799,-1019,-1276,-1452,-1581,-1677,-1757,-1821,-1886,-1933,-1997,1257,1257,1483,1468,1512,1422,1497,1406,1467,1496,1421,1510,1134,1134,1225,1225,1466,1451,1374,1405,1252,1252,1358,1480,1164,1164,1251,1251,1238,1238,1389,1465,-1407,1054,1101,-1423,1207,-1439,830,830,1248,1038,1237,1117,1223,1148,1236,1208,411,426,395,410,379,269,1193,1222,1132,1235,1221,1116,976,976,1192,1162,1177,1220,1131,1191,963,963,-1647,961,780,-1663,558,558,994,993,437,408,393,407,829,978,813,797,947,-1743,721,721,377,392,844,950,828,890,706,706,812,859,796,960,948,843,934,874,571,571,-1919,690,555,689,421,346,539,539,944,779,918,873,932,842,903,888,570,570,931,917,674,674,-2575,1562,-2591,1609,-2607,1654,1322,1322,1441,1441,1696,1546,1683,1593,1669,1624,1426,1426,1321,1321,1639,1680,1425,1425,1305,1305,1545,1668,1608,1623,1667,1592,1638,1666,1320,1320,1652,1607,1409,1409,1304,1304,1288,1288,1664,1637,1395,1395,1335,1335,1622,1636,1394,1394,1319,1319,1606,1621,1392,1392,1137,1137,1137,1137,345,390,360,375,404,373,1047,-2751,-2767,-2783,1062,1121,1046,-2799,1077,-2815,1106,1061,789,789,1105,1104,263,355,310,340,325,354,352,262,339,324,1091,1076,1029,1090,1060,1075,833,833,788,788,1088,1028,818,818,803,803,561,561,531,531,816,771,546,546,289,274,288,258,
    -253,-317,-381,-446,-478,-509,1279,1279,-811,-1179,-1451,-1756,-1900,-2028,-2189,-2253,-2333,-2414,-2445,-2511,-2526,1313,1298,-2559,1041,1041,1040,1040,1025,1025,1024,1024,1022,1007,1021,991,1020,975,1019,959,687,687,1018,1017,671,671,655,655,1016,1015,639,639,758,758,623,623,757,607,756,591,755,575,754,559,543,543,1009,783,-575,-621,-685,-749,496,-590,750,749,734,748,974,989,1003,958,988,973,1002,942,987,957,972,1001,926,986,941,971,956,1000,910,985,925,999,894,970,-1071,-1087,-1102,1390,-1135,1436,1509,1451,1374,-1151,1405,1358,1480,1420,-1167,1507,1494,1389,1342,1465,1435,1450,1326,1505,1310,1493,1373,1479,1404,1492,1464,1419,428,443,472,397,736,526,464,464,486,457,442,471,484,482,1357,1449,1434,1478,1388,1491,1341,1490,1325,1489,1463,1403,1309,1477,1372,1448,1418,1433,1476,1356,1462,1387,-1439,1475,1340,1447,1402,1474,1324,1461,1371,1473,269,448,1432,1417,1308,1460,-1711,1459,-1727,1441,1099,1099,1446,1386,1431,1401,-1743,1289,1083,1083,1160,1160,1458,1445,1067,1067,1370,1457,1307,1430,1129,1129,1098,1098,268,432,267,416,266,400,-1887,1144,1187,1082,1173,1113,1186,1066,1050,1158,1128,1143,1172,1097,1171,1081,420,391,1157,1112,1170,1142,1127,1065,1169,1049,1156,1096,1141,1111,1155,1080,1126,1154,1064,1153,1140,1095,1048,-2159,1125,1110,1137,-2175,823,823,1139,1138,807,807,384,264,368,263,868,838,853,791,867,822,852,837,866,806,865,790,-2319,851,821,836,352,262,850,805,849,-2399,533,533,835,820,336,261,578,548,563,577,532,532,832,772,562,562,547,547,305,275,560,515,290,290,288,258};
static const u8 tab32[]={130,162,193,209,44,28,76,140,9,9,9,9,9,9,9,9,190,254,222,238,126,94,157,157,109,61,173,205}; static const u8 tab33[]={252,236,220,204,188,172,156,140,124,108,92,76,60,44,28,12};
static const i16 tabindex[2*16]={0,32,64,98,0,132,180,218,292,364,426,538,648,746,0,1126,1460,1460,1460,1460,1460,1460,1460,1460,1842,1842,1842,1842,1842,1842,1842,1842}; static const u8 g_linbits[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,6,8,10,13,4,5,6,7,8,9,11,13};
static const float g_sec[24]={10.19000816f,0.50060302f,0.50241929f,3.40760851f,0.50547093f,0.52249861f,2.05778098f,0.51544732f,0.56694406f,1.48416460f,0.53104258f,0.64682180f,1.16943991f,0.55310392f,0.78815460f,0.97256821f,0.58293498f,1.06067765f,0.83934963f,0.62250412f,1.72244716f,0.74453628f,0.67480832f,5.10114861f};
static const float g_win[]={ -1,26,-31,208,218,401,-519,2063,2000,4788,-5517,7134,5959,35640,-39336,74992,-1,24,-35,202,222,347,-581,2080,1952,4425,-5879,7640,5288,33791,-41176,74856,-1,21,-38,196,225,294,-645,2087,1893,4063,-6237,8092,4561,31947,-43006,74630,-1,19,-41,190,227,244,-711,2085,1822,3705,-6589,8492,3776,30112,-44821,74313,-1,17,-45,183,228,197,-779,2075,1739,3351,-6935,8840,2935,28289,-46617,73908,-1,16,-49,176,228,153,-848,2057,1644,3004,-7271,9139,2037,26482,-48390,73415,-2,14,-53,169,227,111,-919,2032,1535,2663,-7597,9389,1082,24694,-50137,72835,-2,13,-58,161,224,72,-991,2001,1414,2330,-7910,9592,70,22929,-51853,72169,-2,11,-63,154,221,36,-1064,1962,1280,2006,-8209,9750,-998,21189,-53534,71420,-2,10,-68,147,215,2,-1137,1919,1131,1692,-8491,9863,-2122,19478,-55178,70590,-3,9,-73,139,208,-29,-1210,1870,970,1388,-8755,9935,-3300,17799,-56778,69679,-3,8,-79,132,200,-57,-1283,1817,794,1095,-8998,9966,-4533,16155,-58333,68692,-4,7,-85,125,189,-83,-1356,1759,605,814,-9219,9959,-5818,14548,-59838,67629,-4,7,-91,117,177,-106,-1428,1698,402,545,-9416,9916,-7154,12980,-61289,66494,-5,6,-97,111,163,-127,-1498,1634,185,288,-9585,9838,-8540,11455,-62684,65290};
typedef struct { int frame_bytes,channels,sample_rate,layer,bitrate_kbps; } mp3dec_frame_info;
typedef struct { const u8 *buf; int pos,limit; } mp3_bs;
typedef struct { const u8 *sfbtab; u16 part_23_length,big_values,scalefac_compress; u8 global_gain,block_type,mixed_block_flag,n_long_sfb,n_short_sfb,table_select[3],region_count[3],subblock_gain[3],preflag,scalefac_scale,count1_table,scfsi; } mp3L3_gr_info;
typedef struct { mp3_bs bs; u8 maindata[511 + 2304]; mp3L3_gr_info gr_info[4]; float grbuf[2][576],scf[40],syn[18+15][2*32]; u8 ist_pos[2][39]; } mp3dec_scratch;
typedef struct { float mdct_overlap[2][9*32], qmf_state[15*2*32]; int reserv; u8 header[4],reserv_buf[511]; mp3dec_scratch scratch; } mp3dec;
typedef struct { mp3dec decoder; u32 channels,sampleRate,mp3FChan,mp3FrameSampleRate,pcmFConsInMP3F,pcmFRemInMP3F,delayInPCMFrames,paddingInPCMFrames; void *pUserData; u8 pcmFrames[sizeof(float) * (1152 * 2)]; u64 currentPCMFrame,streamCursor,streamLength,streamStartOffset,totalPCMFrameCount; bool atEnd; size_t dataSize,dataCapacity,dataConsumed; u8 *pData; } mp3;
static u32 mp3_bs_get_bits(mp3_bs *bs, int n) { u32 next,cache=0, s=bs->pos&7; int shl=n+s; const u8 *p=bs->buf+(bs->pos>>3); if ((bs->pos+=n)>bs->limit) {return 0;} next=*p++&(255>>s); while ((shl-=8)>0) { cache|=next<<shl; next=*p++; } return cache|(next>>-shl); }
static int mp3_hdr_valid(const u8 *h) { int bitrate_idx=MP3_HDR_GET_BITRATE(h); return h[0]==0xff && ((h[1]&0xF0)==0xf0||(h[1]&0xFE)==0xe2) && (MP3_HDR_GET_LAYER(h)!=0) && (bitrate_idx!=0) && (bitrate_idx!=15) && (MP3_HDR_GET_SAMPLE_RATE(h)!=3); }
static int mp3_hdr_compare(const u8 *h1, const u8 *h2) { return mp3_hdr_valid(h2) && ((h1[1]^h2[1])&0xFE)==0 && ((h1[2]^h2[2])&0x0C)==0; }
static unsigned mp3_hdr_bitrate_kbps(const u8 *h) { return 2*g_halfrate[!!MP3_HDR_TEST_MPEG1(h)][(((h[1]) >> 1) & 3)-1/*layer*/][((h[2]) >> 4)/*bitrate*/]; }
static unsigned mp3_hdr_sample_rate_hz(const u8 *h) { static const unsigned g_hz[3]={44100,48000,32000}; return g_hz[MP3_HDR_GET_SAMPLE_RATE(h)]>>(int)!MP3_HDR_TEST_MPEG1(h)>>(int)!MP3_HDR_TEST_NOT_MPEG25(h); }
static unsigned mp3_hdr_frame_samples(const u8 *h) { return ((h[1]&6) == 6) ? 384 : (1152>>(int)MP3_HDR_IS_FRAME_576(h)); }
static int mp3_hdr_frame_bytes(const u8 *h) { int fb=mp3_hdr_frame_samples(h)*mp3_hdr_bitrate_kbps(h)*125/mp3_hdr_sample_rate_hz(h); if (MP3_HDR_IS_LAYER_1(h)) {fb&=~3;} return fb; }
static int mp3_hdr_padding(const u8 *h) { return MP3_HDR_TEST_PADDING(h)?(MP3_HDR_IS_LAYER_1(h)?4:1):0; }
void InitSCFTables() { for (int i=0;i<23;++i) g_scf_long[0][i] = g_scf_long[1][i] = g_scf_long[2][i] = g_sfc_long_024[i]; }
static int mp3L3_read_side_info(mp3_bs *bs, mp3L3_gr_info *gr, const u8 *hdr) {
    unsigned tables,scfsi=0; int main_data_begin,part_23_sum = 0, gr_count=MP3_HDR_IS_MONO(hdr) ? 1 : 2, sr_idx=MP3_HDR_GET_SAMPLE_RATEHDR(hdr); sr_idx-=(sr_idx!=0);
    if (MP3_HDR_TEST_MPEG1(hdr)) { gr_count*=2; main_data_begin=mp3_bs_get_bits(bs,9); scfsi=mp3_bs_get_bits(bs,7+gr_count); }
    else main_data_begin = mp3_bs_get_bits(bs,8+gr_count)>>gr_count;
    do {
        if (MP3_HDR_IS_MONO(hdr)) scfsi<<=4;
        gr->part_23_length=(u16)mp3_bs_get_bits(bs,12); part_23_sum+=gr->part_23_length; gr->big_values=(u16)mp3_bs_get_bits(bs,9); if (gr->big_values>288) return -1;
        gr->global_gain=(u8)mp3_bs_get_bits(bs,8); gr->scalefac_compress=(u16)mp3_bs_get_bits(bs,MP3_HDR_TEST_MPEG1(hdr)?4:9); gr->sfbtab=g_scf_long[sr_idx]; gr->n_long_sfb=22; gr->n_short_sfb=0;
        if (mp3_bs_get_bits(bs,1)) {
            gr->block_type = (u8)mp3_bs_get_bits(bs,2); if (!gr->block_type) return -1;
            gr->mixed_block_flag = (u8)mp3_bs_get_bits(bs,1); gr->region_count[0] = 7; gr->region_count[1] = 255;
            if (gr->block_type==2) {
                scfsi&=0x0F0F;
                if (!gr->mixed_block_flag) { gr->region_count[0] = 8; gr->sfbtab = g_scf_short[sr_idx]; gr->n_long_sfb = 0; gr->n_short_sfb = 39; }
                else                       { gr->sfbtab = g_scf_mixed[sr_idx]; gr->n_long_sfb=MP3_HDR_TEST_MPEG1(hdr) ? 8 : 6; gr->n_short_sfb = 30; }
            }
            tables=mp3_bs_get_bits(bs,10)<<5;
            gr->subblock_gain[0]=(u8)mp3_bs_get_bits(bs,3); gr->subblock_gain[1]=(u8)mp3_bs_get_bits(bs,3); gr->subblock_gain[2]=(u8)mp3_bs_get_bits(bs,3);
        } else { gr->block_type=0; gr->mixed_block_flag=0; tables=mp3_bs_get_bits(bs,15); gr->region_count[0]=(u8)mp3_bs_get_bits(bs,4); gr->region_count[1]=(u8)mp3_bs_get_bits(bs,3); gr->region_count[2]=255; }
        gr->table_select[0]=(u8)(tables>>10); gr->table_select[1]=(u8)((tables>>5)&31); gr->table_select[2]=(u8)((tables)&31);
        gr->preflag=(u8)(MP3_HDR_TEST_MPEG1(hdr) ? mp3_bs_get_bits(bs,1) : (gr->scalefac_compress>=500));
        gr->scalefac_scale=(u8)mp3_bs_get_bits(bs,1); gr->count1_table=(u8)mp3_bs_get_bits(bs,1); gr->scfsi=(u8)((scfsi>>12)&15);
        scfsi <<= 4; gr++;
    } while(--gr_count);
    if (part_23_sum+bs->pos > bs->limit+main_data_begin*8) return -1;
    return main_data_begin;
}

static void mp3L3_read_scalefactors(u8 *scf, u8 *ist_pos, const u8 *scf_size, const u8 *scf_count, mp3_bs *bs, int scfsi) {
    for (int i=0; i<4&&scf_count[i]; i++,scfsi*=2) {
        int cnt = scf_count[i];
        if (scfsi & 8) {mcpy(scf,ist_pos,cnt);} else { int bits=scf_size[i]; if(!bits){mset(scf,0,cnt); mset(ist_pos,0,cnt);} else {int max_scf=(scfsi<0)?((1<<bits)-1):-1; for (int k=0;k<cnt;k++) {int s=mp3_bs_get_bits(bs,bits); ist_pos[k]=(u8)(s==max_scf?-1:s); scf[k]=(u8)s;} } }
        ist_pos+=cnt; scf+=cnt;
    }
    scf[0]=scf[1]=scf[2]=0;
}

static float mp3L3_ldexp_q2(float y, int exp_q2) { static const float g_expfrac[4]={9.31322575e-10f,7.83145814e-10f,6.58544508e-10f,5.53767716e-10f}; int e; do { e=vmin(30*4,exp_q2); y*=g_expfrac[e&3]*(1<<30>>(e>>2)); } while ((exp_q2-=e)>0); return y; }
#define MP3_MAX_SCFI (((255+-1*4-210)+3)&~3)
static void mp3L3_decode_scalefactors(const u8 *hdr, u8 *ist_pos, mp3_bs *bs, const mp3L3_gr_info *gr, float *scf, int ch) {
    const u8 *scf_partition=g_scf_partitions[!!gr->n_short_sfb+!gr->n_long_sfb];
    u8 scf_size[4],iscf[40]; int i,scf_shift=gr->scalefac_scale+1,gain_exp,scfsi=gr->scfsi; float gain;
    if (MP3_HDR_TEST_MPEG1(hdr)) { static const u8 g_scfc_decode[16]={0,1,2,3,12,5,6,7,9,10,11,13,14,15,18,19}; int part=g_scfc_decode[gr->scalefac_compress]; scf_size[1]=scf_size[0]=(u8)(part>>2); scf_size[3]=scf_size[2]=(u8)(part&3); }
    else {
        static const u8 g_mod[6*4]={5,5,4,4,5,5,4,1,4,3,1,1,5,6,6,1,4,4,4,1,4,3,1,1};
        int k,modprod,sfc,ist=MP3_HDR_TEST_I_STEREO(hdr)&&ch;
        sfc=gr->scalefac_compress>>ist;
        for (k=ist*3*4; sfc>=0; sfc-=modprod,k+=4) { for (modprod=1,i=3;i>=0;i--) {scf_size[i]=(u8)(sfc/modprod%g_mod[k+i]); modprod*=g_mod[k+i];} }
        scf_partition+=k; scfsi=-16;
    }
    mp3L3_read_scalefactors(iscf,ist_pos,scf_size,scf_partition,bs,scfsi);
    if (gr->n_short_sfb) {
        int sh=3-scf_shift;
        for (i=0;i<gr->n_short_sfb;i+=3) { iscf[gr->n_long_sfb+i+0]=(u8)(iscf[gr->n_long_sfb+i+0]+(gr->subblock_gain[0]<<sh)); iscf[gr->n_long_sfb+i+1]=(u8)(iscf[gr->n_long_sfb+i+1]+(gr->subblock_gain[1]<<sh)); iscf[gr->n_long_sfb+i+2]=(u8)(iscf[gr->n_long_sfb+i+2]+(gr->subblock_gain[2]<<sh)); }
    } else if (gr->preflag) { static const u8 g_preamp[10]={1,1,1,1,2,2,3,3,3,2}; for (i=0;i<10;i++) {iscf[11+i]=(u8)(iscf[11+i]+g_preamp[i]);} }
    gain_exp=gr->global_gain+-1*4-210-(MP3_HDR_IS_MS_STEREO(hdr)?2:0);
    gain=mp3L3_ldexp_q2(1<<(MP3_MAX_SCFI/4),MP3_MAX_SCFI-gain_exp);
    for (i=0;i<(int)(gr->n_long_sfb+gr->n_short_sfb);i++) scf[i]=mp3L3_ldexp_q2(gain,iscf[i]<<scf_shift);
}

static float mp3L3_pow_43(int x) { if(x<129){return g_mp3_pow43[16+x];} int mult=256; if(x<1024){mult=16; x<<=3;} int sign=2*x&64; float frac=(float)((x&63)-sign)/((x&~63)+sign); return g_mp3_pow43[16+((x+sign)>>6)]*(1.f+frac*((4.f/3)+frac*(2.f/9)))*mult; }
static void mp3L3_huffman(float *dst, mp3_bs *bs, const mp3L3_gr_info *gr_info, const float *scf, int layer3gr_limit) {
    #define MP3_FLUSH_BITS(n) { bs_cache<<=(n); bs_sh+=(n); }
    float one=0.0f; int ireg=0,big_val_cnt=gr_info->big_values; const u8 *sfb=gr_info->sfbtab; const u8 *bs_next_ptr=bs->buf+bs->pos/8;
    u32 bs_cache=(((bs_next_ptr[0]*256u+bs_next_ptr[1])*256u+bs_next_ptr[2])*256u+bs_next_ptr[3])<<(bs->pos&7); int pairs_to_decode,np,bs_sh=(bs->pos&7)-8; bs_next_ptr+=4;
    while (big_val_cnt>0) {
        int tab_num=gr_info->table_select[ireg], sfb_cnt=gr_info->region_count[ireg++]; const i16 *codebook=tabs+tabindex[tab_num]; int linbits=g_linbits[tab_num];
        if (linbits) {
            do {
                np=*sfb++/2; pairs_to_decode=vmin(big_val_cnt,np); one=*scf++;
                do {
                    int j,w=5,leaf=codebook[(bs_cache>>(32-w))];
                    while (leaf<0){MP3_FLUSH_BITS(w);w=leaf&7;leaf=codebook[(bs_cache>>(32-w))-(leaf>>3)];}
                    MP3_FLUSH_BITS(leaf>>8);
                    for (j=0;j<2;j++,dst++,leaf>>=4){
                        int lsb=leaf&0x0F;
                        if (lsb==15) { lsb += (bs_cache>>(32-(linbits))); MP3_FLUSH_BITS(linbits); while(bs_sh>=0){bs_cache|=(u32)*bs_next_ptr++<<bs_sh;bs_sh-=8;}; *dst= one * mp3L3_pow_43(lsb) * ((i32)bs_cache < 0 ? -1 : 1); }
                        else *dst=g_mp3_pow43[16+lsb-16*(bs_cache>>31)]*one;
                        MP3_FLUSH_BITS(lsb?1:0);
                    }
                    while(bs_sh>=0){bs_cache|=(u32)*bs_next_ptr++<<bs_sh;bs_sh-=8;};
                } while(--pairs_to_decode);
            } while((big_val_cnt-=np)>0&&--sfb_cnt>=0);
        } else {
            do {
                np=*sfb++/2; pairs_to_decode=vmin(big_val_cnt,np); one=*scf++;
                do {
                    int j,w=5,leaf=codebook[(bs_cache>>(32-w))];
                    while (leaf<0){MP3_FLUSH_BITS(w);w=leaf&7;leaf=codebook[(bs_cache>>(32-w))-(leaf>>3)];}
                    MP3_FLUSH_BITS(leaf>>8);
                    for (j=0;j<2;j++,dst++,leaf>>=4) { int lsb=leaf&0x0F; *dst=g_mp3_pow43[16+lsb-16*(bs_cache>>31)]*one; MP3_FLUSH_BITS(lsb?1:0); }
                    while(bs_sh>=0){bs_cache|=(u32)*bs_next_ptr++<<bs_sh;bs_sh-=8;};
                } while(--pairs_to_decode);
            } while((big_val_cnt-=np)>0&&--sfb_cnt>=0);
        }
    }
    for (np=1-big_val_cnt;;dst+=4) {
        const u8 *codebook_count1=(gr_info->count1_table)?tab33:tab32;
        int leaf=codebook_count1[(bs_cache>>28)];
        if (!(leaf&8)) leaf=codebook_count1[(leaf>>3)+(bs_cache<<4>>(32-(leaf&3)))];
        MP3_FLUSH_BITS(leaf&7);
        if (((bs_next_ptr-bs->buf)*8-24+bs_sh)>layer3gr_limit) break;
        if(!--np) { np=*sfb++/2; if(!np) {break;} one=*scf++; }; if(leaf&(128>>0)){dst[0]=((i32)bs_cache<0)?-one:one;MP3_FLUSH_BITS(1)} if(leaf&(128>>1)){dst[1]=((i32)bs_cache<0)?-one:one;MP3_FLUSH_BITS(1)}
        if(!--np) { np=*sfb++/2; if(!np) {break;} one=*scf++; }; if(leaf&(128>>2)){dst[2]=((i32)bs_cache<0)?-one:one;MP3_FLUSH_BITS(1)} if(leaf&(128>>3)){dst[3]=((i32)bs_cache<0)?-one:one;MP3_FLUSH_BITS(1)}
        while(bs_sh>=0){bs_cache|=(u32)*bs_next_ptr++<<bs_sh;bs_sh-=8;};
    }
    bs->pos=layer3gr_limit;
}

static void mp3L3_midside_stereo(float *l, int n) { int i=0; float *r=l+576; for (; i<n; i++) { float a=l[i],b=r[i]; l[i]=a+b; r[i]=a-b; } }
static void mp3L3_intensity_stereo_band(float *l, int n, float kl, float kr) { int i; for(i=0;i<n;i++){l[i+576]=l[i]*kr; l[i]=l[i]*kl;} }
static void mp3L3_stereo_top_band(const float *r, const u8 *sfb, int nbands, int max_band[3]) { int i,k; max_band[0]=max_band[1]=max_band[2]=-1; for (i=0;i<nbands;i++){for(k=0;k<sfb[i];k+=2){if(r[k]!=0||r[k+1]!=0){max_band[i%3]=i;break;}}r+=sfb[i];} }
static void mp3L3_stereo_process(float *left, const u8 *ist_pos, const u8 *sfb, const u8 *hdr, int max_band[3], int mpeg2_sh) {
    static const float g_pan[7*2]={0,1,0.21132487f,0.78867513f,0.36602540f,0.63397460f,0.5f,0.5f,0.63397460f,0.36602540f,0.78867513f,0.21132487f,1,0};
    unsigned i,max_pos=MP3_HDR_TEST_MPEG1(hdr)?7:64;
    for (i=0;sfb[i];i++){
        unsigned ipos=ist_pos[i];
        if ((int)i>max_band[i%3]&&ipos<max_pos){
            float kl,kr,s=MP3_HDR_TEST_MS_STEREO(hdr)?1.41421356f:1;
            if(MP3_HDR_TEST_MPEG1(hdr)){kl=g_pan[2*ipos];kr=g_pan[2*ipos+1];} else {kl=1;kr=mp3L3_ldexp_q2(1,(ipos+1)>>1<<mpeg2_sh);if(ipos&1){kl=kr;kr=1;}}
            mp3L3_intensity_stereo_band(left,sfb[i],kl*s,kr*s);
        } else if (MP3_HDR_TEST_MS_STEREO(hdr)) mp3L3_midside_stereo(left,sfb[i]);
        left+=sfb[i];
    }
}
static void mp3L3_intensity_stereo(float *left, u8 *ist_pos, const mp3L3_gr_info *gr, const u8 *hdr) {
    int max_band[3],n_sfb=gr->n_long_sfb+gr->n_short_sfb,i,max_blocks=gr->n_short_sfb?3:1;
    mp3L3_stereo_top_band(left+576,gr->sfbtab,n_sfb,max_band);
    if (gr->n_long_sfb) max_band[0]=max_band[1]=max_band[2]=vmax(vmax(max_band[0],max_band[1]),max_band[2]);
    for (i=0;i<max_blocks;i++){ int default_pos = MP3_HDR_TEST_MPEG1(hdr) ? 3 : 0, itop = n_sfb-max_blocks + i, prev = itop - max_blocks; ist_pos[itop] = (u8)(max_band[i] >= prev ? default_pos : ist_pos[prev]); }
    mp3L3_stereo_process(left,ist_pos,gr->sfbtab,hdr,max_band,gr[1].scalefac_compress&1);
}

static void mp3L3_reorder(float *grbuf, float *scratch, const u8 *sfb) { int i,len; float *src=grbuf,*dst=scratch; for(;0!=(len=*sfb);sfb+=3,src+=2*len){for(i=0;i<len;i++,src++){*dst++=src[0*len];*dst++=src[1*len];*dst++=src[2*len];}} mcpy(grbuf,scratch,(dst-scratch)*sizeof(float)); }
static void mp3L3_antialias(float *grbuf, int nbands) {
    static const float g_aa[2][8]={{0.85749293f,0.88174200f,0.94962865f,0.98331459f,0.99551782f,0.99916056f,0.99989920f,0.99999316f},{0.51449576f,0.47173197f,0.31337745f,0.18191320f,0.09457419f,0.04096558f,0.01419856f,0.00369997f}};
    for(;nbands>0;nbands--,grbuf+=18){ int i=0; for(;i<8;i++){float u=grbuf[18+i],d=grbuf[17-i];grbuf[18+i]=u*g_aa[0][i]-d*g_aa[1][i];grbuf[17-i]=u*g_aa[1][i]+d*g_aa[0][i];} }
}

static void mp3L3_dct3_9(float *y) { float s1,s3,s5,s7,t0,t2,t4,s0=y[0],s2=y[2],s4=y[4],s6=y[6],s8=y[8]; t0=s0+s6*0.5f; s0-=s6; t4=(s4+s2)*0.93969262f; t2=(s8+s2)*0.76604444f; s6=(s4-s8)*0.17364818f; s4+=s8-s2; s2=s0-s4*0.5f; y[4]=s4+s0; s8=t0-t2+s6; s0=t0-t4+t2; s4=t0+t4-s6; s1=y[1]; s3=y[3]; s5=y[5]; s7=y[7]; s3*=0.86602540f; t0=(s5+s1)*0.98480775f; t4=(s5-s7)*0.34202014f; t2=(s1+s7)*0.64278761f; s1=(s1-s5-s7)*0.86602540f; s5=t0-s3-t2; s7=t4-s3-t0; s3=t4+s3-t2; y[0]=s4-s7; y[1]=s2+s1; y[2]=s0-s3; y[3]=s8+s5; y[5]=s8-s5; y[6]=s0+s3; y[7]=s2-s1; y[8]=s4+s7; }
static void mp3L3_imdct36(float *grbuf, float *overlap, const float *win, int nbands) {
    int i,j;
    static const float g_twid9[18]={0.73727734f,0.79335334f,0.84339145f,0.88701083f,0.92387953f,0.95371695f,0.97629601f,0.99144486f,0.99904822f,0.67559021f,0.60876143f,0.53729961f,0.46174861f,0.38268343f,0.30070580f,0.21643961f,0.13052619f,0.04361938f};
    for (j=0;j<nbands;j++,grbuf+=18,overlap+=9){
        float co[9],si[9];
        co[0]=-grbuf[0]; si[0]=grbuf[17];
        for(i=0;i<4;i++){si[8-2*i]=grbuf[4*i+1]-grbuf[4*i+2];co[1+2*i]=grbuf[4*i+1]+grbuf[4*i+2];si[7-2*i]=grbuf[4*i+4]-grbuf[4*i+3];co[2+2*i]=-(grbuf[4*i+3]+grbuf[4*i+4]);}
        mp3L3_dct3_9(co); mp3L3_dct3_9(si);
        si[1]=-si[1];si[3]=-si[3];si[5]=-si[5];si[7]=-si[7];
        i=0;
        for(;i<9;i++){float ovl=overlap[i],sum=co[i]*g_twid9[9+i]+si[i]*g_twid9[i];overlap[i]=co[i]*g_twid9[i]-si[i]*g_twid9[9+i];grbuf[i]=ovl*win[i]-sum*win[9+i];grbuf[17-i]=ovl*win[9+i]+sum*win[i];}
    }
}

static void mp3L3_idct3(float x0,float x1,float x2,float *dst){float m1=x1*0.86602540f,a1=x0-x2*0.5f;dst[1]=x0+x2;dst[0]=a1+m1;dst[2]=a1-m1;}
static void imdct12(float *x,float *dst,float *overlap){
    static const float g_twid3[6]={0.79335334f,0.92387953f,0.99144486f,0.60876143f,0.38268343f,0.13052619f};
    float co[3],si[3]; int i; mp3L3_idct3(-x[0],x[6]+x[3],x[12]+x[9],co); mp3L3_idct3(x[15],x[12]-x[9],x[6]-x[3],si);
    si[1]=-si[1];
    for(i=0;i<3;i++){float ovl=overlap[i],sum=co[i]*g_twid3[3+i]+si[i]*g_twid3[i];overlap[i]=co[i]*g_twid3[i]-si[i]*g_twid3[3+i];dst[i]=ovl*g_twid3[2-i]-sum*g_twid3[5-i];dst[5-i]=ovl*g_twid3[5-i]+sum*g_twid3[2-i];}
}

static void mp3L3_imdct_short(float *grbuf,float *overlap,int nbands){ for(;nbands>0;nbands--,overlap+=9,grbuf+=18){float tmp[18]; mcpy(tmp,grbuf,sizeof(tmp)); mcpy(grbuf,overlap,6*sizeof(float)); imdct12(tmp,grbuf+6,overlap+6); imdct12(tmp+1,grbuf+12,overlap+6); imdct12(tmp+2,overlap,overlap+6);} }
static void mp3L3_change_sign(float *grbuf){int b,i;for(b=0,grbuf+=18;b<32;b+=2,grbuf+=36)for(i=1;i<18;i+=2)grbuf[i]=-grbuf[i];}
static const float g_mdct_window[2][18]={{0.99904822f,0.99144486f,0.97629601f,0.95371695f,0.92387953f,0.88701083f,0.84339145f,0.79335334f,0.73727734f,0.04361938f,0.13052619f,0.21643961f,0.30070580f,0.38268343f,0.46174861f,0.53729961f,0.60876143f,0.67559021f},{1,1,1,1,1,1,0.99144486f,0.92387953f,0.79335334f,0,0,0,0,0,0,0.13052619f,0.38268343f,0.60876143f}};
static void mp3L3_imdct_gr(float *grbuf,float *overlap,unsigned block_type,unsigned n_long_bands){ if (n_long_bands){mp3L3_imdct36(grbuf,overlap,g_mdct_window[0],n_long_bands);grbuf+=18*n_long_bands;overlap+=9*n_long_bands;} if (block_type==2) {mp3L3_imdct_short(grbuf,overlap,32-n_long_bands);} else {mp3L3_imdct36(grbuf,overlap,g_mdct_window[block_type==3],32-n_long_bands);} }
static void mp3L3_save_reservoir(mp3dec *h, mp3dec_scratch *s) { int pos=(s->bs.pos+7)/8u,remains=s->bs.limit/8u-pos; if (remains>511){pos+=remains-511;remains=511;} if (remains>0) {mmov(h->reserv_buf,s->maindata+pos,remains);} h->reserv=remains; }
static int mp3L3_restore_reservoir(mp3dec *h, mp3_bs *bs, mp3dec_scratch *s, int main_data_begin) { int frame_bytes=(bs->limit-bs->pos)/8,bytes_have=vmin(h->reserv,main_data_begin); mcpy(s->maindata,h->reserv_buf+vmax(0,h->reserv-main_data_begin),vmin(h->reserv,main_data_begin)); mcpy(s->maindata+bytes_have,bs->buf+bs->pos/8,frame_bytes); s->bs.buf=s->maindata; s->bs.pos=0; s->bs.limit=(bytes_have+frame_bytes) * 8; return h->reserv>=main_data_begin; }
static void mp3L3_decode(mp3dec *h, mp3dec_scratch *s, mp3L3_gr_info *gr_info, int nch){
    int ch; for(ch=0;ch<nch;ch++) { int limit=s->bs.pos+gr_info[ch].part_23_length; mp3L3_decode_scalefactors(h->header,s->ist_pos[ch],&s->bs,gr_info+ch,s->scf,ch); mp3L3_huffman(s->grbuf[ch],&s->bs,gr_info+ch,s->scf,limit); }
    if (MP3_HDR_TEST_I_STEREO(h->header)) mp3L3_intensity_stereo(s->grbuf[0],s->ist_pos[1],gr_info,h->header);
    else if (MP3_HDR_IS_MS_STEREO(h->header)) mp3L3_midside_stereo(s->grbuf[0],576);
    for(ch=0;ch<nch;ch++,gr_info++){
        int aa_bands=31,n_long_bands=(gr_info->mixed_block_flag?2:0)<<(int)(MP3_HDR_GET_SAMPLE_RATEHDR(h->header)==2);
        if (gr_info->n_short_sfb){aa_bands=n_long_bands-1;mp3L3_reorder(s->grbuf[ch]+n_long_bands*18,s->syn[0],gr_info->sfbtab+gr_info->n_long_sfb);}
        mp3L3_antialias(s->grbuf[ch],aa_bands); mp3L3_imdct_gr(s->grbuf[ch],h->mdct_overlap[ch],gr_info->block_type,n_long_bands); mp3L3_change_sign(s->grbuf[ch]);
    }
}

static void mp3d_DCT_II(float *grbuf, int n){
    int i;
    for(int k=0;k<n;k++){
        float t[4][8],*x,*y=grbuf+k;
        for(x=t[0],i=0;i<8;i++,x++) { float x0=y[i*18], x1=y[(15-i)*18], x2=y[(16+i)*18],x3=y[(31-i)*18], t0=x0+x3, t1=x1+x2, t2=(x1-x2)*g_sec[3*i], t3=(x0-x3)*g_sec[3*i+1]; x[0]=t0+t1; x[8]=(t0-t1)*g_sec[3*i+2]; x[16]=t3+t2; x[24]=(t3-t2)*g_sec[3*i+2]; }
        for(x=t[0],i=0;i<4;i++,x+=8) {
            float x0=x[0], x1=x[1], x2=x[2], x3=x[3], x4=x[4], x5=x[5], x6=x[6], x7=x[7], xt;
            xt=x0-x7; x0+=x7; x7=x1-x6; x1+=x6; x6=x2-x5; x2+=x5; x5=x3-x4; x3+=x4; x4=x0-x3; x0+=x3; x3=x1-x2; x1+=x2; x[0]=x0+x1;
            x[4]=(x0-x1)*0.70710677f; x5+=x6; x6=(x6+x7)*0.70710677f; x7+=xt; x3=(x3+x4)*0.70710677f; x5-=x7*0.198912367f; x7+=x5*0.382683432f; x5-=x7*0.198912367f;
            x0=xt-x6; xt+=x6; x[1]=(xt+x7)*0.50979561f; x[2]=(x4+x3)*0.54119611f; x[3]=(x0-x5)*0.60134488f; x[5]=(x0+x5)*0.89997619f; x[6]=(x4-x3)*1.30656302f; x[7]=(xt-x7)*2.56291556f;
        }
        for(i=0;i<7;i++,y+=4*18){y[0]=t[0][i];y[18]=t[2][i]+t[3][i]+t[3][i+1];y[36]=t[1][i]+t[1][i+1];y[54]=t[2][i+1]+t[3][i]+t[3][i+1];}
        y[0]=t[0][7];y[18]=t[2][7]+t[3][7];y[36]=t[1][7];y[54]=t[3][7];
    }
}

typedef float mp3_sample_t;
static float mp3d_scale_pcm(float sample) { return sample*(1.f/32768.f); }
static void mp3d_synth_pair(mp3_sample_t *pcm, int nch, const float *z){
    float a; a =(z[14*64]-z[0])*29; a+=(z[1*64]+z[13*64])*213; a+=(z[12*64]-z[2*64])*459; a+=(z[ 3*64]+z[11*64])*2037; a+=(z[10*64]-z[4*64])*5153; a+=(z[5*64]+z[9*64])*6574; a+=(z[8*64]-z[6*64])*37489; a+=z[7*64]*75038;
    pcm[0]=mp3d_scale_pcm(a); z+=2; a =z[14*64]*104; a+=z[12*64]*1567; a+=z[10*64]*9727; a+=z[8*64]*64019; a+=z[6*64]*-9975; a+=z[4*64]*-45; a+=z[2*64]*146; a+=z[0*64]*-5; pcm[16*nch]=mp3d_scale_pcm(a);
}

static void mp3d_synth(float *xl, mp3_sample_t *dstl, int nch, float *lins){
    int i; float *xr=xl+576*(nch-1); mp3_sample_t *dstr=dstl+(nch-1); float *zlin=lins+15*64; const float *w=g_win;
    zlin[4*15]=xl[18*16];zlin[4*15+1]=xr[18*16];zlin[4*15+2]=xl[0];zlin[4*15+3]=xr[0]; zlin[4*31]=xl[1+18*16];zlin[4*31+1]=xr[1+18*16];zlin[4*31+2]=xl[1];zlin[4*31+3]=xr[1];
    mp3d_synth_pair(dstr,nch,lins+4*15+1); mp3d_synth_pair(dstr+32*nch,nch,lins+4*15+64+1); mp3d_synth_pair(dstl,nch,lins+4*15); mp3d_synth_pair(dstl+32*nch,nch,lins+4*15+64);
    for(i=14;i>=0;i--){
        #define MP3_LOAD(k) float w0=*w++;float w1=*w++;float *vz=&zlin[4*i-k*64];float *vy=&zlin[4*i-(15-k)*64];
        #define MP3_S0(k) {int j;MP3_LOAD(k) for(j=0;j<4;j++)b[j]=vz[j]*w1+vy[j]*w0,a[j]=vz[j]*w0-vy[j]*w1;}
        #define MP3_S1(k) {int j;MP3_LOAD(k) for(j=0;j<4;j++)b[j]+=vz[j]*w1+vy[j]*w0,a[j]+=vz[j]*w0-vy[j]*w1;}
        #define MP3_S2(k) {int j;MP3_LOAD(k) for(j=0;j<4;j++)b[j]+=vz[j]*w1+vy[j]*w0,a[j]+=vy[j]*w1-vz[j]*w0;}
        float a[4],b[4];
        zlin[4*i]=xl[18*(31-i)]; zlin[4*i+1]=xr[18*(31-i)]; zlin[4*i+2]=xl[1+18*(31-i)]; zlin[4*i+3]=xr[1+18*(31-i)]; zlin[4*(i+16)]=xl[1+18*(1+i)];zlin[4*(i+16)+1]=xr[1+18*(1+i)];zlin[4*(i-16)+2]=xl[18*(1+i)];zlin[4*(i-16)+3]=xr[18*(1+i)];
        MP3_S0(0) MP3_S2(1) MP3_S1(2) MP3_S2(3) MP3_S1(4) MP3_S2(5) MP3_S1(6) MP3_S2(7)
        dstr[(15-i)*nch]=mp3d_scale_pcm(a[1]); dstr[(17+i)*nch]=mp3d_scale_pcm(b[1]); dstl[(15-i)*nch]=mp3d_scale_pcm(a[0]); dstl[(17+i)*nch]=mp3d_scale_pcm(b[0]); dstr[(47-i)*nch]=mp3d_scale_pcm(a[3]); dstr[(49+i)*nch]=mp3d_scale_pcm(b[3]); dstl[(47-i)*nch]=mp3d_scale_pcm(a[2]); dstl[(49+i)*nch]=mp3d_scale_pcm(b[2]);
    }
}

static void mp3d_synth_granule(float *qmf_state, float *grbuf, int nbands, int nch, mp3_sample_t *pcm, float *lins){ for(int i=0;i<nch;i++) {mp3d_DCT_II(grbuf+576*i,nbands);} mcpy(lins,qmf_state,sizeof(float)*15*64); for(int i=0;i<nbands;i+=2) {mp3d_synth(grbuf+i,pcm+32*nch*i,nch,lins+i*64);} mcpy(qmf_state,lins+nbands*64,sizeof(float)*15*64); }
static int mp3d_match_frame(const u8 *hdr, int mp3_bytes){ for(int i=0,nmatch=0;nmatch<10;nmatch++){ i+=mp3_hdr_frame_bytes(hdr+i)+mp3_hdr_padding(hdr+i); if (i + 4 > mp3_bytes) {return nmatch>0;} if (!mp3_hdr_compare(hdr,hdr+i)) {return 0;} } return 1; }
static int mp3d_find_frame(const u8 *mp3, int mp3_bytes, int *ptr_frame_bytes){
    int i;
    for(i=0;i<mp3_bytes-4;i++,mp3++){
        if (mp3_hdr_valid(mp3)){
            int frame_bytes=mp3_hdr_frame_bytes(mp3); int fp=frame_bytes+mp3_hdr_padding(mp3);
            if((frame_bytes&&i+fp<=mp3_bytes&&mp3d_match_frame(mp3,mp3_bytes-i))||(!i&&fp==mp3_bytes)){*ptr_frame_bytes=fp;return i;}
        }
    }
    *ptr_frame_bytes=0; return mp3_bytes;
}

static void mp3dec_init(mp3dec *dec) { dec->header[0]=0; }
static int mp3dec_decode_frame(mp3dec *dec, const u8 *mp3, int mp3_bytes, void *pcm, mp3dec_frame_info *info){
    int i=0,igr,frame_size=0,success=1;
    const u8 *hdr; mp3_bs bs_frame[1];
    if (mp3_bytes>4&&dec->header[0]==0xff&&mp3_hdr_compare(dec->header,mp3)){ frame_size=mp3_hdr_frame_bytes(mp3)+mp3_hdr_padding(mp3); if(frame_size!=mp3_bytes&&(frame_size+4>mp3_bytes||!mp3_hdr_compare(mp3,mp3+frame_size))){frame_size=0;} }
    if (!frame_size){ mset(dec,0,sizeof(mp3dec)); i=mp3d_find_frame(mp3,mp3_bytes,&frame_size); if (!frame_size || i + frame_size > mp3_bytes) {info->frame_bytes=i;return 0;} }
    hdr=mp3+i; mcpy(dec->header,hdr,4); info->frame_bytes=i+frame_size; info->channels=MP3_HDR_IS_MONO(hdr) ? 1 : 2; info->sample_rate=mp3_hdr_sample_rate_hz(hdr); info->layer = 4 - MP3_HDR_GET_LAYER(hdr); info->bitrate_kbps=mp3_hdr_bitrate_kbps(hdr);
    bs_frame[0].buf=hdr + 4; bs_frame[0].pos=0; bs_frame[0].limit=(frame_size - 4) * 8; if(MP3_HDR_IS_CRC(hdr)){mp3_bs_get_bits(bs_frame,16);} if(info->layer!=3){return 0;}  /* Layer 1/2 not supported */
    int main_data_begin=mp3L3_read_side_info(bs_frame,dec->scratch.gr_info,hdr); if(main_data_begin<0||bs_frame->pos>bs_frame->limit){mp3dec_init(dec); return 0;}
    success=mp3L3_restore_reservoir(dec,bs_frame,&dec->scratch,main_data_begin);
    if(success&&pcm!=NULL){ for(igr=0;igr<(MP3_HDR_TEST_MPEG1(hdr)?2:1);igr++,pcm=MP3_OFFSET_PTR(pcm,sizeof(mp3_sample_t)*576*info->channels)){ mset(dec->scratch.grbuf[0],0,576 * 2 * sizeof(float)); mp3L3_decode(dec,&dec->scratch,dec->scratch.gr_info+igr*info->channels,info->channels); mp3d_synth_granule(dec->qmf_state,dec->scratch.grbuf[0],18,info->channels,(mp3_sample_t*)pcm,dec->scratch.syn[0]); } }
    mp3L3_save_reservoir(dec,&dec->scratch); return success*mp3_hdr_frame_samples(dec->header);
}

static size_t mp3_on_read_os(void *ud, void *buf, size_t n) { FHandle f = (FHandle)(uintptr_t)ud; if (f == INVALID_FHANDLE) {return 0;} long result = OS_Read(f,buf,n); return (result > 0) ? (size_t)result : 0; }
static bool mp3_on_seek_os(void *ud, int offset, u8 origin) { FHandle f = (FHandle)(uintptr_t)ud; if (f == INVALID_FHANDLE) {return false;} int whence = origin; return OS_Seek(f,(i64)offset,whence) >= 0; }
static size_t mp3_on_read(mp3 *p, void *buf, size_t n) { size_t r = mp3_on_read_os(p->pUserData,buf,n); p->streamCursor += r; return r; }
static size_t mp3_on_read_clamped(mp3 *p, void *buf, size_t n) { if (p->streamLength == (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF)) {return mp3_on_read(p,buf,n);} u64 rem = p->streamLength - p->streamCursor; if (n > rem) n = (size_t)rem; return mp3_on_read(p,buf,n); }
static bool mp3_on_seek(mp3 *p, int offset, u8 origin) { if (!mp3_on_seek_os(p->pUserData,offset,origin)) {return false;} if (origin == 0) {p->streamCursor = (u64)offset;}else{p->streamCursor += (u64)offset;} return true; }
static u32 mp3_decode_next_frame_ex(mp3 *p, mp3_sample_t *pPCMFrames, mp3dec_frame_info *pInfo) {
    u32 pcmFramesRead = 0; if (p->atEnd) return 0;
    for (;;) {
        mp3dec_frame_info info;
        if (p->dataSize < 16384) {
            if (p->pData) mmov(p->pData, p->pData + p->dataConsumed, p->dataSize);
            p->dataConsumed = 0;
            if (p->dataCapacity < (16384 * 4)) { u8 *nd = (u8*)OS_Realloc(p->pData,p->dataCapacity,16384 * 4); p->pData = nd; p->dataCapacity = 16384 * 4; }
            size_t bytesRead = mp3_on_read_clamped(p, p->pData + p->dataSize, p->dataCapacity - p->dataSize);
            if (!bytesRead && p->dataSize == 0) { p->atEnd = 1; return 0; }
            p->dataSize += bytesRead;
        }
        if (p->dataSize > 2147483647) { p->atEnd = 1; return 0; }
        if (!p->pData) return 0;
        pcmFramesRead = mp3dec_decode_frame(&p->decoder,p->pData + p->dataConsumed,(int)p->dataSize,pPCMFrames,&info);
        p->dataConsumed += (size_t)info.frame_bytes; p->dataSize -= (size_t)info.frame_bytes;
        if (pcmFramesRead > 0) {
            pcmFramesRead = mp3_hdr_frame_samples(p->decoder.header);
            p->pcmFConsInMP3F = 0; p->pcmFRemInMP3F = pcmFramesRead; p->mp3FChan = info.channels; p->mp3FrameSampleRate = info.sample_rate;
            if (pInfo) *pInfo = info;
            break;
        } else if (info.frame_bytes == 0) {
            mmov(p->pData, p->pData + p->dataConsumed, p->dataSize);
            p->dataConsumed = 0;
            if (p->dataCapacity == p->dataSize) { size_t needed=p->dataCapacity + 16384*4; u8 *nd=(u8*)OS_Realloc(p->pData,p->dataCapacity,needed); p->pData=nd; p->dataCapacity=needed; }
            size_t bytesRead = mp3_on_read_clamped(p,p->pData + p->dataSize,p->dataCapacity - p->dataSize);
            if (!bytesRead) { p->atEnd = 1; return 0; }
            p->dataSize += bytesRead;
        }
    }
    return pcmFramesRead;
}

static u32 mp3_decode_next_frame(mp3 *p) { return mp3_decode_next_frame_ex(p,(mp3_sample_t*)p->pcmFrames,NULL); }
static void mp3_skip_id3v2(mp3 *p) {
    char h[10]; if (mp3_on_read_os(p->pUserData, h, 10) != 10) return;
    if (h[0] == 'I' && h[1] == 'D' && h[2] == '3') {
        u32 sz = (((u32)h[6] & 0x7F) << 21) | (((u32)h[7] & 0x7F) << 14) | (((u32)h[8] & 0x7F) << 7) | ((u32)h[9] & 0x7F);
        if (h[5] & 0x10) sz += 10;
        mp3_on_seek_os(p->pUserData,(int)sz,1); // SEEK_CUR
        p->streamStartOffset += 10 + sz; p->streamCursor=p->streamStartOffset;
    } else mp3_on_seek_os(p->pUserData,0,0); // SEEK_SET
}

static bool mp3_init_internal(mp3 *p) {
    mp3dec_init(&p->decoder);
    p->streamCursor = p->streamStartOffset=0; p->streamLength = (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF); p->delayInPCMFrames = p->paddingInPCMFrames=0; p->totalPCMFrameCount = (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF);
    if (mp3_on_seek_os(p->pUserData, 0, 2)) { // SEEK_END
        i64 slen = OS_Tell((FHandle)(uintptr_t)p->pUserData);
        if (slen > 0) { if (slen > 128) { char tag[3]; mp3_on_seek_os(p->pUserData,-128,2); if (mp3_on_read(p, tag, 3) == 3 && tag[0]=='T' && tag[1]=='A' && tag[2]=='G') slen -= 128; } p->streamLength = (u64)slen; }
        mp3_on_seek_os(p->pUserData,0,0); p->streamCursor = 0;
    }
    mp3_skip_id3v2(p); mp3dec_frame_info firstFrameInfo; u32 firstFramePCMFrameCount = mp3_decode_next_frame_ex(p,(mp3_sample_t*)p->pcmFrames,&firstFrameInfo);
    if (firstFramePCMFrameCount == 0) { OS_Free(p->pData,p->dataCapacity); p->pData = NULL; p->dataCapacity = 0; return false; }
    p->channels=p->mp3FChan; p->sampleRate=p->mp3FrameSampleRate; return true;
}

static bool mp3_init_file(mp3 *pMP3, const char *path) { if(!pMP3 || !path){return false;} mset(pMP3,0,sizeof(mp3)); FHandle f=OS_OpenReadonly(path); if(f == INVALID_FHANDLE){return false;} pMP3->pUserData=(void*)(uintptr_t)f; bool r=mp3_init_internal(pMP3); if(!r){OS_Close(f); return false;} return true; }
static void mp3_uninit(mp3 *pMP3) { if (!pMP3) {return;} if (pMP3->pUserData) {OS_Close((FHandle)(uintptr_t)pMP3->pUserData); pMP3->pUserData=NULL;} OS_Free(pMP3->pData, pMP3->dataCapacity); pMP3->pData = NULL; pMP3->dataCapacity = 0; }
static void mp3_reset(mp3 *p) { p->pcmFConsInMP3F=0; p->pcmFRemInMP3F=0; p->currentPCMFrame=0; p->dataSize=0; p->atEnd=0; mp3dec_init(&p->decoder); }
static bool mp3_seek_to_start_of_stream(mp3 *p){u64 o=p->streamStartOffset;if(!mp3_on_seek(p,o<=0x7FFFFFFF?(int)o:0x7FFFFFFF,0))return 0;if(o>0x7FFFFFFF){o-=0x7FFFFFFF;while(o>0){int c=(o<=0x7FFFFFFF)?(int)o:0x7FFFFFFF;if(!mp3_on_seek(p,c,1))return 0;o-=c;}}mp3_reset(p);return 1;}
static u64 mp3_read_pcm_frames_raw(mp3 *p, u64 framesToRead, void *pBufferOut){
    u64 totalFramesRead=0;
    while(framesToRead>0){
        u32 framesToConsume;
        if(p->currentPCMFrame<p->delayInPCMFrames){ u32 skip=(u32)vmin(p->pcmFRemInMP3F,p->delayInPCMFrames-p->currentPCMFrame); p->currentPCMFrame+=skip; p->pcmFConsInMP3F+=skip; p->pcmFRemInMP3F-=skip; }
        framesToConsume=(u32)vmin(p->pcmFRemInMP3F,framesToRead);
        if(p->totalPCMFrameCount != (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF) && p->totalPCMFrameCount > p->paddingInPCMFrames){
            if(p->currentPCMFrame<(p->totalPCMFrameCount-p->paddingInPCMFrames)){ u64 rem=(p->totalPCMFrameCount-p->paddingInPCMFrames)-p->currentPCMFrame; if(framesToConsume>rem) framesToConsume=(u32)rem; } else break;
        }
        if(pBufferOut){ float *out=(float*)MP3_OFFSET_PTR(pBufferOut,sizeof(float)*totalFramesRead*p->channels); float *in =(float*)MP3_OFFSET_PTR(&p->pcmFrames[0],sizeof(float)*p->pcmFConsInMP3F*p->mp3FChan); mcpy(out,in,sizeof(float)*framesToConsume*p->channels); }
        p->currentPCMFrame+=framesToConsume; p->pcmFConsInMP3F+=framesToConsume; p->pcmFRemInMP3F-=framesToConsume; totalFramesRead+=framesToConsume; framesToRead-=framesToConsume;
        if(framesToRead==0) break;
        if(p->totalPCMFrameCount != (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF) && p->totalPCMFrameCount > p->paddingInPCMFrames && p->currentPCMFrame >= (p->totalPCMFrameCount - p->paddingInPCMFrames)) break;
        if(mp3_decode_next_frame(p)==0) break;
    }
    return totalFramesRead;
}

static u64 mp3_read_pcm_frames_f32(mp3* m, u64 framesToRead, float *pBufferOut){ if(!m) {return 0;} return mp3_read_pcm_frames_raw(m,framesToRead,pBufferOut); }
static bool mp3_seek_to_pcm_frame(mp3* m, u64 fidx){ if(!m) {return 0;} if(fidx==0){return mp3_seek_to_start_of_stream(m);} if(fidx<m->currentPCMFrame){ if(!mp3_seek_to_start_of_stream(m)) {return 0;} } u64 toSkip=fidx-m->currentPCMFrame; u64 skipped=mp3_read_pcm_frames_f32(m,toSkip,NULL); return skipped==toSkip; }
static u64 mp3_get_pcm_frame_count(mp3* pMP3){
    u64 total; if(pMP3->totalPCMFrameCount != (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF)){ total=pMP3->totalPCMFrameCount; if(total>=pMP3->delayInPCMFrames){total-=pMP3->delayInPCMFrames;} if(total>=pMP3->paddingInPCMFrames){total-=pMP3->paddingInPCMFrames;} return total; }
    u64 savedFrame=pMP3->currentPCMFrame; if(!mp3_seek_to_start_of_stream(pMP3)) return 0;
    total=0; for(;;){ u32 n=mp3_decode_next_frame_ex(pMP3,NULL,NULL); if(!n){break;}total+=n; } mp3_seek_to_start_of_stream(pMP3); mp3_seek_to_pcm_frame(pMP3,savedFrame); return total;
}

typedef struct { FHandle fp; u16 channels,bitsPerSample,fmtTag; u32 sampleRate; u64 totalPCMFrameCount,dataChunkDataPos,bytesRemaining; } WaveFile;
static u16 WavU16LE(const u8 *d) { return (u16)(d[0]|(d[1]<<8)); }
static u32 WavU32LE(const u8 *d) { return (u32)(d[0]|(d[1]<<8)|(d[2]<<16)|(d[3]<<24)); }
static bool WavInit(WaveFile *w, const char *path) {
    u8 buf[36]; mset(w,0,sizeof(*w)); w->fp = OS_OpenReadonly(path); if (w->fp == INVALID_FHANDLE) return false;
    if (OS_Read(w->fp,buf,12) != 12) goto fail;
    if (mcmp(buf,"RIFF",4) != 0) goto fail;
    if (mcmp(buf+8,"WAVE",4) != 0) goto fail;
    bool got_fmt=false,got_data=false;
    for (;;) {
        u8 chunkId[4],szBuf[4]; if ((OS_Read(w->fp,chunkId,4) != 4) || (OS_Read(w->fp,szBuf,4) != 4)) break;
        u32 chunkSize = WavU32LE(szBuf);
        if (mcmp(chunkId, "fmt ", 4) == 0) {
            if (chunkSize < 16) goto fail;
            u8 fmt[18]; u32 toRead = chunkSize < 18 ? chunkSize : 18; if (OS_Read(w->fp,fmt,toRead) != (long)toRead) goto fail;
            if (chunkSize > toRead) OS_Seek(w->fp, (i64)(chunkSize - toRead),1);
            w->fmtTag = WavU16LE(fmt+0); w->channels = WavU16LE(fmt+2); w->sampleRate = WavU32LE(fmt+4); w->bitsPerSample = WavU16LE(fmt+14);
            if (w->fmtTag == 0xFFFE && toRead >= 18) { u16 cbSize = WavU16LE(fmt + 16); if (cbSize >= 22) { u8 ext[22]; if(OS_Read(w->fp,ext,22) == 22){w->fmtTag=WavU16LE(ext + 6);} } }
            if (w->fmtTag != 0x1) goto fail; // PCM format
            if (w->bitsPerSample != 8 && w->bitsPerSample != 16) goto fail;
            got_fmt = true;
        } else if (mcmp(chunkId,"data",4) == 0) {
            w->dataChunkDataPos = (u64)OS_Tell(w->fp);
            u32 bpf = (u32)w->channels * (w->bitsPerSample / 8);
            if (bpf == 0) goto fail;
            w->bytesRemaining = chunkSize - (chunkSize % bpf);
            w->totalPCMFrameCount = w->bytesRemaining / bpf;
            got_data = true;
            break; /* data chunk is last thing we need */
        } else OS_Seek(w->fp,(i64)(chunkSize + (chunkSize & 1)),1);
    }

    if (got_fmt && got_data) return true;
    fail:
    if (w->fp != INVALID_FHANDLE) { OS_Close(w->fp); w->fp = INVALID_FHANDLE; }
    return false;
}

static u64 WavReadPCMFrames(WaveFile *w, u64 framesToRead, float *out) {
    if (!w || !out || framesToRead == 0) return 0;
    u32 bps = w->bitsPerSample; u32 bpf = (u32)w->channels * (bps / 8); if (bpf == 0) return 0;
    u64 framesLeft=w->bytesRemaining / bpf; if(framesToRead > framesLeft){framesToRead=framesLeft;}
    u64 totalRead=0; u8  tmp[4096];
    while (framesToRead > 0) {
        u64 batchFrames=framesToRead; u64 batchBytes=batchFrames * bpf;
        if (batchBytes > sizeof(tmp)) { batchFrames = sizeof(tmp) / bpf; batchBytes  = batchFrames * bpf; }
        long got=OS_Read(w->fp,tmp,(size_t)batchBytes);
        u64 gotFrames=got / bpf; u64 samples=gotFrames * w->channels;
        if (bps == 8) { for (u64 i = 0; i < samples; i++) {*out++ = (tmp[i] / 255.0f) * 2.0f - 1.0f;} }
        else { for (u64 i = 0; i < samples; i++) {i16 s; mcpy(&s,tmp + i*2,2); *out++ = s * (1.0f / 32768.0f);} } // 16bit LE

        w->bytesRemaining -= gotFrames * bpf; framesToRead -= gotFrames; totalRead += gotFrames; if (gotFrames < batchFrames) break;
    }
    return totalRead;
}

typedef struct { mp3 dec; bool open; float fade_vol,fade_target,fade_step; u32 src_rate; u64 frames_decoded,total_frames; } mp3_channel_t;
static wav_channel_t wav_ch[MAX_CHANNELS],*ext_ch[MAX_CHANNELS]; static u32 wav_count,ext_count,mp3_slot,log_frame_count,log_frame_pos; 
static mp3_channel_t mp3_ch[2]; static float *log_samples; static size_t log_allocSize=0; static bool log_playing,mp3_paused = false;
static float *resample_stereo(float *src, size_t srcSize, u32 *frames, u32 src_rate, size_t* sz) {
    if (src_rate == AUDIO_RATE) return src;
    u32 sf = *frames, df = (u32)((u64)sf*AUDIO_RATE/src_rate); float *dst = (float*)OS_Alloc(df*2*sizeof(float)); *sz = df*2*sizeof(float); float ratio = (float)sf/(float)df;
    for (u32 i = 0; i < df; i++) { float pos = i*ratio; u32 a = (u32)pos, b = a+1<sf?a+1:a; float t = pos-(float)a; dst[i*2+0] = src[a*2+0]+t*(src[b*2+0]-src[a*2+0]); dst[i*2+1] = src[a*2+1]+t*(src[b*2+1]-src[a*2+1]); }
    OS_Free(src,srcSize); *frames = df; return dst;
}

static void WavUnInit(WaveFile *w) { if (w->fp != INVALID_FHANDLE) { OS_Close(w->fp); w->fp = INVALID_FHANDLE; } }
static float *load_wav(const char *path,u32 *out_frames, size_t* sz) {
    WaveFile wav; if (!WavInit(&wav,path)) {return NULL;} if (wav.channels > 2) { WavUnInit(&wav); return NULL; }
    u64 frames = wav.totalPCMFrameCount; float *buf = (float*)OS_Alloc(frames*AUDIO_CHANNELS*sizeof(float)); size_t bufSize = frames*AUDIO_CHANNELS*sizeof(float); u64 got = WavReadPCMFrames(&wav,frames,buf);
    if (wav.channels == 1) for (i64 i=(i64)got-1;i>=0;i--) { buf[i*2+1]=buf[i]; buf[i*2]=buf[i]; }
    u32 src_rate = wav.sampleRate; WavUnInit(&wav); *out_frames = (u32)got; return resample_stereo(buf,bufSize,out_frames,src_rate,sz); // Reallocates and returns new buffer, freeing the buf alloc'ed here
}

i32 GetFreeWavSlot() { i32 retval = -1; for (u32 i = 0; i < wav_count; i++) { if (!wav_ch[i].playing && wav_ch[i].samples) {OS_Free(wav_ch[i].samples,wav_ch[i].allocSize); wav_ch[i].samples = NULL; wav_ch[i].allocSize = 0; retval=i; } } return retval; }
#include "synth.c" // Audio Synthesis Engine
static void wave_mix(wav_channel_t* w, float* mix) {
    float vol = w->volume * (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeEffects/100.0f); V3 pos = w->pos; float dist = V3_Dist(pos,World.position[PLAYER1]); float spatial_atten = (dist >= 64.0f) ? 0.0f : ((dist <= 1.0f) ? 1.0f : 1.0f-(dist-1.0f)/63.0f);
    if (w->positional) vol *= spatial_atten;
    for (i32 f = 0; f < AUDIO_FRAMES; f++) { if (w->frame_pos >= w->frame_count){ if (w->looping){w->frame_pos=0;}else{w->playing=false; break;} } mix[f*2+0] += w->samples[w->frame_pos*2+0]*vol; mix[f*2+1] += w->samples[w->frame_pos*2+1]*vol; w->frame_pos++; }
}

static void audio_mix_period(i16 *out) {
    float mix[AUDIO_FRAMES*AUDIO_CHANNELS]; mset(mix,0,sizeof(mix));
    for (u32 c=0;c<wav_count;c++) { if ( wav_ch[c].playing &&  wav_ch[c].samples) {wave_mix(&wav_ch[c],mix);} } // General wav file playback
    for (u32 c=0;c<ext_count;c++) { if (ext_ch[c]->playing && ext_ch[c]->samples) {wave_mix(ext_ch[c], mix);} } // Looped Ambients
    for (u32 c=0;c<MAX_SYNTH_VOICES;c++) if (syn_ch[c].active) synth_mix(&syn_ch[c],mix); // Synthesized audio (oh yes!)
    if (log_playing && log_samples) {
        float vol = (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeMessage/100.0f);
        for (i32 f = 0; f < AUDIO_FRAMES; f++) {
            if (log_frame_pos >= log_frame_count) { log_playing=false; break; }
            mix[f*2+0] += log_samples[log_frame_pos*2+0] * vol; mix[f*2+1] += log_samples[log_frame_pos*2+1] * vol; log_frame_pos++;
        }
    }
    if (!mp3_paused) {
        for (u32 s = 0; s < 2; s++) {
            mp3_channel_t *m = &mp3_ch[s];
            if (!m->open) continue;
            u32 src_rate = m->src_rate ? m->src_rate : AUDIO_RATE;
            u64 frames_to_read = (src_rate == AUDIO_RATE) ? AUDIO_FRAMES : (u64)((u64)AUDIO_FRAMES*src_rate/AUDIO_RATE)+2;
            float raw[AUDIO_FRAMES*4];
            u64 got = mp3_read_pcm_frames_f32(&m->dec,frames_to_read,raw);
            if (got == 0) { mp3_uninit(&m->dec); m->open=false; continue; }
            float vol = m->fade_vol * (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeMusic/100.0f);
            m->frames_decoded += got; float ratio = (float)got/(float)AUDIO_FRAMES;
            for (i32 f = 0; f < AUDIO_FRAMES; f++) {
                float pos = f*ratio; u32 a=(u32)pos, b=(a+1<(u32)got)?a+1:a; float t=pos-(float)a;
                float l = raw[a*2+0]+t*(raw[b*2+0]-raw[a*2+0]), r = raw[a*2+1]+t*(raw[b*2+1]-raw[a*2+1]);
                mix[f*2+0] += l*vol; mix[f*2+1] += r*vol;
                if (m->fade_step != 0.0f) {
                    m->fade_vol += m->fade_step;
                    if (m->fade_step>0.0f && m->fade_vol>=m->fade_target) { m->fade_vol=m->fade_target; m->fade_step=0.0f; }
                    else if (m->fade_step<0.0f && m->fade_vol<=m->fade_target) { m->fade_vol=m->fade_target; m->fade_step=0.0f; if (m->fade_target==0.0f) { mp3_uninit(&m->dec); m->open=false; } }
                }
            }
        }
    }
    for (u32 i = 0; i < AUDIO_FRAMES * AUDIO_CHANNELS; i++) { float s = mix[i]; s = s > 1.0f ? 1.0f : (s < -1.0f ? -1.0f : s); out[i] = (i16)(s * 32767.0f); }
}

void play_wav(const char *path,float volume,V3 pos,bool positional) {
    if (sEqual(path,"null")) return;
    char p[128]; sFormat(p,sizeof(p),"./Audio/%s.wav",path);
    i32 slot = GetFreeWavSlot();
    if (slot==-1 && wav_count<MAX_CHANNELS) slot=wav_count++;
    if (slot==-1) { DualLog("WARNING: Max WAV channels (%d) reached\n",MAX_CHANNELS); return; }
    u32 frames; size_t sz=0; float *buf = load_wav(p,&frames,&sz);
    if (!buf) { DualLog("ERROR: Failed to load WAV %s\n",p); return; }
    wav_ch[slot] = (wav_channel_t){ .samples = buf, .allocSize = sz, .frame_count = frames, .frame_pos = 0, .volume = volume, .looping = false, .positional = positional, .pos=pos, .playing = true };
}

void play_message(const char *path) {
    if (log_playing && log_samples && log_allocSize > 0) { log_playing=false; OS_Free(log_samples,log_allocSize); log_samples=NULL; log_allocSize=0; }
    u32 frames; float *buf = load_wav(path,&frames,&log_allocSize); if (!buf) { DualLogError("Failed to load %s\n",path); return; }
    log_samples=buf; log_frame_count=frames; log_frame_pos=0; log_playing=true;
}

i32 SndInit(const char *path, wav_channel_t *w) { u32 frames; size_t sz=0; float *buf=load_wav(path,&frames,&sz); if(!buf){return -1;} w->samples=buf; w->allocSize=sz; w->frame_count=frames; w->frame_pos=0; w->volume=1.0f; w->looping=w->positional=w->playing=false; return 0; }
i32 SndStart(wav_channel_t* w) { w->frame_pos = 0; w->playing = true; for (u32 i=0;i<ext_count;++i) if (ext_ch[i] == w) return 0; if (ext_count < MAX_CHANNELS) ext_ch[ext_count++] = w; return 0; }
void SndStop(wav_channel_t* w) { w->playing=false; }
void SndUninit(wav_channel_t* w) { if (w->samples) { OS_Free(w->samples,w->allocSize); w->samples = NULL; w->allocSize = 0; } w->playing = false; for (u32 i=0;i<ext_count;++i) if (ext_ch[i] == w) { ext_ch[i] = ext_ch[--ext_count]; break; } }
void SndSetVolume(wav_channel_t* w, float volume) { w->volume=volume; }
void SoundSetLooping(wav_channel_t* w, bool loop) { w->looping=loop; }
bool SndPlaying(wav_channel_t* w)  { return w->playing; }
i32 SndFrmCurpos(wav_channel_t* w,u64 *pCursor) { *pCursor=w->frame_pos; return 0; }
float SndLen(wav_channel_t* w) {  return (float)w->frame_count / (float)AUDIO_RATE; }
static void mp3_open_slot(i32 s, const char *path, float fade_from, float fade_to, i32 fade_ms) {
    mp3_channel_t *m = &mp3_ch[s]; if (m->open) { mp3_uninit(&m->dec); m->open=false; } if (!mp3_init_file(&m->dec,path)) { DualLog("ERROR: Failed to load MP3 %s\n",path); return; }
    m->src_rate = m->dec.sampleRate; m->total_frames = mp3_get_pcm_frame_count(&m->dec); mp3_seek_to_pcm_frame(&m->dec,0); m->frames_decoded = 0; m->open = true; m->fade_target = fade_to;
    m->fade_vol = (m->fade_step = (fade_ms > 0) ? (fade_to - fade_from) / (AUDIO_RATE * fade_ms / 1000.0f) : 0.0f) == 0.0f ? fade_to : fade_from;
}

void play_mp3(const char *path, i32 fade_ms) { i32 old = mp3_slot, next = mp3_slot ? 0 : 1; if (mp3_ch[old].open) { mp3_ch[old].fade_target = 0.0f; mp3_ch[old].fade_step = (fade_ms > 0) ? -mp3_ch[old].fade_vol / (AUDIO_RATE * fade_ms / 1000.0f) : -1.0f; } mp3_open_slot(mp3_slot = next,path,0.0f,1.0f,fade_ms); }
void mp3_clear() { for (i32 i=0;i<2;i++) if (mp3_ch[i].open) { mp3_uninit(&mp3_ch[i].dec); mp3_ch[i].open=false; } mp3_slot=0; }
void MP3Pause() { mp3_paused = true; }
void MP3Resume() { mp3_paused = false; }
float GetMP3RemainingTime() { mp3_channel_t *m = &mp3_ch[mp3_slot]; return (!m->open || m->frames_decoded >= m->total_frames) ? 0.0f : (!m->total_frames ? 1.0f : (float)(m->total_frames - m->frames_decoded) / (m->src_rate ? m->src_rate : AUDIO_RATE)); }
static FHandle pcm_fds[8]; static i32 pcm_fd_count = 0;
pthread_t audThreadID; void* AudThread(void* arg); 
#if defined(_WIN32)
    void AudioUpdate() {
        if (pcm_fd_count==0) {return;} 
        i16 buf[AUDIO_FRAMES*AUDIO_CHANNELS]; pcm_sync_t sync; if (pcm_sync(pcm_fds[0],&sync) < 0) {return;}
        u32 avail = AUDBUF_SIZE - ((sync.control.appl_ptr - sync.status.hw_ptr > AUDBUF_SIZE) ? 0 : sync.control.appl_ptr - sync.status.hw_ptr);
        while (avail>=(u32)AUDIO_FRAMES) { audio_mix_period(buf); for (i32 i=0;i<pcm_fd_count;i++) { if (pcm_write(buf,AUDIO_FRAMES)<0) {pcm_prepare(pcm_fds[i]);} } avail-=AUDIO_FRAMES; }
    }
    
    void InitAudio() { InitSCFTables(); FHandle first = pcm_open_all(AUDIO_RATE,AUDIO_CHANNELS,AUDIO_FRAMES,AUDIO_PERIODS); if (first == INVALID_FHANDLE) { DualLog("ERROR: No WASAPI audio device found\n"); return; } pcm_fds[0] = first; pcm_fd_count = 1; pthread_create(&audThreadID,NULL,AudThread,NULL); }
#else // Linux
    typedef void snd_pcm_t;
    typedef int (*pfnspo)(snd_pcm_t**,const char*,int,int); typedef int (*pfn_snd_pcm_close)(snd_pcm_t*);    typedef int (*pfnspw)(snd_pcm_t*,const void*,u32);
    typedef int (*pfnspr)(snd_pcm_t*,int,int);              typedef int (*pfnspp)(snd_pcm_t*);               typedef int (*pfnsphps)();
    typedef int (*pfnsphpa)(snd_pcm_t*,void*);              typedef int (*pfnsphpsa)(snd_pcm_t*,void*,u32);  typedef int (*pfnsphpsf)(snd_pcm_t*,void*,int);
    typedef int (*pfnsphp)(snd_pcm_t*,void*);               typedef int (*pfnsphpsc)(snd_pcm_t*, void*,u32); typedef int (*pfnsphpsrn)(snd_pcm_t*,void*,u32*,int*);
    typedef int (*pfnsphpspsn)(snd_pcm_t*,void*,u64*,int*); typedef int (*pfnsphpspn)(snd_pcm_t*,void*,u32*,int*); static snd_pcm_t *apcm; static pfnspw snd_pcm_writei; static pfnspr snd_pcm_recover;
    static bool alsa_try_open_default() {
        void *so = dlopen("libasound.so.2",2); if (!so) {so = dlopen("libasound.so",2);} if (!so) { DualLog("Audio: libasound not found\n"); return false; }
        pfnspo spo = dlsym(so,"snd_pcm_open");                              pfnsphpa sphpa = dlsym(so,"snd_pcm_hw_params_any");            pfnsphps sphps = dlsym(so,"snd_pcm_hw_params_sizeof");            pfnsphpsa sphpsa = dlsym(so,"snd_pcm_hw_params_set_access");
        pfnsphpsf sphpsf = dlsym(so,"snd_pcm_hw_params_set_format");        pfnsphpsc sphpsc = dlsym(so,"snd_pcm_hw_params_set_channels"); pfnsphpsrn sphpsrn = dlsym(so,"snd_pcm_hw_params_set_rate_near"); pfnsphpspsn sphpspsn = dlsym(so,"snd_pcm_hw_params_set_period_size_near");
        pfnsphpspn sphpspn= dlsym(so,"snd_pcm_hw_params_set_periods_near"); pfnsphp snd_pcm_hw_params = dlsym(so,"snd_pcm_hw_params");     snd_pcm_writei  = dlsym(so,"snd_pcm_writei");                     snd_pcm_recover = dlsym(so,"snd_pcm_recover"); 
        pfnspp spp = dlsym(so,"snd_pcm_prepare");
        if (!spo || !sphps || !sphpa || !sphpsa || !sphpsf || !sphpsc || !sphpsrn || !sphpspsn || !sphpspn || !snd_pcm_hw_params || !snd_pcm_writei || !snd_pcm_recover || !spp) { DualLogError("Audio: libasound missing required symbols\n"); return false; }
        int r = spo(&apcm,"default",0,0);  if (r < 0 ||                            !apcm) { DualLogError("snd_pcm_open('default') failed: %d\n",r); return false; }
        int sz = sphps(); u8 hwp_buf[640]; if (sz > (int)sizeof(hwp_buf)                ) { DualLogError("hw_params_t too large (%d)\n",sz); return false; }
        void *hwp = hwp_buf;               if ((r = sphpa(apcm,hwp))                 < 0) { DualLogError("hw_params_any failed\n"); return false; }
                                           if ((r = sphpsa(apcm,hwp,3))              < 0) { DualLogError("set_access failed\n"); return false; }
                                           if ((r = sphpsf(apcm,hwp,2)   )           < 0) { DualLogError("set_format S16_LE failed\n"); return false; }
                                           if ((r = sphpsc(apcm,hwp,AUDIO_CHANNELS)) < 0) { DualLogError("set_channels(%d) failed\n",AUDIO_CHANNELS); return false; }
        u32 rate   =AUDIO_RATE; int dir=0; if ((r = sphpsrn(apcm,hwp,&rate,&dir))    < 0) { DualLogError("set_rate(%u) failed\n", AUDIO_RATE); return false; }
        u64 period =AUDIO_FRAMES;   dir=0; if ((r = sphpspsn(apcm,hwp,&period,&dir)) < 0) { DualLogError("set_period_size(%d) failed\n", AUDIO_FRAMES); return false; }
        u32 periods=AUDIO_PERIODS;  dir=0; if ((r = sphpspn(apcm,hwp,&periods,&dir)) < 0) { DualLogError("set_periods(%d) failed\n", AUDIO_PERIODS); return false; }
                                           if ((r = snd_pcm_hw_params(apcm,hwp))     < 0) { DualLogError("hw_params apply failed\n"); return false; }
                                           if ((r = spp(apcm))                       < 0) { DualLogError("snd_pcm_prepare failed\n"); return false; }
        return true;
    }

    static void init_pcm_device(i32 card, i32 dev) {
        FHandle r = pcm_open(card,dev,1|(1<<1)); if (r == INVALID_FHANDLE) return;
        pcm_params_t p; hw_params_fill(&p.hw_params); pcm_sw_params_t *sw = &p.sw_params; mset(sw,0,sizeof(*sw)); sw->start_threshold = 1; sw->period_step = 1;
        hw_params_set(&p.hw_params,0/*PCM_FORMAT*/,3);                  hw_params_set(&p.hw_params,11/*PCM_RATE*/,AUDIO_RATE); hw_params_set(&p.hw_params,10/*PCM_CHANNELS*/,AUDIO_CHANNELS);
        hw_params_set(&p.hw_params,13/*PCM_PERIOD_SIZE*/,AUDIO_FRAMES); hw_params_set(&p.hw_params,15,AUDIO_PERIODS);
        if (pcm_params_setup(r,&p) >= 0 && pcm_fd_count < 8) pcm_fds[pcm_fd_count++] = r;
        else { DualLogError("Audio: raw device card=%d dev=%d setup failed, closing\n",card,dev); OS_Close(r); }
    }

    void AudioUpdate() { if (!apcm) {return;} i16 buf[AUDIO_FRAMES*AUDIO_CHANNELS]; audio_mix_period(buf); int r = snd_pcm_writei(apcm,buf,(u32)AUDIO_FRAMES); if (r < 0 && snd_pcm_recover(apcm,r,0) >= 0) { snd_pcm_writei(apcm,buf,(u32)AUDIO_FRAMES); } }
    void InitAudio() { InitSCFTables(); if (!alsa_try_open_default()) { for (i32 card = 0; card < 8; card++) { for (i32 dev = 0; dev < 8; dev++) init_pcm_device(card,dev); } if (pcm_fd_count == 0) {DualLogError("Audio: no output device found\n"); return; } } pthread_create(&audThreadID,NULL,AudThread,NULL); }
#endif

void* AudThread(void* arg) { (void)arg; while (1) { AudioUpdate(); OS_USleep(1000); } return NULL; }
// Looping Ambients SFX System
#define MAXAMB 256
typedef struct { wav_channel_t sound; u32 loaded; float length_sec; } AmbientSlot; typedef struct { u16 index; const char* filename; } AmbientDef;
u16 ambReg[MAXAMB]; static AmbientSlot ambientSlots[MAXAMB] = {0}; static u16 ambs=0;
static const AmbientDef ambientSounds[MAXAMB] = {
    {621,"airhiss"},        {622,"clicker"},  {623,"compressor"},    {624,"dishwasher"},{625,"drip_amb"},{626,"fan1"},         {627,"generator_gas"},   {628,"gurgle"},    {629,"icemaker"},       {630,"intake"},            {631,"lathe"},        {632,"lev3loop1"},    {633,"lev3loop2"},
    {634,"lev3loop3"},      {635,"lev3loop4"},{636,"liquid_bubble"}, {637,"lava2"},     {638,"rain"},    {639,"machgear_loop"},{640,"machine_ambience"},{641,"machine_go"},{642,"machine_humamb7"},{643,"machine_humlonoise"},{644,"machine_loop1"},{645,"machine_loop2"},{646,"machinea1"},
    {647,"machinevat_loop"},{648,"mist"},     {649,"pipewater_loop"},{650,"powerloom"}, {651,"pump"},    {652,"pump2"},        {653,"rain"},            {654,"steam_loop"},{655,"washing_machine"}};
void MixAmbs() {    
    for (u16 i=0;i<ambs;++i) {
        u16 a = ambReg[i];
        const AmbientDef* def = NULL; for (size_t j=0;j<MAXAMB;++j) { if (ambientSounds[j].index==World.instances[a].index) {def = &ambientSounds[j]; break; } }
        float d = V3_Dist(World.position[PLAYER1],World.position[ambReg[i]]);
        AmbientSlot* slot = &ambientSlots[(size_t)(def - ambientSounds)];
        if (d < 7.68f && PositionVisibleFromPlayerCell(World.position[ambReg[i]].x,World.position[ambReg[i]].z)) {
            if (!slot->loaded) {
                SndUninit(&slot->sound);
                char path[512]; sFormat(path,sizeof(path),"./Audio/ambient/%s.wav",def->filename);
                if (SndInit(path,&slot->sound) != 0) continue;
                slot->length_sec = SndLen(&slot->sound); if(slot->length_sec <= 0.0f) {SndUninit(&slot->sound); continue;}
                SoundSetLooping(&slot->sound,true);
                slot->loaded = 1;
            }
            if (!SndPlaying(&slot->sound)) SndStart(&slot->sound);
            if (slot->length_sec > 0.0f) { u64 cur; SndFrmCurpos(&slot->sound,&cur); } // Time sync
            float final_vol = World.instances[a].volume * ((d <= 1.0f) ? 1.0f : (d >= 7.68f) ? 0.0f : (7.68f - d) / (7.68f - 1.0f));
            SndSetVolume(&slot->sound,final_vol);
        } else if (SndPlaying(&slot->sound)) SndStop(&slot->sound);
    }
}

void ResetLevelAudio(void) { ambs=0; mset(ambReg,0,ambs * sizeof(u16)); for (u16 i = INSTS_1ST_IDX; i<World.instCount;++i) { if(IdxIsAmbient(World.instances[i].index)){ambReg[ambs]=i; ambs++; if(ambs >= MAXAMB){DualLogError("Ambient noises %u > %u!\n",ambs,MAXAMB); break;} World.instances[i].volume=EDefs[World.instances[i].index].volume * 0.5f;} } }
// Music System
#define BUFFER_MS 50
#define AUD_BUFFER_T 0.05f
const char* levelMusicLooped[MAX_LEVELS] = {"looped/track0","looped/track1","looped/track2","looped/track3","looped/track4","looped/track5","looped/track6","looped/track7","looped/track8","looped/track9","looped/track10","looped/track11","looped/track12","looped/track13"};
const char* reactorMusic[13] = {"THM4-01_reactorcombat1","THM4-02_reactorcombat2","THM4-03_reactorcombat3","THM4-04_reactorcombat4","THM4-05_reactorwalkingatocombat","THM4-06_reactorwalkingbtocombat","THM4-09_reactorwalkinga1","THM4-10_reactorwalkinga2","THM4-11_reactorwalkingb1","THM4-12_reactorwalkingb2","THM4-13_reactorwalkingb3","THM4-14_reactorwalkingc1","THM4-15_reactorwalkingc2"};
const char* medicalMusic[11] = {"THM1-19_medicalstart","THM1-01_medicalwalking1","THM1-02_medicalwalking2","THM1-03_medicalwalking3","THM1-04_medicalwalking4","THM1-05_medicalcombat1","THM1-06_medicalcombat2","THM1-07_medicalcombat3","THM1-08_medicalcombat4","THM1-09_medicalcombat5","THM1-10_medicalcombat6"};
const char* scienceMusic[10] = {"THM3-17_sciencestart","THM3-03_science1","THM3-04_science2","THM3-05_science3","THM3-06_science4","THM3-07_science5","THM3-08_science6","THM3-09_science7","THM3-01_scienceaction1","THM3-02_scienceaction2"};
const char* executiveMusic[13] = {"THM2-11_executive1","THM2-12_executive2","THM2-13_executive3","THM2-08_executive4","THM2-09_executive5","THM2-10_executive6","THM2-04_executive2","THM2-05_executive3","THM2-06_executivefluterlude","THM2-07_executivefluterludewithguitar","THM2-01_executiveaction3","THM2-02_executiveaction4","THM2-03_executiveaction5"};
const char* groveMusic[24] = {"THM5-07_groveaction1","THM5-08_groveaction1","THM5-09_groveaction2","THM5-10_groveaction3","THM5-11_groveaction4","THM5-12_groveaction5","THM5-13_groveaction6","THM5-14_groveaction7","THM5-15_groveaction8","THM5-33_grove1","THM5-34_grove2","THM5-38_grove3","THM5-39_grove4","THM5-40_grove5","THM5-35_grove99","THM5-36_grove100","THM5-37_grove101","THM5-41_grove102","THM5-42_grove103","THM5-01_grove105","THM5-02_grove106","THM5-03_grove107","THM5-04_grove108","THM5-05_grove109"};
const char* securityMusic[19] = {"THM6-05_securityaction1","THM6-06_securityaction2","THM6-07_securityaction3","THM6-08_securityaction4","THM6-09_securityaction5","THM6-10_securityaction6","THM6-01_security1","THM6-02_security2","THM6-03_security3","THM6-04_security4","THM6-11_security100","THM6-12_security101","THM6-13_security1","THM6-14_security2","THM6-15_security3","THM6-17_security4","THM6-18_security5","THM6-19_security6","THM6-20_security7"};
const char* cyberMusic[13] = {"THM10-02_cyberstart","THM10-01_cyber1","THM10-03_cyber2","THM10-04_cyber3","THM10-05_cyber4","THM10-06_cyber5","THM10-07_cyber6","THM10-08_cyber7","THM10-09_cyber8"};
const char* levelMusicElevator[13] = {"THM7-01_elevator1","THM7-01_elevator1","THM7-02_elevator2","THM7-03_elevator3","THM7-04_elevator4","THM7-05_elevator5","THM7-06_elevator6","THM7-07_elevator7","THM7-08_elevator8","THM7-01_elevator1","THM7-01_elevator1","THM7-01_elevator1","THM7-01_elevator1"};
const char* levelMusicRevive[MAX_LEVELS] = {"THM4-18_reactorrevive","THM1-18_medicalrevive","THM3-19_sciencerevive","THM3-19_sciencerevive","THM3-19_sciencerevive","THM1-18_medicalrevive","THM2-18_executiverevive","THM4-18_reactorrevive","THM6-22_securityrevive","THM1-18_medicalrevive","THM2-18_executiverevive","THM2-18_executiverevive","THM2-18_executiverevive","THM1-18_medicalrevive"};
const char* levelMusicDistortion[MAX_LEVELS] = {"THM6-49_securitydistorted","THM1-48_medicaldistorted","THM3-49_sciencedistorted","THM3-49_sciencedistorted","THM1-48_medicaldistorted","THM1-48_medicaldistorted","THM2-46_executivedistorted","THM1-48_medicaldistorted","THM6-49_securitydistorted","THM1-48_medicaldistorted","THM1-48_medicaldistorted","THM1-48_medicaldistorted","THM1-48_medicaldistorted","THM10-41_cyberdistorted"};
const char* levelMusicDeath[MAX_LEVELS] = {"THM0-17_death","THM1-17_death","THM3-18_death","THM0-17_death","THM3-18_death","THM0-17_death","THM2-17_death","THM0-17_death","THM6-21_death","THM0-17_death","THM5-17_death","THM5-17_death","THM5-17_death","THM10-16_death"};
void PlayMenuMusic() { mp3_clear(); play_mp3("./Audio/music/TITLOOP-00_menu.mp3",1500); }
void PlayGameMusic() { mp3_clear(); /*play_mp3("./Audio/music/THM1-19_medicalstart.mp3",100);*/ }
const char* GetCorrespondingLevelClip(TrackType ttype) {
    switch(ttype) { // Override types, return from these first before special level handling
        case TT_Revive:   return levelMusicRevive[World.curLev];   case TT_Death:      return levelMusicDeath[World.curLev];
        case TT_Elevator: return levelMusicElevator[World.curLev]; case TT_Distortion: return levelMusicDistortion[World.curLev];
    }
    if (World.curLev == 0 || World.curLev == 5 || World.curLev == 7) { // 0  REACTOR, 5 FLIGHT, 7 ENGINEERING
        if (World.Sys_Music.levelEntry) return reactorMusic[6];
        if (ttype == TT_Combat)  return reactorMusic[random_range_u8(0,6)];
        return reactorMusic[random_range_u8(6,13)];
    } else if (World.curLev == 1) { // 1  MEDICAL
        if (World.Sys_Music.levelEntry) return medicalMusic[0];
        if (ttype == TT_Combat)  return medicalMusic[random_range_u8(5,11)];
        return medicalMusic[random_range_u8(1,5)];
    } else if (World.curLev == 2 || World.curLev == 4) { // 2  SCIENCE, 4 STORAGE
        if (World.Sys_Music.levelEntry) return scienceMusic[0];
        if (ttype == TT_Combat)  return scienceMusic[random_range_u8(8,10)];
        return scienceMusic[random_range_u8(1,8)];
    } else if (World.curLev == 8) { // 8 SECURITY
        if (World.Sys_Music.levelEntry) return securityMusic[9];
        if (ttype == TT_Combat)  return securityMusic[random_range_u8(0,6)];
        return securityMusic[random_range_u8(6,19)];
    } else if (World.curLev == 6) { // 6 EXECUTIVE
        if (World.Sys_Music.levelEntry) return executiveMusic[0];
        if (ttype == TT_Combat)  return executiveMusic[random_range_u8(9,13)];
        return executiveMusic[random_range_u8(0,10)];
    } else if (World.curLev == 10 || World.curLev == 11 || World.curLev == 12) { // 10, 12 GROVES
        if (World.Sys_Music.levelEntry) return groveMusic[19];
        if (ttype == TT_Combat)  return groveMusic[random_range_u8(0,9)];
        return executiveMusic[random_range_u8(9,24)];
    } else if (World.curLev == 13) { // 13 CYBERSPACE
        if (World.Sys_Music.levelEntry)     return cyberMusic[0];
        if (World.Sys_Music.cyberTube)      return cyberMusic[random_range_u8(4,8)];
        if (random_range(0.0f,1.0f) < 0.5f) return cyberMusic[random_range_u8(1,5)];
        else                                return cyberMusic[8];
    }
    return levelMusicLooped[0];
}

void PlayTrack(TrackType ttype, MusicType mtype) {
    char p[128]; 
    if (!Sys_Settings.DynamicMusic) { // Looped Music (Dynamic Music off)
        if (mtype == MT_Override) {
                 if (ttype == TT_Revive){sFormat(p,sizeof(p),"./Audio/music/%s.mp3",levelMusicRevive[World.curLev]);}else if(ttype == TT_Death){sFormat(p,sizeof(p),"./Audio/music/%s.mp3",levelMusicDeath[World.curLev]);}
            else if (ttype == TT_Elevator){sFormat(p,sizeof(p),"./Audio/music/%s.mp3",levelMusicElevator[World.curLev]);}else if(ttype == TT_Distortion){sFormat(p,sizeof(p),"./Audio/music/%s.mp3",levelMusicDistortion[World.curLev]);}
        } else sFormat(p,sizeof(p),"./Audio/music/%s.mp3",levelMusicLooped[World.curLev]);
        play_mp3(p,0);
        return;
    } // Normal Dynamic Music System
    if (mtype == MT_Override) mp3_clear();
    sFormat(p,sizeof(p),"./Audio/music/%s.mp3",GetCorrespondingLevelClip(ttype));
    play_mp3(p,BUFFER_MS);
    if (!World.Sys_Music.elevator){World.Sys_Music.levelEntry=false;}
}

void UpdateMusic() {
    if (World.paused && !World.menuActive) { MP3Pause(); return; }
    MP3Resume();
    float remaining = GetMP3RemainingTime(); if(remaining > AUD_BUFFER_T){return;} if(World.menuActive){play_mp3("./Audio/music/TITLOOP-00_menu.mp3",1500); return;}
    if(World.Sys_Music.inCombat && !World.Sys_Music.inZone && World.Sys_Music.combatImpulseFinished < World.pauseRelativeTime) {
        World.Sys_Music.inCombat=false; PlayTrack(TT_Combat,MT_Override); World.Sys_Music.combatImpulseFinished=World.pauseRelativeTime + 20.0; return;
    }
    if(World.Sys_Music.inZone){ if(World.Sys_Music.distortion){PlayTrack(TT_Distortion,MT_Override); return;} if(World.Sys_Music.elevator){PlayTrack(TT_Elevator,MT_Override); return;} }
    if(Sys_Settings.DynamicMusic || remaining <= AUD_BUFFER_T){PlayTrack(TT_Walking,MT_Walking);}
}

void ResetLevelMusic(void) { mp3_clear(); World.Sys_Music.levelEntry = true; World.Sys_Music.inZone = World.Sys_Music.cyberTube = false; World.Sys_Music.clipFinished = World.Sys_Music.combatImpulseFinished = get_time(); World.Sys_Music.combatImpulseFinished += 5.0; }
void UpdateAudio() { if (!World.paused && !World.menuActive) {MixAmbs();} UpdateMusic(); }
