// g_music.c - Music System
#include "mod.h"
#define BUFFER_MS 50
#define AUD_BUFFER_T 0.05f
MusicSystem Sys_Music;
const char* levelMusicLooped[14] = {
    "looped/track0.mp3", "looped/track1.mp3", "looped/track2.mp3", "looped/track3.mp3",
    "looped/track4.mp3", "looped/track5.mp3", "looped/track6.mp3", "looped/track7.mp3",
    "looped/track8.mp3", "looped/track9.mp3", "looped/track10.mp3", "looped/track11.mp3",
    "looped/track12.mp3", "looped/track13.mp3"
};

const char* reactorMusic[13] = {
    "THM4-01_reactorcombat1.mp3", "THM4-02_reactorcombat2.mp3", "THM4-03_reactorcombat3.mp3",
    "THM4-04_reactorcombat4.mp3", "THM4-05_reactorwalkingatocombat.mp3", "THM4-06_reactorwalkingbtocombat.mp3",
    "THM4-09_reactorwalkinga1.mp3", "THM4-10_reactorwalkinga2.mp3", "THM4-11_reactorwalkingb1.mp3",
    "THM4-12_reactorwalkingb2.mp3", "THM4-13_reactorwalkingb3.mp3", "THM4-14_reactorwalkingc1.mp3",
    "THM4-15_reactorwalkingc2.mp3"
};

const char* medicalMusic[11] = {
    "THM1-19_medicalstart.mp3", "THM1-01_medicalwalking1.mp3", "THM1-02_medicalwalking2.mp3",
    "THM1-03_medicalwalking3.mp3", "THM1-04_medicalwalking4.mp3", "THM1-05_medicalcombat1.mp3",
    "THM1-06_medicalcombat2.mp3", "THM1-07_medicalcombat3.mp3", "THM1-08_medicalcombat4.mp3",
    "THM1-09_medicalcombat5.mp3", "THM1-10_medicalcombat6.mp3"
};

const char* scienceMusic[10] = {
    "THM3-17_sciencestart.mp3", "THM3-03_science1.mp3", "THM3-04_science2.mp3",
    "THM3-05_science3.mp3", "THM3-06_science4.mp3", "THM3-07_science5.mp3",
    "THM3-08_science6.mp3", "THM3-09_science7.mp3", "THM3-01_scienceaction1.mp3",
    "THM3-02_scienceaction2.mp3"
};

const char* executiveMusic[13] = {
    "THM2-11_executive1.mp3", "THM2-12_executive2.mp3", "THM2-13_executive3.mp3",
    "THM2-08_executive4.mp3", "THM2-09_executive5.mp3", "THM2-10_executive6.mp3",
    "THM2-04_executive2.mp3", "THM2-05_executive3.mp3", "THM2-06_executivefluterlude.mp3",
    "THM2-07_executivefluterludewithguitar.mp3", "THM2-01_executiveaction3.mp3",
    "THM2-02_executiveaction4.mp3", "THM2-03_executiveaction5.mp3"
};

const char* groveMusic[24] = {
    "THM5-07_groveaction1.mp3", "THM5-08_groveaction1.mp3", "THM5-09_groveaction2.mp3",
    "THM5-10_groveaction3.mp3", "THM5-11_groveaction4.mp3", "THM5-12_groveaction5.mp3",
    "THM5-13_groveaction6.mp3", "THM5-14_groveaction7.mp3", "THM5-15_groveaction8.mp3",
    "THM5-33_grove1.mp3", "THM5-34_grove2.mp3", "THM5-38_grove3.mp3", "THM5-39_grove4.mp3",
    "THM5-40_grove5.mp3", "THM5-35_grove99.mp3", "THM5-36_grove100.mp3", "THM5-37_grove101.mp3",
    "THM5-41_grove102.mp3", "THM5-42_grove103.mp3", "THM5-01_grove105.mp3", "THM5-02_grove106.mp3",
    "THM5-03_grove107.mp3", "THM5-04_grove108.mp3", "THM5-05_grove109.mp3"
};

const char* securityMusic[19] = {
    "THM6-05_securityaction1.mp3", "THM6-06_securityaction2.mp3", "THM6-07_securityaction3.mp3",
    "THM6-08_securityaction4.mp3", "THM6-09_securityaction5.mp3", "THM6-10_securityaction6.mp3",
    "THM6-01_security1.mp3", "THM6-02_security2.mp3", "THM6-03_security3.mp3",
    "THM6-04_security4.mp3", "THM6-11_security100.mp3", "THM6-12_security101.mp3",
    "THM6-13_security1.mp3", "THM6-14_security2.mp3", "THM6-15_security3.mp3",
    "THM6-17_security4.mp3", "THM6-18_security5.mp3", "THM6-19_security6.mp3",
    "THM6-20_security7.mp3"
};

const char* cyberMusic[13] = {
    "THM10-02_cyberstart.mp3", "THM10-01_cyber1.mp3", "THM10-03_cyber2.mp3",
    "THM10-04_cyber3.mp3", "THM10-05_cyber4.mp3", "THM10-06_cyber5.mp3",
    "THM10-07_cyber6.mp3", "THM10-08_cyber7.mp3", "THM10-09_cyber8.mp3"
};

const char* levelMusicElevator[13] = {
    "THM7-01_elevator1.mp3", "THM7-01_elevator1.mp3", "THM7-02_elevator2.mp3",
    "THM7-03_elevator3.mp3", "THM7-04_elevator4.mp3", "THM7-05_elevator5.mp3",
    "THM7-06_elevator6.mp3", "THM7-07_elevator7.mp3", "THM7-08_elevator8.mp3",
    "THM7-01_elevator1.mp3", "THM7-01_elevator1.mp3", "THM7-01_elevator1.mp3",
    "THM7-01_elevator1.mp3"
};

const char* levelMusicRevive[14] = {
    "THM4-18_reactorrevive.mp3", "THM1-18_medicalrevive.mp3", "THM3-19_sciencerevive.mp3",
    "THM3-19_sciencerevive.mp3", "THM3-19_sciencerevive.mp3", "THM1-18_medicalrevive.mp3",
    "THM2-18_executiverevive.mp3", "THM4-18_reactorrevive.mp3", "THM6-22_securityrevive.mp3",
    "THM1-18_medicalrevive.mp3", "THM2-18_executiverevive.mp3", "THM2-18_executiverevive.mp3",
    "THM2-18_executiverevive.mp3", "THM1-18_medicalrevive.mp3"
};

const char* levelMusicDistortion[14] = {
    "THM6-49_securitydistorted.mp3", "THM1-48_medicaldistorted.mp3", "THM3-49_sciencedistorted.mp3",
    "THM3-49_sciencedistorted.mp3", "THM1-48_medicaldistorted.mp3", "THM1-48_medicaldistorted.mp3",
    "THM2-46_executivedistorted.mp3", "THM1-48_medicaldistorted.mp3", "THM6-49_securitydistorted.mp3",
    "THM1-48_medicaldistorted.mp3", "THM1-48_medicaldistorted.mp3", "THM1-48_medicaldistorted.mp3",
    "THM1-48_medicaldistorted.mp3", "THM10-41_cyberdistorted.mp3"
};

const char* levelMusicDeath[14] = {
    "THM0-17_death.mp3", "THM1-17_death.mp3", "THM3-18_death.mp3", "THM0-17_death.mp3",
    "THM3-18_death.mp3", "THM0-17_death.mp3", "THM2-17_death.mp3", "THM0-17_death.mp3",
    "THM6-21_death.mp3", "THM0-17_death.mp3", "THM5-17_death.mp3", "THM5-17_death.mp3",
    "THM5-17_death.mp3", "THM10-16_death.mp3"
};

MOD_TO_ENGINE void PlayMenuMusic(void) { mp3_clear(); play_mp3("./Audio/music/TITLOOP-00_menu.mp3",1500); }
MOD_TO_ENGINE void PlayGameMusic(void) { mp3_clear(); /*play_mp3("./Audio/music/THM1-19_medicalstart.mp3",100);*/ }

const char* GetCorrespondingLevelClip(TrackType ttype) {
    switch(ttype) { // Override types, return from these first before special level handling
        case TrackType_Revive:     return levelMusicRevive[Eng_Global->currentLevel];
        case TrackType_Death:      return levelMusicDeath[Eng_Global->currentLevel];
        case TrackType_Elevator:   return levelMusicElevator[Eng_Global->currentLevel];
        case TrackType_Distortion: return levelMusicDistortion[Eng_Global->currentLevel];
    }

    if (Eng_Global->currentLevel == 0 || Eng_Global->currentLevel == 5 || Eng_Global->currentLevel == 7) { // 0  REACTOR, 5 FLIGHT, 7 ENGINEERING
        if (Sys_Music.levelEntry)      return reactorMusic[6];
        if (ttype == TrackType_Combat) return reactorMusic[random_range_u8(0,6)];
        return reactorMusic[random_range_u8(6,13)];
    } else if (Eng_Global->currentLevel == 1) { // 1  MEDICAL
        if (Sys_Music.levelEntry) return medicalMusic[0];
        if (ttype == TrackType_Combat) return medicalMusic[random_range_u8(5,11)];
        return medicalMusic[random_range_u8(1,5)];
    } else if (Eng_Global->currentLevel == 2 || Eng_Global->currentLevel == 4) { // 2  SCIENCE, 4 STORAGE
        if (Sys_Music.levelEntry)      return scienceMusic[0];
        if (ttype == TrackType_Combat) return scienceMusic[random_range_u8(8,10)];
        return scienceMusic[random_range_u8(1,8)];
    } else if (Eng_Global->currentLevel == 8) { // 8 SECURITY
        if (Sys_Music.levelEntry)      return securityMusic[9];
        if (ttype == TrackType_Combat) return securityMusic[random_range_u8(0,6)];
        return securityMusic[random_range_u8(6,19)];
    } else if (Eng_Global->currentLevel == 6) { // 6 EXECUTIVE
        if (Sys_Music.levelEntry)      return executiveMusic[0];
        if (ttype == TrackType_Combat) return executiveMusic[random_range_u8(9,13)];
        return executiveMusic[random_range_u8(0,10)];
    } else if (Eng_Global->currentLevel == 10 || Eng_Global->currentLevel == 11 || Eng_Global->currentLevel == 12) { // 10, 12 GROVES
        if (Sys_Music.levelEntry)      return groveMusic[19];
        if (ttype == TrackType_Combat) return groveMusic[random_range_u8(0,9)];
        return executiveMusic[random_range_u8(9,24)];
    } else if (Eng_Global->currentLevel == 13) { // 13 CYBERSPACE
        if (Sys_Music.levelEntry)           return cyberMusic[0];
        if (Sys_Music.cyberTube)            return cyberMusic[random_range_u8(4,8)];
        if (random_range(0.0f,1.0f) < 0.5f) return cyberMusic[random_range_u8(1,5)];
        else                                return cyberMusic[8];
    }

    return levelMusicLooped[0];
}

void PlayTrack(TrackType ttype, MusicType mtype) {
    char mp[64];
    if (!Eng_Settings->DynamicMusic) { // Looped Music (Dynamic Music off)
        u8 lev = Eng_Global->currentLevel;
        if (mtype == MusicType_Override) {
                 if (ttype == TrackType_Revive) { StringFormat(mp,sizeof(mp),"./Audio/music/%s",levelMusicRevive[lev]); play_mp3(mp,0); }
            else if (ttype == TrackType_Death) { StringFormat(mp,sizeof(mp),"./Audio/music/%s",levelMusicDeath[lev]); play_mp3(mp,0); } 
            else if (ttype == TrackType_Elevator) { StringFormat(mp,sizeof(mp),"./Audio/music/%s",levelMusicElevator[lev]); play_mp3(mp,0); }
            else if (ttype == TrackType_Distortion) { StringFormat(mp,sizeof(mp),"./Audio/music/%s",levelMusicDistortion[lev]); play_mp3(mp,0); }
        } else { StringFormat(mp,sizeof(mp),"./Audio/music/%s",levelMusicLooped[lev]); play_mp3(mp,0); }   play_mp3(levelMusicLooped[lev],0);
        return;
    }
    

    // Normal Dynamic Music System
    if (mtype == MusicType_Override) mp3_clear();
    StringFormat(mp,sizeof(mp),"./Audio/music/%s",GetCorrespondingLevelClip(ttype));
    play_mp3(mp,BUFFER_MS);
    if (!Sys_Music.elevator) Sys_Music.levelEntry = false; // already used by GetCorresponding... just now
}

void MusicNotifyZone(TrackType tt) {
    Sys_Music.inZone = true;
    switch(tt) {
        case TrackType_Elevator: Sys_Music.elevator = true; break;
        case TrackType_Distortion: Sys_Music.distortion = true; break;
    }
}

void MusicTriggerEnter(u16 self, u16 other) {
    if (Eng_Global->instances[self].tickFinished < Eng_Global->pauseRelativeTime) { // Prevent flickering retrigger when player slides along glancing angle of trigger volume.
        if (other == PLAYER1 || other == PLAYER2) {
            PlayTrack(Eng_Global->instances[self].trackType,Eng_Global->instances[self].musicType);
            MusicNotifyZone(Eng_Global->instances[self].trackType);
        }
        
        Eng_Global->instances[self].tickFinished = Eng_Global->pauseRelativeTime + 0.1;
    }
}

void MusicTriggerExit(u16 other) {
    if (other == PLAYER1 || other == PLAYER2) { mp3_clear(); Sys_Music.inZone = Sys_Music.elevator = Sys_Music.distortion = false; } // return to normal upon leaving the trigger
}

MOD_TO_ENGINE void UpdateMusic(void) {
    if (Eng_Global->gamePaused && !Eng_Global->menuActive) { MP3Pause(); return; }
    MP3Resume();
    float remaining = GetMP3RemainingTime(); if (remaining > AUD_BUFFER_T) return;

    if (Eng_Global->menuActive) { play_mp3("./Audio/music/TITLOOP-00_menu.mp3",1500); return; }
    if (Sys_Music.inCombat && !Sys_Music.inZone && Sys_Music.combatImpulseFinished < Eng_Global->pauseRelativeTime) {
        DualLog("Combat music!\n");
        Sys_Music.inCombat = false;
        PlayTrack(TrackType_Combat, MusicType_Override);
        Sys_Music.combatImpulseFinished = Eng_Global->pauseRelativeTime + 20.0;
        return;
    }

    if (Sys_Music.inZone) {
        DualLog("Entered a music zone!\n");
        if (Sys_Music.distortion) { PlayTrack(TrackType_Distortion, MusicType_Override); return; }
        if (Sys_Music.elevator) { PlayTrack(TrackType_Elevator, MusicType_Override); return; }
    }
    
    if (Eng_Settings->DynamicMusic || remaining <= AUD_BUFFER_T) { DualLog("Dynamic normal track selection\n"); PlayTrack(TrackType_Walking, MusicType_Walking); }
}

MOD_TO_ENGINE void ResetLevelMusic(void) {
    mp3_clear(); Sys_Music.levelEntry = true; Sys_Music.inZone = Sys_Music.cyberTube = false;
    Sys_Music.clipFinished = Sys_Music.combatImpulseFinished = get_time(); Sys_Music.combatImpulseFinished += 5.0;
}
