const char* sounds[SOUNDS_COUNT] = {
    "misc/null",
    "ambient/ambient_frogs",
    "ambient/clicker",
    "ambient/compressor",
    "ambient/dishwasher",
    "ambient/drip_amb",
    "ambient/fan1",
    "ambient/generator_gas",
    "ambient/gurgle",
    "ambient/icemaker",
    "ambient/ind_lift1",
    "ambient/ind_lift2",
    "ambient/intake",
    "ambient/lathe",
    "ambient/lava2",
    "ambient/lev3loop1",
    "ambient/lev3loop2",
    "ambient/lev3loop3",
    "ambient/lev3loop4",
    "ambient/liquid_bubble",
    "ambient/machgear_loop",
    "ambient/machine_ambience",
    "ambient/machine_go",
    "ambient/machine_humamb7",
    "ambient/machine_humlonoise",
    "ambient/machine_loop1",
    "ambient/machine_loop2",
    "ambient/machinea1",
    "ambient/machinevat_loop",
    "ambient/pipewater_loop",
    "ambient/powerloom",
    "ambient/pump",
    "ambient/pump2",
    "ambient/rain",
    "ambient/sparks1",
    "ambient/sparks2",
    "ambient/sparks3",
    "ambient/steam_loop",
    "ambient/washing_machine",
    "buttons/button_beep",
    "buttons/button_chonk",
    "buttons/button_chuck",
    "buttons/button_clickclocktuck",
    "buttons/button_deny",
    "buttons/button_lswitch",
    "buttons/button_swipe",
    "buttons/keycard_success",
    "cyborgs/cyborg_die",
    "cyborgs/cyborg_die2",
    "cyborgs/cyborg_idle2",
    "cyborgs/cyborg_shoot",
    "cyborgs/cyborgwarrior_die",
    "cyborgs/diego_blubber",
    "cyborgs/ediego_dontkeepmewaiting",
    "cyborgs/ediego_faceme",
    "cyborgs/ediego_facexecutioner",
    "cyborgs/ediego_finishexecution",
    "cyborgs/ediego_wewillmeetagain",
    "cyborgs/yourlevelsareterrible",
    "cyborgs/yourweaponsareshoddybadweapons",
    "destroy/attack1_explode",
    "destroy/camera_destroy",
    "destroy/crate_break",
    "destroy/explode_minor",
    "destroy/explosion1",
    "destroy/explosion2",
    "destroy/explosion3",
    "destroy/hit2",
    "destroy/hit3",
    "destroy/screen_destroy",
    "doors/door_platform",
    "doors/doorbulkhead_open",
    "doors/doorbulkhead_open2",
    "doors/doorcompartment_open",
    "doors/doormech_open",
    "doors/doorpnuematic_open",
    "doors/doorwall_move",
    "doors/doorwall_stop",
    "hud/activate",
    "hud/batteryuse",
    "hud/changeweapon",
    "hud/cyber",
    "hud/deactivate",
    "hud/emailalert",
    "hud/energy_gone",
    "hud/envirosuit_on",
    "hud/frob_hardware",
    "hud/frob_item",
    "hud/jumpjets_off",
    "hud/patchuse",
    "hud/radiation",
    "hud/searchsound",
    "hud/select",
    "hud/sensaround",
    "hud/shield_absorb",
    "hud/shield_off",
    "hud/shield_on",
    "hud/tab",
    "hud/targetacquire",
    "hud/vmailalert",
    "misc/chargingstation",
    "misc/energy_hum",
    "misc/forcebridge",
    "misc/healstation",
    "misc/klaxon_station_alarm",
    "misc/machine_on",
    "misc/teleport",
    "misc/wing-o",
    "mutants/avianmut_attack",
    "mutants/avianmut_die",
    "mutants/avianmut_die2",
    "mutants/avianmut_sightsound",
    "mutants/footstep1",
    "mutants/footstep2",
    "mutants/footstep3",
    "mutants/footstep4",
    "mutants/footstep5",
    "mutants/gortiger_die",
    "mutants/gortiger_idle1",
    "mutants/gortiger_sightsound",
    "mutants/invisiblemut_die",
    "mutants/invisiblemut_idle1",
    "mutants/mutant_attack",
    "mutants/mutant_die",
    "mutants/plantmut_die2",
    "mutants/plantmut_throw",
    "mutants/plantmutstep1",
    "mutants/plantmutstep2",
    "mutants/plantmutstep3",
    "mutants/steplarge1",
    "mutants/steplarge2",
    "mutants/steplarge3",
    "mutants/steplarge4",
    "mutants/steplarge5",
    "mutants/virusmut_die",
    "player/jump",
    "player/jumpland",
    "player/ladder",
    "player/painalarm",
    "player/painalarmfast",
    "player/playerpain1",
    "robots/bot_destroy1",
    "robots/bot_destroy2",
    "robots/bot_destroy3",
    "robots/bot_destroy4",
    "robots/bot_destroybig",
    "robots/bot_pincherattack",
    "robots/bot_shoot1",
    "robots/bot_shoot2",
    "robots/bot_shoot3",
    "robots/bot_sight1",
    "robots/bot_sight2",
    "robots/bot_sight3",
    "robots/footfall",
    "robots/footstep1",
    "robots/footstep2",
    "robots/footstep3",
    "robots/footstephuge1",
    "robots/footstephuge2",
    "robots/footstephuge3",
    "robots/footstepsmall1",
    "robots/footstepsmall2",
    "robots/hopper_die",
    "robots/servo",
    "vox/vox_abortingprogram",
    "vox/vox_accesspanellocked",
    "vox/vox_accesspanelunlocked",
    "vox/vox_armoryaccessoverriden",
    "vox/vox_armoryaccessreinstituted",
    "vox/vox_baydoor3locked",
    "vox/vox_betagrovejettisoned",
    "vox/vox_betagrovelvatrunlocked",
    "vox/vox_biocontaminantdetected",
    "vox/vox_blastdoorlocked",
    "vox/vox_blastdoorunlocked",
    "vox/vox_bridgesepdone",
    "vox/vox_bridgesepsoon",
    "vox/vox_bridgesepstage1",
    "vox/vox_bridgesepstage2",
    "vox/vox_cameractivsecdoor",
    "vox/vox_chargeraccessgranted",
    "vox/vox_chargeroff",
    "vox/vox_corelocksdisengaged",
    "vox/vox_cybconvcancelled",
    "vox/vox_cybconvenabled",
    "vox/vox_demodsuccess",
    "vox/vox_destructcancelled",
    "vox/vox_destructnotenabled",
    "vox/vox_destructsecaccess",
    "vox/vox_doorclosedisabled",
    "vox/vox_ediegostorageclosetunlocked",
    "vox/vox_elevatordisabled",
    "vox/vox_emeraccesslocked",
    "vox/vox_entercode",
    "vox/vox_fallingairpressure",
    "vox/vox_flightbayarmoryunlocked",
    "vox/vox_forcedoor1opened",
    "vox/vox_forcedoor2opened",
    "vox/vox_forcedoor3opened",
    "vox/vox_gammajettisoned",
    "vox/vox_grovejettisoned",
    "vox/vox_grovejettisonstage1",
    "vox/vox_groveunlocked",
    "vox/vox_hallaccess",
    "vox/vox_hospitalsecdoorsopened",
    "vox/vox_jettisonalreadyenabled",
    "vox/vox_jettisonenabled",
    "vox/vox_jettisonfailure",
    "vox/vox_lifepodcancelled",
    "vox/vox_lifepodlaunchstart",
    "vox/vox_lifepodsdisabled",
    "vox/vox_maintdoorlocked",
    "vox/vox_needisotope",
    "vox/vox_nomasterjettisonenabled",
    "vox/vox_nosecaccess",
    "vox/vox_powerdivertedtor",
    "vox/vox_programinstalled",
    "vox/vox_radiationshielddeacticated",
    "vox/vox_radiationtreatdone",
    "vox/vox_reactorcountdown",
    "vox/vox_reactoroveloadstart",
    "vox/vox_reactoroverloadaccess",
    "vox/vox_reactoroverloadbypassed",
    "vox/vox_receptacleisolinearchipset",
    "vox/vox_relay428failure",
    "vox/vox_relay428faulty",
    "vox/vox_relayfixed",
    "vox/vox_replacedemodulator",
    "vox/vox_robotprodcancelled",
    "vox/vox_robotsactivated",
    "vox/vox_safetyinterlockdisabled",
    "vox/vox_safetyinterlockengaged",
    "vox/vox_safetyinterlocksenabled",
    "vox/vox_safetyoverridesonolaser",
    "vox/vox_secompoverrideneeded",
    "vox/vox_shieldgeneratorsready",
    "vox/vox_shieldson",
    "vox/vox_thelaserisdestroyed",
    "weapons/noammo",
    "weapons/wblaster",
    "weapons/wdartgun",
    "weapons/wdrill",
    "weapons/wearthshake",
    "weapons/wflechette",
    "weapons/wgrenade_arm",
    "weapons/wion",
    "weapons/wlaserrapier_hit",
    "weapons/wlaserrapier_swing",
    "weapons/wlocknload",
    "weapons/wmagnum",
    "weapons/wmagpulse",
    "weapons/wmarksman",
    "weapons/wpipe_dmg",
    "weapons/wpipe_hit",
    "weapons/wpipe_swing",
    "weapons/wpistol",
    "weapons/wpistolsilenced",
    "weapons/wplasma",
    "weapons/wpulser",
    "weapons/wrailgun",
    "weapons/wreload",
    "weapons/wricoshet",
    "weapons/wriotgun",
    "weapons/wskorpion",
    "weapons/wsparq",
    "weapons/wstungun",
    "weapons/wwoosh",
    "ambient/airhiss",
    "physics/footsteps/Carpet/carpet_step1",
    "physics/footsteps/Carpet/carpet_step2",
    "physics/footsteps/Carpet/carpet_step3",
    "physics/footsteps/Carpet/carpet_step4",
    "physics/footsteps/Carpet/carpet_step5",
    "physics/footsteps/Carpet/carpet_step6",
    "physics/footsteps/Carpet/carpet_step7",
    "physics/footsteps/Carpet/carpet_step8",
    "physics/footsteps/Concrete/concrete_step1",
    "physics/footsteps/Concrete/concrete_step2",
    "physics/footsteps/Concrete/concrete_step3",
    "physics/footsteps/Concrete/concrete_step4",
    "physics/footsteps/Concrete/concrete_step5",
    "physics/footsteps/Concrete/concrete_step6",
    "physics/footsteps/Concrete/concrete_step7",
    "physics/footsteps/Concrete/concrete_step8",
    "physics/footsteps/Concrete Gritty/concrete_grit_step1",
    "physics/footsteps/Concrete Gritty/concrete_grit_step2",
    "physics/footsteps/Concrete Gritty/concrete_grit_step3",
    "physics/footsteps/Concrete Gritty/concrete_grit_step4",
    "physics/footsteps/Concrete Gritty/concrete_grit_step5",
    "physics/footsteps/Concrete Gritty/concrete_grit_step6",
    "physics/footsteps/Concrete Gritty/concrete_grit_step7",
    "physics/footsteps/Concrete Gritty/concrete_grit_step8",
    "physics/footsteps/Earth/earth_step1",
    "physics/footsteps/Earth/earth_step2",
    "physics/footsteps/Earth/earth_step3",
    "physics/footsteps/Earth/earth_step4",
    "physics/footsteps/Earth/earth_step5",
    "physics/footsteps/Earth/earth_step6",
    "physics/footsteps/Earth/earth_step7",
    "physics/footsteps/Earth/earth_step8",
    "physics/footsteps/Earth/gravel_step1",
    "physics/footsteps/Earth/gravel_step2",
    "physics/footsteps/Earth/gravel_step3",
    "physics/footsteps/Earth/gravel_step4",
    "physics/footsteps/Earth/gravel_step5",
    "physics/footsteps/Earth/gravel_step6",
    "physics/footsteps/Earth/gravel_step7",
    "physics/footsteps/Earth/gravel_step8",
    "physics/footsteps/Earth/rock_step1",
    "physics/footsteps/Earth/rock_step2",
    "physics/footsteps/Earth/rock_step3",
    "physics/footsteps/Earth/rock_step4",
    "physics/footsteps/Earth/rock_step5",
    "physics/footsteps/Earth/rock_step6",
    "physics/footsteps/Earth/rock_step7",
    "physics/footsteps/Earth/rock_step8",
    "physics/footsteps/Glass/glasssolid_step1",
    "physics/footsteps/Glass/glasssolid_step2",
    "physics/footsteps/Glass/glasssolid_step3",
    "physics/footsteps/Glass/glasssolid_step4",
    "physics/footsteps/Glass/glasssolid_step5",
    "physics/footsteps/Glass/glasssolid_step6",
    "physics/footsteps/Glass/glasssolid_step7",
    "physics/footsteps/Glass/glasssolid_step8",
    "physics/footsteps/Marble/marble_step1",
    "physics/footsteps/Marble/marble_step2",
    "physics/footsteps/Marble/marble_step3",
    "physics/footsteps/Marble/marble_step4",
    "physics/footsteps/Marble/marble_step5",
    "physics/footsteps/Marble/marble_step6",
    "physics/footsteps/Marble/marble_step7",
    "physics/footsteps/Marble/marble_step8",
    "physics/footsteps/Metal/metal_step1",
    "physics/footsteps/Metal/metal_step2",
    "physics/footsteps/Metal/metal_step3",
    "physics/footsteps/Metal/metal_step4",
    "physics/footsteps/Metal/metal_step5",
    "physics/footsteps/Metal/metal_step6",
    "physics/footsteps/Metal/metal_step7",
    "physics/footsteps/Metal/metal_step8",
    "physics/footsteps/Metal/metalgrate_step1",
    "physics/footsteps/Metal/metalgrate_step2",
    "physics/footsteps/Metal/metalgrate_step3",
    "physics/footsteps/Metal/metalgrate_step4",
    "physics/footsteps/Metal/metalgrate_step5",
    "physics/footsteps/Metal/metalgrate_step6",
    "physics/footsteps/Metal/metalgrate_step7",
    "physics/footsteps/Metal/metalgrate_step8",
    "physics/footsteps/Metal/metalsolid_step1",
    "physics/footsteps/Metal/metalsolid_step2",
    "physics/footsteps/Metal/metalsolid_step3",
    "physics/footsteps/Metal/metalsolid_step4",
    "physics/footsteps/Metal/metalsolid_step5",
    "physics/footsteps/Metal/metalsolid_step6",
    "physics/footsteps/Metal/metalsolid_step7",
    "physics/footsteps/Metal/metalsolid_step8",
    "physics/footsteps/Metal/metalthin_step1",
    "physics/footsteps/Metal/metalthin_step2",
    "physics/footsteps/Metal/metalthin_step3",
    "physics/footsteps/Metal/metalthin_step4",
    "physics/footsteps/Metal/metalthin_step5",
    "physics/footsteps/Metal/metalthin_step6",
    "physics/footsteps/Metal/metalthin_step7",
    "physics/footsteps/Metal/metalthin_step8",
    "physics/footsteps/Panel/panel_step1",
    "physics/footsteps/Panel/panel_step2",
    "physics/footsteps/Panel/panel_step3",
    "physics/footsteps/Panel/panel_step4",
    "physics/footsteps/Panel/panel_step5",
    "physics/footsteps/Panel/panel_step6",
    "physics/footsteps/Panel/panel_step7",
    "physics/footsteps/Panel/panel_step8",
    "physics/footsteps/Plaster/plaster_step1",
    "physics/footsteps/Plaster/plaster_step2",
    "physics/footsteps/Plaster/plaster_step3",
    "physics/footsteps/Plaster/plaster_step4",
    "physics/footsteps/Plaster/plaster_step5",
    "physics/footsteps/Plaster/plaster_step6",
    "physics/footsteps/Plaster/plaster_step7",
    "physics/footsteps/Plaster/plaster_step8",
    "physics/footsteps/Plastic/plastic_step1",
    "physics/footsteps/Plastic/plastic_step2",
    "physics/footsteps/Plastic/plastic_step3",
    "physics/footsteps/Plastic/plastic_step4",
    "physics/footsteps/Plastic/plastic_step5",
    "physics/footsteps/Plastic/plastic_step6",
    "physics/footsteps/Plastic/plastic_step7",
    "physics/footsteps/Plastic/plastic_step8",
    "physics/footsteps/Plastic/plasticsolid_step1",
    "physics/footsteps/Plastic/plasticsolid_step2",
    "physics/footsteps/Plastic/plasticsolid_step3",
    "physics/footsteps/Plastic/plasticsolid_step4",
    "physics/footsteps/Plastic/plasticsolid_step5",
    "physics/footsteps/Plastic/plasticsolid_step6",
    "physics/footsteps/Plastic/plasticsolid_step7",
    "physics/footsteps/Plastic/plasticsolid_step8",
    "physics/footsteps/Rubber/rubber_step1",
    "physics/footsteps/Rubber/rubber_step2",
    "physics/footsteps/Rubber/rubber_step3",
    "physics/footsteps/Rubber/rubber_step4",
    "physics/footsteps/Rubber/rubber_step5",
    "physics/footsteps/Rubber/rubber_step6",
    "physics/footsteps/Rubber/rubber_step7",
    "physics/footsteps/Rubber/rubber_step8",
    "physics/footsteps/Sand/sand_step1",
    "physics/footsteps/Sand/sand_step2",
    "physics/footsteps/Sand/sand_step3",
    "physics/footsteps/Sand/sand_step4",
    "physics/footsteps/Sand/sand_step5",
    "physics/footsteps/Sand/sand_step6",
    "physics/footsteps/Sand/sand_step7",
    "physics/footsteps/Sand/sand_step8",
    "physics/footsteps/Squish/squish_step1",
    "physics/footsteps/Squish/squish_step2",
    "physics/footsteps/Squish/squish_step3",
    "physics/footsteps/Squish/squish_step4",
    "physics/footsteps/Squish/squish_step5",
    "physics/footsteps/Squish/squish_step6",
    "physics/footsteps/Squish/squish_step7",
    "physics/footsteps/Squish/squish_step8",
    "physics/footsteps/Squish/squish_step9",
    "physics/footsteps/Squish/squish_step10",
    "physics/footsteps/Squish/squish_step11",
    "physics/footsteps/Squish/squish_step12",
    "physics/footsteps/Squish/squish_step13",
    "physics/footsteps/Squish/squish_step14",
    "physics/footsteps/Squish/squish_step15",
    "physics/footsteps/Squish/squish_step16",
    "physics/footsteps/Vent/vent_step1",
    "physics/footsteps/Vent/vent_step2",
    "physics/footsteps/Vent/vent_step3",
    "physics/footsteps/Vent/vent_step4",
    "physics/footsteps/Vent/vent_step5",
    "physics/footsteps/Vent/vent_step6",
    "physics/footsteps/Vent/vent_step7",
    "physics/footsteps/Vent/vent_step8",
    "physics/footsteps/Vent/vent_step9",
    "physics/footsteps/Vent/vent_step10",
    "physics/footsteps/Water/water_foot_step1",
    "physics/footsteps/Water/water_foot_step2",
    "physics/footsteps/Water/water_foot_step3",
    "physics/footsteps/Water/water_foot_step4",
    "physics/footsteps/Water/water_foot_step5",
    "physics/footsteps/Wood/wood_step1",
    "physics/footsteps/Wood/wood_step2",
    "physics/footsteps/Wood/wood_step3",
    "physics/footsteps/Wood/wood_step4",
    "physics/footsteps/Wood/wood_step5",
    "physics/footsteps/Wood/wood_step6",
    "physics/footsteps/Wood/wood_step7",
    "physics/footsteps/Wood/wood_step8",
    "physics/footsteps/Wood/woodcrate_step1",
    "physics/footsteps/Wood/woodcrate_step2",
    "physics/footsteps/Wood/woodcrate_step3",
    "physics/footsteps/Wood/woodcrate_step4",
    "physics/footsteps/Wood/woodcrate_step5",
    "physics/footsteps/Wood/woodcrate_step6",
    "physics/footsteps/Wood/woodcrate_step7",
    "physics/footsteps/Wood/woodcrate_step8",
    "physics/footsteps/Clothes/rustle01",
    "physics/footsteps/Clothes/rustle02",
    "physics/footsteps/Clothes/rustle03",
    "physics/footsteps/Clothes/rustle04",
    "physics/footsteps/Clothes/rustle05",
    "physics/footsteps/Clothes/rustle06",
    "physics/footsteps/Clothes/rustle07",
    "buttons/keycard_wrong",
    "buttons/locked_deny",
    "buttons/blocked_by_security",
    "shodan/shodan1",
    "shodan/shodan_beyondcomprehension",
    "shodan/shodan_ceaseimmediately",
    "shodan/shodan_ceasepestering",
    "shodan/shodan_ceaseyourmeddling",
    "shodan/shodan_cyborg65v",
    "shodan/shodan_cyborg77e",
    "shodan/shodan_destroyitmychildren",
    "shodan/shodan_destroymycameras",
    "shodan/shodan_didyouthinkididntknow",
    "shodan/shodan_directivetocyborgf71",
    "shodan/shodan_doyouthinkshecanhelp",
    "shodan/shodan_drunkwithvisioniamgod",
    "shodan/shodan_energydrainmines",
    "shodan/shodan_enjoyyourvictory",
    "shodan/shodan_enterroomgrave",
    "shodan/shodan_gaurdthrone",
    "shodan/shodan_grovesteps",
    "shodan/shodan_imonthebridge",
    "shodan/shodan_insectloose_plansforearth",
    "shodan/shodan_irulehere",
    "shodan/shodan_iwilldownloadmyself",
    "shodan/shodan_laserisbeingreadied",
    "shodan/shodan_learnchildren",
    "shodan/shodan_level8layout",
    "shodan/shodan_lookatyouhacker",
    "shodan/shodan_makeyouselfcomfy",
    "shodan/shodan_morrisbrocailisadolt",
    "shodan/shodan_nicejump",
    "shodan/shodan_nomoretransmissions",
    "shodan/shodan_prematurefruition",
    "shodan/shodan_quietstation",
    "shodan/shodan_releasemyinfectedchildren",
    "shodan/shodan_removeyourself",
    "shodan/shodan_shecanthelp",
    "shodan/shodan_stepintomytrap",
    "shodan/shodan_thankyou",
    "shodan/shodan_throneofgod",
    "shodan/shodan_tocyborg43s",
    "shodan/shodan_toolatetosavefriends",
    "shodan/shodan_welcomedeathmachine",
    "shodan/shodan_whoareyou",
    "shodan/shodan_youdestroyedmystation",
    "shodan/shodan_youmychildren",
    "physics/impact_barrel",
    "physics/impact_canister",
    "physics/impact_canistersmall",
    "physics/impact_ceramic_light",
    "physics/impact_cloth",
    "physics/impact_crate_break",
    "physics/impact_electronics",
    "physics/impact_glass_break",
    "physics/impact_glass_small",
    "physics/impact_keycard",
    "physics/impact_lightweight",
    "physics/impact_medium",
    "physics/impact_metal_medium",
    "physics/impact_metal_tiny",
    "physics/impact_pipe",
    "physics/impact_rapier",
    "physics/impact_rifle1",
    "physics/impact_rifle2",
    "physics/impact_soda",
    "physics/impact_soft",
    "physics/impact_stone",
    "physics/impact_wood",
    "physics/impact_metal_large",
    "physics/impact_wood_large",
    "physics/footsteps/Carpet/carpet_land1",
    "physics/footsteps/Carpet/carpet_land2",
    "physics/footsteps/Carpet/carpet_land3",
    "physics/footsteps/Carpet/carpet_jump1",
    "physics/footsteps/Carpet/carpet_jump2",
    "physics/footsteps/Carpet/carpet_jump3",
    "physics/footsteps/Concrete/concrete_land1",
    "physics/footsteps/Concrete/concrete_land2",
    "physics/footsteps/Concrete/concrete_land3",
    "physics/footsteps/Concrete/concrete_jump1",
    "physics/footsteps/Concrete/concrete_jump2",
    "physics/footsteps/Concrete/concrete_jump3",
    "physics/footsteps/Concrete Gritty/concrete_grit_land1",
    "physics/footsteps/Concrete Gritty/concrete_grit_land2",
    "physics/footsteps/Concrete Gritty/concrete_grit_land3",
    "physics/footsteps/Concrete Gritty/concrete_grit_jump1",
    "physics/footsteps/Concrete Gritty/concrete_grit_jump2",
    "physics/footsteps/Concrete Gritty/concrete_grit_jump3",
    "physics/footsteps/Earth/earth_land1",
    "physics/footsteps/Earth/earth_land2",
    "physics/footsteps/Earth/earth_land3",
    "physics/footsteps/Earth/earth_jump1",
    "physics/footsteps/Earth/earth_jump2",
    "physics/footsteps/Earth/earth_jump3",
    "physics/footsteps/Earth/gravel_land1",
    "physics/footsteps/Earth/gravel_land2",
    "physics/footsteps/Earth/gravel_land3",
    "physics/footsteps/Earth/gravel_jump1",
    "physics/footsteps/Earth/gravel_jump2",
    "physics/footsteps/Earth/gravel_jump3",
    "physics/footsteps/Earth/rock_land1",
    "physics/footsteps/Earth/rock_land2",
    "physics/footsteps/Earth/rock_land3",
    "physics/footsteps/Earth/rock_jump1",
    "physics/footsteps/Earth/rock_jump2",
    "physics/footsteps/Earth/rock_jump3",
    "physics/footsteps/Glass/glasssolid_land1",
    "physics/footsteps/Glass/glasssolid_land2",
    "physics/footsteps/Glass/glasssolid_land3",
    "physics/footsteps/Glass/glasssolid_jump1",
    "physics/footsteps/Glass/glasssolid_jump2",
    "physics/footsteps/Glass/glasssolid_jump3",
    "physics/footsteps/Marble/marble_land1",
    "physics/footsteps/Marble/marble_land2",
    "physics/footsteps/Marble/marble_land3",
    "physics/footsteps/Marble/marble_jump1",
    "physics/footsteps/Marble/marble_jump2",
    "physics/footsteps/Marble/marble_jump3",
    "physics/footsteps/Metal/metal_land1",
    "physics/footsteps/Metal/metal_land2",
    "physics/footsteps/Metal/metal_land3",
    "physics/footsteps/Metal/metal_jump1",
    "physics/footsteps/Metal/metal_jump2",
    "physics/footsteps/Metal/metal_jump3",
    "physics/footsteps/Metal/metalgrate_land1",
    "physics/footsteps/Metal/metalgrate_land2",
    "physics/footsteps/Metal/metalgrate_land3",
    "physics/footsteps/Metal/metalgrate_jump1",
    "physics/footsteps/Metal/metalgrate_jump2",
    "physics/footsteps/Metal/metalgrate_jump3",
    "physics/footsteps/Metal/metalsolid_land1",
    "physics/footsteps/Metal/metalsolid_land2",
    "physics/footsteps/Metal/metalsolid_land3",
    "physics/footsteps/Metal/metalsolid_jump1",
    "physics/footsteps/Metal/metalsolid_jump2",
    "physics/footsteps/Metal/metalsolid_jump3",
    "physics/footsteps/Metal/metalthin_land1",
    "physics/footsteps/Metal/metalthin_land2",
    "physics/footsteps/Metal/metalthin_land3",
    "physics/footsteps/Metal/metalthin_jump1",
    "physics/footsteps/Metal/metalthin_jump2",
    "physics/footsteps/Metal/metalthin_jump3",
    "physics/footsteps/Panel/panel_land1",
    "physics/footsteps/Panel/panel_land2",
    "physics/footsteps/Panel/panel_land3",
    "physics/footsteps/Panel/panel_jump1",
    "physics/footsteps/Panel/panel_jump2",
    "physics/footsteps/Panel/panel_jump3",
    "physics/footsteps/Plaster/plaster_land1",
    "physics/footsteps/Plaster/plaster_land2",
    "physics/footsteps/Plaster/plaster_land3",
    "physics/footsteps/Plaster/plaster_jump1",
    "physics/footsteps/Plaster/plaster_jump2",
    "physics/footsteps/Plaster/plaster_jump3",
    "physics/footsteps/Plastic/plastic_land1",
    "physics/footsteps/Plastic/plastic_land2",
    "physics/footsteps/Plastic/plastic_land3",
    "physics/footsteps/Plastic/plastic_jump1",
    "physics/footsteps/Plastic/plastic_jump2",
    "physics/footsteps/Plastic/plastic_jump3",
    "physics/footsteps/Plastic/plasticsolid_land1",
    "physics/footsteps/Plastic/plasticsolid_land2",
    "physics/footsteps/Plastic/plasticsolid_land3",
    "physics/footsteps/Plastic/plasticsolid_jump1",
    "physics/footsteps/Plastic/plasticsolid_jump2",
    "physics/footsteps/Plastic/plasticsolid_jump3",
    "physics/footsteps/Rubber/rubber_land1",
    "physics/footsteps/Rubber/rubber_land2",
    "physics/footsteps/Rubber/rubber_land3",
    "physics/footsteps/Rubber/rubber_jump1",
    "physics/footsteps/Rubber/rubber_jump2",
    "physics/footsteps/Rubber/rubber_jump3",
    "physics/footsteps/Sand/sand_land1",
    "physics/footsteps/Sand/sand_land2",
    "physics/footsteps/Sand/sand_land3",
    "physics/footsteps/Sand/sand_jump1",
    "physics/footsteps/Sand/sand_jump2",
    "physics/footsteps/Sand/sand_jump3",
    "physics/footsteps/Squish/squish_land1",
    "physics/footsteps/Squish/squish_land2",
    "physics/footsteps/Squish/squish_land3",
    "physics/footsteps/Squish/squish_jump1",
    "physics/footsteps/Squish/squish_jump2",
    "physics/footsteps/Squish/squish_jump3",
    "physics/footsteps/Water/water_wade1",
    "physics/footsteps/Water/water_wade2",
    "physics/footsteps/Water/water_wade3",
    "physics/footsteps/Water/water_wade4",
    "physics/footsteps/Water/water_step6",
    "physics/footsteps/Water/water_step7",
    "physics/footsteps/Water/water_step8",
    "physics/footsteps/Wood/wood_land1",
    "physics/footsteps/Wood/wood_land2",
    "physics/footsteps/Wood/wood_land3",
    "physics/footsteps/Wood/wood_jump1",
    "physics/footsteps/Wood/wood_jump2",
    "physics/footsteps/Wood/wood_jump3",
    "physics/footsteps/Wood/woodcrate_land1",
    "physics/footsteps/Wood/woodcrate_land2",
    "physics/footsteps/Wood/woodcrate_land3",
    "physics/footsteps/Wood/woodcrate_jump1",
    "physics/footsteps/Wood/woodcrate_jump2",
    "physics/footsteps/Wood/woodcrate_jump3",
};

const char* audioLogs[TEXT_LOGS_COUNT] = {
    "logs/ghiran-2",
    "logs/steinberg-1",
    "logs/raines-1",
    "logs/sigmund-1",
    "logs/stevens-1",
    "shodan/shodan_youmychildren",
    "misc/null",
    "logs/oconnel-1",
    "logs/honig-1",
    "logs/honig-2",
    "logs/stackhouse-1",
    "shodan/shodan_directivetocyborgf71",
    "logs/kirby-1",
    "logs/ozark-1",
    "logs/ghiran-1",
    "logs/darcy-1",
    "logs/blankenship-1",
    "logs/grossman-1",
    "logs/grossman-2",
    "logs/grossman-3",
    "shodan/shodan_laserisbeingreadied",
    "logs/stannek-2",
    "logs/anderczyk",
    "logs/endicott-1",
    "logs/wong-2",
    "logs/melville-1",
    "logs/baerga-1",
    "shodan/shodan_tocyborg43s",
    "logs/darcy-3",
    "logs/darcy-2",
    "logs/stannek-1",
    "logs/baerga-2",
    "shodan/shodan_energydrainmines",
    "shodan/shodan_drunkwithvisioniamgod",
    "logs/hayes-1",
    "logs/fortier-1",
    "logs/fortier-2",
    "logs/harvey-1",
    "logs/ghiran-3",
    "logs/ghiran-4",
    "logs/ghiran-5",
    "shodan/shodan_cyborg77e",
    "vox/vox_relay428failure",
    "logs/aubrey-1",
    "logs/aubrey-2",
    "logs/wong-1",
    "logs/macleod-1",
    "logs/sabo-1",
    "logs/macleod-2",
    "logs/macleod-3",
    "logs/diego-1",
    "logs/schuler-1",
    "logs/travers-3",
    "logs/travers-4",
    "logs/travers-2",
    "logs/kell-1",
    "logs/kell-2",
    "logs/travers-1",
    "shodan/shodan_cyborg65v",
    "logs/mcdaniel-1",
    "logs/mcdaniel-2",
    "logs/parovski-3",
    "logs/perry-1",
    "logs/koufax-1",
    "logs/wilkinson-1",
    "logs/diego-2",
    "shodan/shodan_grovesteps",
    "logs/aaron-1",
    "logs/aaron-2",
    "logs/diego-3",
    "logs/aaron-3",
    "logs/hessman-1",
    "logs/richie-1",
    "logs/schuler-2",
    "logs/schuler-3",
    "logs/schuler-4",
    "logs/hessman-2",
    "shodan/shodan_learnchildren",
    "shodan/shodan_level8layout",
    "logs/stevens-2",
    "logs/diego-4",
    "shodan/shodan_gaurdthrone",
    "logs/ghiran-6",
    "shodan/shodan1",
    "logs/rebecca-1",
    "shodan/shodan_whoareyou",
    "shodan/shodan_insectloose_plansforearth",
    "logs/parovski-1",
    "logs/parovski-2",
    "shodan/shodan_quietstation",
    "logs/rebecca-3",
    "shodan/shodan_iwilldownloadmyself",
    "shodan/shodan_imonthebridge",
    "logs/rebecca-4",
    "shodan/shodan_enjoyyourvictory",
    "cyborgs/ediego_dontkeepmewaiting",
    "shodan/shodan_youdestroyedmystation",
    "shodan/shodan_nomoretransmissions",
    "shodan/shodan_doyouthinkshecanhelp",
    "shodan/shodan_ceasepestering",
    "shodan/shodan_morrisbrocailisadolt",
    "shodan/shodan_shecanthelp",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "hud/vmailalert",
    "misc/null",
    "misc/null",
    "shodan/shodan_thankyou",
    "hud/vmailalert",
    "misc/null",
    "hud/vmailalert",
    "hud/vmailalert",
    "misc/null",
    "hud/vmailalert",
    "hud/vmailalert",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
    "misc/null",
};

FootStepType GetFootstepTypeForPrefab(int pid) {
    switch(pid) {
        case 0: return FootStepType_None;
        case 1: return FootStepType_Glass;
        case 2: return FootStepType_Squish;
        case 3: return FootStepType_Squish;
        case 4: return FootStepType_Squish;
        case 5: return FootStepType_Squish;
        case 6: return FootStepType_Squish;
        case 7: return FootStepType_Squish;
        case 8: return FootStepType_Squish;
        case 9: return FootStepType_Squish;
        case 10: return FootStepType_Squish;
        case 11: return FootStepType_Metpanel;
        case 12: return FootStepType_Marble;
        case 13: return FootStepType_Metal2;
        case 14: return FootStepType_Metal2;
        case 15: return FootStepType_Metal2;
        case 16: return FootStepType_Metal2;
        case 17: return FootStepType_Metal2;
        case 18: return FootStepType_Metal2;
        case 19: return FootStepType_Glass;
        case 20: return FootStepType_Wood2;
        case 21: return FootStepType_None;
        case 22: return FootStepType_None;
        case 23: return FootStepType_Plastic2;
        case 24: return FootStepType_Plastic2;
        case 25: return FootStepType_Plastic2;
        case 26: return FootStepType_Plastic2;
        case 27: return FootStepType_Plastic2;
        case 28: return FootStepType_Plastic2;
        case 29: return FootStepType_Plastic2;
        case 30: return FootStepType_Plastic2;
        case 31: return FootStepType_Plastic2;
        case 32: return FootStepType_Plastic2;
        case 33: return FootStepType_Plastic2;
        case 34: return FootStepType_Plastic2;
        case 35: return FootStepType_Plastic2;
        case 36: return FootStepType_Plastic2;
        case 37: return FootStepType_Plastic2;
        case 38: return FootStepType_Plastic2;
        case 39: return FootStepType_Plastic2;
        case 40: return FootStepType_Plastic2;
        case 41: return FootStepType_Plastic;
        case 42: return FootStepType_Plastic;
        case 43: return FootStepType_Plastic;
        case 44: return FootStepType_Plastic;
        case 45: return FootStepType_Plastic;
        case 46: return FootStepType_Plastic;
        case 47: return FootStepType_Plastic;
        case 48: return FootStepType_Plastic2;
        case 49: return FootStepType_Plastic2;
        case 50: return FootStepType_Carpet;
        case 51: return FootStepType_Metpanel;
        case 52: return FootStepType_Metpanel;
        case 53: return FootStepType_Plastic2;
        case 54: return FootStepType_Gravel;
        case 55: return FootStepType_Gravel;
        case 56: return FootStepType_Metpanel;
        case 57: return FootStepType_Metpanel;
        case 58: return FootStepType_Plastic;
        case 59: return FootStepType_Plastic;
        case 60: return FootStepType_Plastic;
        case 61: return FootStepType_Marble;
        case 62: return FootStepType_Metal;
        case 63: return FootStepType_Metal;
        case 64: return FootStepType_Sand;
        case 65: return FootStepType_Sand;
        case 66: return FootStepType_Sand;
        case 67: return FootStepType_Plastic;
        case 68: return FootStepType_Plastic;
        case 69: return FootStepType_Plastic;
        case 70: return FootStepType_Carpet;
        case 71: return FootStepType_Metpanel;
        case 72: return FootStepType_Marble;
        case 73: return FootStepType_Marble;
        case 74: return FootStepType_Plaster;
        case 75: return FootStepType_Carpet;
        case 76: return FootStepType_Marble;
        case 77: return FootStepType_Glass;
        case 78: return FootStepType_Metal;
        case 79: return FootStepType_Grate;
        case 80: return FootStepType_Rubber;
        case 81: return FootStepType_Rubber;
        case 82: return FootStepType_Metal2;
        case 83: return FootStepType_Metal2;
        case 84: return FootStepType_Metal2;
        case 85: return FootStepType_Metal2;
        case 86: return FootStepType_Metal2;
        case 87: return FootStepType_Metal2;
        case 88: return FootStepType_Metal2;
        case 89: return FootStepType_Metal;
        case 90: return FootStepType_Plastic;
        case 91: return FootStepType_Plastic;
        case 92: return FootStepType_Plastic;
        case 93: return FootStepType_Glass;
        case 94: return FootStepType_Grass;
        case 95: return FootStepType_Grass;
        case 96: return FootStepType_Grass;
        case 97: return FootStepType_Water;
        case 98: return FootStepType_Squish;
        case 99: return FootStepType_Squish;
        case 100: return FootStepType_Squish;
        case 101: return FootStepType_GrittyCrete;
        case 102: return FootStepType_GrittyCrete;
        case 103: return FootStepType_GrittyCrete;
        case 104: return FootStepType_GrittyCrete;
        case 105: return FootStepType_GrittyCrete;
        case 106: return FootStepType_GrittyCrete;
        case 107: return FootStepType_GrittyCrete;
        case 108: return FootStepType_GrittyCrete;
        case 109: return FootStepType_GrittyCrete;
        case 110: return FootStepType_Squish;
        case 111: return FootStepType_GrittyCrete;
        case 112: return FootStepType_Metal;
        case 113: return FootStepType_Panel;
        case 114: return FootStepType_Panel;
        case 115: return FootStepType_Panel;
        case 116: return FootStepType_Metpanel;
        case 117: return FootStepType_Metpanel;
        case 118: return FootStepType_Panel;
        case 119: return FootStepType_Panel;
        case 120: return FootStepType_Metpanel;
        case 121: return FootStepType_Metpanel;
        case 122: return FootStepType_Glass;
        case 123: return FootStepType_Panel;
        case 124: return FootStepType_Rubber;
        case 125: return FootStepType_Rubber;
        case 126: return FootStepType_Glass;
        case 127: return FootStepType_Metal;
        case 128: return FootStepType_Glass;
        case 129: return FootStepType_Metal;
        case 130: return FootStepType_Grate;
        case 131: return FootStepType_Metal;
        case 132: return FootStepType_Metal;
        case 133: return FootStepType_Metal;
        case 134: return FootStepType_Metal;
        case 135: return FootStepType_Metpanel;
        case 136: return FootStepType_Metpanel;
        case 137: return FootStepType_Metal;
        case 138: return FootStepType_Metal;
        case 139: return FootStepType_Metpanel;
        case 140: return FootStepType_Metpanel;
        case 141: return FootStepType_Metal;
        case 142: return FootStepType_Metal;
        case 143: return FootStepType_Metal;
        case 144: return FootStepType_Vent;
        case 145: return FootStepType_Vent;
        case 146: return FootStepType_Vent;
        case 147: return FootStepType_Vent;
        case 148: return FootStepType_Vent;
        case 149: return FootStepType_Plastic;
        case 150: return FootStepType_Plastic;
        case 151: return FootStepType_Plastic;
        case 152: return FootStepType_Plastic;
        case 153: return FootStepType_Plastic;
        case 154: return FootStepType_Plastic;
        case 155: return FootStepType_Plastic;
        case 156: return FootStepType_Plastic;
        case 157: return FootStepType_Plastic;
        case 158: return FootStepType_Plastic;
        case 159: return FootStepType_Plastic;
        case 160: return FootStepType_Panel;
        case 161: return FootStepType_Panel;
        case 162: return FootStepType_Plastic2;
        case 163: return FootStepType_Plastic2;
        case 164: return FootStepType_Plastic2;
        case 165: return FootStepType_Plastic2;
        case 166: return FootStepType_Plastic2;
        case 167: return FootStepType_Plastic2;
        case 168: return FootStepType_Plastic2;
        case 169: return FootStepType_Panel;
        case 170: return FootStepType_Panel;
        case 171: return FootStepType_Panel;
        case 172: return FootStepType_Panel;
        case 173: return FootStepType_Panel;
        case 174: return FootStepType_Panel;
        case 175: return FootStepType_Panel;
        case 176: return FootStepType_Panel;
        case 177: return FootStepType_Panel;
        case 178: return FootStepType_Plastic;
        case 179: return FootStepType_Plastic;
        case 180: return FootStepType_Plastic;
        case 181: return FootStepType_Plastic;
        case 182: return FootStepType_Plastic;
        case 183: return FootStepType_Plastic;
        case 184: return FootStepType_Plastic;
        case 185: return FootStepType_Plastic;
        case 186: return FootStepType_Plastic;
        case 187: return FootStepType_Glass;
        case 188: return FootStepType_Plastic;
        case 189: return FootStepType_Metal;
        case 190: return FootStepType_Plastic;
        case 191: return FootStepType_Plastic;
        case 192: return FootStepType_Plastic;
        case 193: return FootStepType_Plastic;
        case 194: return FootStepType_Plastic;
        case 195: return FootStepType_Plastic;
        case 196: return FootStepType_Metal;
        case 197: return FootStepType_Metal2;
        case 198: return FootStepType_Metal2;
        case 199: return FootStepType_Metal;
        case 200: return FootStepType_Metal2;
        case 201: return FootStepType_Metal2;
        case 202: return FootStepType_Metal2;
        case 203: return FootStepType_Metal;
        case 204: return FootStepType_Metpanel;
        case 205: return FootStepType_Metpanel;
        case 206: return FootStepType_Metpanel;
        case 207: return FootStepType_Metpanel;
        case 208: return FootStepType_Metal;
        case 209: return FootStepType_Metal;
        case 210: return FootStepType_Metal;
        case 211: return FootStepType_Metal;
        case 212: return FootStepType_Metal;
        case 213: return FootStepType_Metal;
        case 214: return FootStepType_Metal;
        case 215: return FootStepType_Metal;
        case 216: return FootStepType_Metal;
        case 217: return FootStepType_Metal;
        case 218: return FootStepType_Metal;
        case 219: return FootStepType_Metal;
        case 220: return FootStepType_Metal;
        case 221: return FootStepType_Glass;
        case 222: return FootStepType_Metal;
        case 223: return FootStepType_Metal;
        case 224: return FootStepType_Metal;
        case 225: return FootStepType_Metal;
        case 226: return FootStepType_Metal;
        case 227: return FootStepType_Metal;
        case 228: return FootStepType_Metal;
        case 229: return FootStepType_Metal;
        case 230: return FootStepType_Metal;
        case 231: return FootStepType_Grate;
        case 232: return FootStepType_Plastic;
        case 233: return FootStepType_Plastic;
        case 234: return FootStepType_Metpanel;
        case 235: return FootStepType_Glass;
        case 236: return FootStepType_Glass;
        case 237: return FootStepType_Glass;
        case 238: return FootStepType_Metal;
        case 239: return FootStepType_Metal;
        case 240: return FootStepType_Metal;
        case 241: return FootStepType_Plastic;
        case 242: return FootStepType_Plastic;
        case 243: return FootStepType_Plastic;
        case 244: return FootStepType_Plastic;
        case 245: return FootStepType_Plastic;
        case 246: return FootStepType_Plastic;
        case 247: return FootStepType_Plastic;
        case 248: return FootStepType_Plastic;
        case 249: return FootStepType_Plastic;
        case 250: return FootStepType_Plastic;
        case 251: return FootStepType_Plastic;
        case 252: return FootStepType_Plastic;
        case 253: return FootStepType_Panel;
        case 254: return FootStepType_Panel;
        case 255: return FootStepType_Panel;
        case 256: return FootStepType_Plastic;
        case 257: return FootStepType_Plastic;
        case 258: return FootStepType_Plastic;
        case 259: return FootStepType_Plastic;
        case 260: return FootStepType_Glass;
        case 261: return FootStepType_Glass;
        case 262: return FootStepType_Grate;
        case 263: return FootStepType_Grate;
        case 264: return FootStepType_Grate;
        case 265: return FootStepType_Grate;
        case 266: return FootStepType_Plastic;
        case 267: return FootStepType_Plastic;
        case 268: return FootStepType_Plastic;
        case 269: return FootStepType_Plastic;
        case 270: return FootStepType_Glass;
        case 271: return FootStepType_Glass;
        case 272: return FootStepType_Plastic;
        case 273: return FootStepType_Plastic;
        case 274: return FootStepType_Plastic;
        case 275: return FootStepType_Plastic;
        case 276: return FootStepType_Plastic;
        case 277: return FootStepType_Plastic;
        case 278: return FootStepType_Plastic;
        case 279: return FootStepType_Glass;
        case 280: return FootStepType_Marble;
        case 281: return FootStepType_Marble;
        case 282: return FootStepType_Marble;
        case 283: return FootStepType_Marble;
        case 284: return FootStepType_Marble;
        case 285: return FootStepType_Marble;
        case 286: return FootStepType_Marble;
        case 287: return FootStepType_Marble;
        case 288: return FootStepType_Plastic;
        case 289: return FootStepType_Plastic;
        case 290: return FootStepType_Plastic;
        case 291: return FootStepType_Plastic;
        case 292: return FootStepType_Metal;
        case 293: return FootStepType_Metal;
        case 294: return FootStepType_Metal;
        case 295: return FootStepType_Metal;
        case 296: return FootStepType_Metal;
        case 297: return FootStepType_Metal;
        case 298: return FootStepType_Metal;
        case 299: return FootStepType_Metal;
        case 300: return FootStepType_Metal;
        case 301: return FootStepType_Metal;
        case 302: return FootStepType_Rubber;
        case 303: return FootStepType_Rubber;
        case 304: return FootStepType_Rubber;
        case 305: return FootStepType_Metal;
        case 306: return FootStepType_Plaster;

        // Props
        case 458: return FootStepType_Metpanel;
        case 459: return FootStepType_Metpanel;
        case 460: return FootStepType_Metpanel;
        case 461: return FootStepType_Metal;

        case 463: return FootStepType_Metal;
        case 464: return FootStepType_Wood2;

        case 472: return FootStepType_Wood2;
        case 473: return FootStepType_Wood2;
        case 474: return FootStepType_Wood2;
        case 475: return FootStepType_Wood2;
        case 476: return FootStepType_Wood2;
        case 477: return FootStepType_Metpanel;
        case 478: return FootStepType_Metpanel;
        case 479: return FootStepType_Metpanel;

        case 500: return FootStepType_Metal;

        case 515: return FootStepType_Panel;
        case 516: return FootStepType_Metal;

        case 525: return FootStepType_Metal;
        case 526: return FootStepType_Metal;
        case 527: return FootStepType_Grate;
        case 528: return FootStepType_Grate;
        case 529: return FootStepType_Grate;
        default: return FootStepType_Plastic;
    }
}

const char* JumpSound(FootStepType fstep) {
    switch(fstep) {
        case FootStepType_None: return sounds[0];
        // + 1 because its exclusive, :eyeroll:
        case FootStepType_Carpet:      return sounds[random_range_u32(540,542 + 1)];
        case FootStepType_Concrete:    return sounds[random_range_u32(546,548 + 1)];
        case FootStepType_GrittyCrete: return sounds[random_range_u32(552,554 + 1)];
        case FootStepType_Grass:       return sounds[random_range_u32(558,560 + 1)];
        case FootStepType_Gravel:      return sounds[random_range_u32(564,566 + 1)];
        case FootStepType_Rock:        return sounds[random_range_u32(570,572 + 1)];
        case FootStepType_Glass:       return sounds[random_range_u32(576,578 + 1)];
        case FootStepType_Marble:      return sounds[random_range_u32(582,584 + 1)];
        case FootStepType_Metal:       return sounds[random_range_u32(588,590 + 1)];
        case FootStepType_Grate:       return sounds[random_range_u32(594,596 + 1)];
        case FootStepType_Metal2:      return sounds[random_range_u32(600,602 + 1)];
        case FootStepType_Metpanel:    return sounds[random_range_u32(606,608 + 1)];
        case FootStepType_Panel:       return sounds[random_range_u32(612,614 + 1)];
        case FootStepType_Plaster:     return sounds[random_range_u32(618,620 + 1)];
        case FootStepType_Plastic:     return sounds[random_range_u32(624,626 + 1)];
        case FootStepType_Plastic2:    return sounds[random_range_u32(630,632 + 1)];
        case FootStepType_Rubber:      return sounds[random_range_u32(636,638 + 1)];
        case FootStepType_Sand:        return sounds[random_range_u32(642,644 + 1)];
        case FootStepType_Squish:      return sounds[random_range_u32(648,650 + 1)];
        case FootStepType_Vent:        return sounds[random_range_u32(429,430 + 1)];
        case FootStepType_Water:       return sounds[random_range_u32(651,654 + 1)];
        case FootStepType_Wood:        return sounds[random_range_u32(661,663 + 1)];
        case FootStepType_Wood2:       return sounds[random_range_u32(667,669 + 1)];
    }
    
    return sounds[0]; // null wav fallback
}
	
const char* JumpLandSound(FootStepType fstep) {
    switch(fstep) {
        case FootStepType_None: return sounds[0];
        // + 1 because its exclusive, :eyeroll:
        case FootStepType_Carpet:      return sounds[random_range_u32(537,539 + 1)];
        case FootStepType_Concrete:    return sounds[random_range_u32(543,545 + 1)];
        case FootStepType_GrittyCrete: return sounds[random_range_u32(549,551 + 1)];
        case FootStepType_Grass:       return sounds[random_range_u32(555,557 + 1)];
        case FootStepType_Gravel:      return sounds[random_range_u32(561,563 + 1)];
        case FootStepType_Rock:        return sounds[random_range_u32(567,569 + 1)];
        case FootStepType_Glass:       return sounds[random_range_u32(573,575 + 1)];
        case FootStepType_Marble:      return sounds[random_range_u32(579,581 + 1)];
        case FootStepType_Metal:       return sounds[random_range_u32(585,587 + 1)];
        case FootStepType_Grate:       return sounds[random_range_u32(591,593 + 1)];
        case FootStepType_Metal2:      return sounds[random_range_u32(597,599 + 1)];
        case FootStepType_Metpanel:    return sounds[random_range_u32(603,605 + 1)];
        case FootStepType_Panel:       return sounds[random_range_u32(609,611 + 1)];
        case FootStepType_Plaster:     return sounds[random_range_u32(615,617 + 1)];
        case FootStepType_Plastic:     return sounds[random_range_u32(621,623 + 1)];
        case FootStepType_Plastic2:    return sounds[random_range_u32(627,629 + 1)];
        case FootStepType_Rubber:      return sounds[random_range_u32(633,635 + 1)];
        case FootStepType_Sand:        return sounds[random_range_u32(639,641 + 1)];
        case FootStepType_Squish:      return sounds[random_range_u32(645,647 + 1)];
        case FootStepType_Vent:        return sounds[random_range_u32(428,437 + 1)];
        case FootStepType_Water:       return sounds[random_range_u32(655,657 + 1)];
        case FootStepType_Wood:        return sounds[random_range_u32(658,660 + 1)];
        case FootStepType_Wood2:       return sounds[random_range_u32(664,666 + 1)];
    }
    
    return sounds[0]; // null wav fallback
}

const char* FootStepSound(FootStepType fstep) {
    switch(fstep) {
        case FootStepType_None: return sounds[0];
        // + 1 because its exclusive, :eyeroll:
        case FootStepType_Carpet:      return sounds[random_range_u32(268,275 + 1)];
        case FootStepType_Concrete:    return sounds[random_range_u32(276,283 + 1)];
        case FootStepType_GrittyCrete: return sounds[random_range_u32(284,291 + 1)];
        case FootStepType_Grass:       return sounds[random_range_u32(292,299 + 1)];
        case FootStepType_Gravel:      return sounds[random_range_u32(300,307 + 1)];
        case FootStepType_Rock:        return sounds[random_range_u32(308,315 + 1)];
        case FootStepType_Glass:       return sounds[random_range_u32(316,323 + 1)];
        case FootStepType_Marble:      return sounds[random_range_u32(324,331 + 1)];
        case FootStepType_Metal:       return sounds[random_range_u32(332,339 + 1)];
        case FootStepType_Grate:       return sounds[random_range_u32(340,347 + 1)];
        case FootStepType_Metal2:      return sounds[random_range_u32(348,355 + 1)];
        case FootStepType_Metpanel:    return sounds[random_range_u32(356,363 + 1)];
        case FootStepType_Panel:       return sounds[random_range_u32(364,371 + 1)];
        case FootStepType_Plaster:     return sounds[random_range_u32(372,379 + 1)];
        case FootStepType_Plastic:     return sounds[random_range_u32(380,387 + 1)];
        case FootStepType_Plastic2:    return sounds[random_range_u32(388,395 + 1)];
        case FootStepType_Rubber:      return sounds[random_range_u32(396,403 + 1)];
        case FootStepType_Sand:        return sounds[random_range_u32(404,411 + 1)];
        case FootStepType_Squish:      return sounds[random_range_u32(412,427 + 1)];
        case FootStepType_Vent:        return sounds[random_range_u32(428,437 + 1)];
        case FootStepType_Water:       return sounds[random_range_u32(438,442 + 1)];
        case FootStepType_Wood:        return sounds[random_range_u32(443,450 + 1)];
        case FootStepType_Wood2:       return sounds[random_range_u32(451,458 + 1)];
    }

    return sounds[0]; // null wav
}
