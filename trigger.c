// trigger.c - Trigger volumes from invisible oriented box colliders for things like gravity lifts, logic actions, ladders
void AddForce(u16 i, V3 f, bool imp); void AddAccessCardToInventory(int index); void UseTargets(u16 activator, const char* targetname); void DeleteInstance(u16 i); void TakeEnergy(float take);
void trigger_cyberpush_touch(u16 self, u16 other) { if (World.diffCyb < 1) {return;} AddForce(other,V3_ScaleByF(World.instances[self].direction,World.instances[self].force * (float)World.deltaTime),false); World.Sys_Music.cyberTube = true; }
void prop_cyber_exit(u16 other) { if (other != PLAYER1) {return;} UIExitCyberspace(); }
void CyberDataFragmentOnTriggerEnter(u16 self, u16 other) { Entity* e = &World.instances[self]; if (other != PLAYER1) {return;} UICyberSprint((u16)e->textIndex); }
void CyberItemInitBeforeLoad(u16 self) { Entity* e = &World.instances[self]; if (World.diffMis == 0 && e->index == 448) {flag_set(&e->entflags,EF_ACTIVE,false); /*item_cyber_data*/} }
bool AddSoftwareItem(u16 index, int vers) {
    Entity* player = &World.instances[PLAYER1];
    float sfxVol = (float)Sys_Settings.VolumeEffects / 100.0f;
    switch(index) {
        case 450/*item_cyber_drill*/:
            if (World.invP1.isPulserNotDrill && !(World.invP1.hasSoft & (1u << SW_PULSER))) World.invP1.isPulserNotDrill = false;
            if (vers > World.invP1.softVersions[SW_DRILL]) World.invP1.softVersions[SW_DRILL] = (u8)vers;
            else CenterStatusPrint("%s",Sys_Text.stringTable[46]);
            World.invP1.hasSoft |= (1u << SW_DRILL); play_wav(sounds[86],sfxVol,(V3){0.0f,0.0f,0.0f},false); CenterStatusPrint("%s%d%s",Sys_Text.stringTable[444],World.invP1.softVersions[SW_DRILL],Sys_Text.stringTable[458]); return true;
        case 454/*item_cyber_pulser*/:
            if (!World.invP1.isPulserNotDrill && !(World.invP1.hasSoft & (1u << SW_PULSER))) World.invP1.isPulserNotDrill = true;
            if (vers > World.invP1.softVersions[SW_PULSER]) World.invP1.softVersions[SW_PULSER] = (u8)vers;
            else CenterStatusPrint("%s",Sys_Text.stringTable[46]);
            World.invP1.hasSoft |= (1u << SW_PULSER); play_wav(sounds[86],sfxVol,(V3){0.0f,0.0f,0.0f},false); CenterStatusPrint("%s%d%s",Sys_Text.stringTable[445],World.invP1.softVersions[SW_PULSER],Sys_Text.stringTable[458]); return true;
        case 456/*item_cyber_shield*/:
            if (vers > World.invP1.softVersions[SW_SHIELD]) World.invP1.softVersions[SW_SHIELD] = (u8)vers;
            else CenterStatusPrint("%s",Sys_Text.stringTable[46]);
            World.invP1.hasSoft |= (1u << SW_SHIELD); play_wav(sounds[86],sfxVol,(V3){0.0f,0.0f,0.0f},false); CenterStatusPrint("%s%d%s",Sys_Text.stringTable[446],World.invP1.softVersions[SW_SHIELD],Sys_Text.stringTable[458]); return true;
        case 457/*item_cyber_turbo*/:
            if (World.invP1.cyberItemIndex < 0) World.invP1.cyberItemIndex = 0;
            World.invP1.softVersions[SW_TURBO]++; World.invP1.hasSoft |= (1u << SW_TURBO); play_wav(sounds[86],sfxVol,(V3){0.0f,0.0f,0.0f},false); CenterStatusPrint("%s",Sys_Text.stringTable[447]); return true;
        case 449/*item_cyber_decoy*/:
            if (World.invP1.cyberItemIndex < 0) World.invP1.cyberItemIndex = 1;
            World.invP1.softVersions[SW_DECOY]++; World.invP1.hasSoft |= (1u << SW_DECOY); play_wav(sounds[86],sfxVol,(V3){0.0f,0.0f,0.0f},false); CenterStatusPrint("%s",Sys_Text.stringTable[448]); return true;
        case 455/*item_cyber_recall*/: if (World.invP1.cyberItemIndex < 0){World.invP1.cyberItemIndex = 2;} World.invP1.softVersions[SW_RECALL]++; World.invP1.hasSoft |= (1u << SW_RECALL); play_wav(sounds[86],sfxVol,(V3){0.0f,0.0f,0.0f},false); CenterStatusPrint("%s",Sys_Text.stringTable[449]); return true;
        case 451/* ;) item_cyber_game*/: { if (vers < 0 || vers >= 7){return false;} World.invP1.hasNewData  = true; World.invP1.hasMinigame |= (u8)(1u << vers); static const u16 gameMsg[7] = {450,451,452,453,454,455,456}; play_wav(sounds[86],sfxVol,(V3){0.0f,0.0f,0.0f},false); CenterStatusPrint("%s",Sys_Text.stringTable[gameMsg[vers]]); return true; }
        case 448/*item_cyber_data*/: World.invP1.hasNewData = true; if (vers >= 0 && vers < T_LOGS_COUNT) {World.invP1.hasLog[vers] = true;} play_wav(sounds[87],sfxVol,(V3){0.0f,0.0f,0.0f},false); CenterStatusPrint("%s",Sys_Text.stringTable[457]); return true; 
        case 452/*item_cyber_integrity*/: if (player->cyberHealth >= 255.0f) {return false;} play_wav(sounds[86],sfxVol,(V3){0.0f,0.0f,0.0f},false); player->cyberHealth += 77.0f; if (player->cyberHealth > 255.0f) {player->cyberHealth = 255.0f;} CenterStatusPrint("%s",Sys_Text.stringTable[459]); return true;
        case 453/*item_cyber_keycard*/: World.invP1.hasNewData = true; if (vers < 0 || vers > 110) vers = 81; AddAccessCardToInventory(vers); return true;
        default: break;
    }
    return false;
}

void CyberItemOnTriggerEnter(u16 self, u16 other) { Entity* e = &World.instances[self]; if (other != PLAYER1) {return;} if (!AddSoftwareItem(e->index,e->version)) {return;} flag_set(&e->entflags,EF_ACTIVE,false); }
void CyberIceOnTriggerEnter(u16 self, u16 other) { (void)self; Entity* e = &World.instances[other]; if (!(e->entflags & EF_RIGIDBODY)) return; World.layer[other] = 24; World.velocity[other] = V3_ScaleByF(World.velocity[other],-1.0f); }
void CyberMineInitBeforeLoad(u16 self) {
    Entity* e = &World.instances[self];
    e->damage = 55.0f;
    if (World.diffCyb < 3) { if (random_range(0.0f,1.0f) < 0.2f) flag_set(&e->entflags,EF_ACTIVE,false); e->damage = 33.0f; }
    if (World.diffCyb < 2) { if (random_range(0.0f,1.0f) < 0.33f) flag_set(&e->entflags,EF_ACTIVE,false); e->damage = 22.0f; }
    if (World.diffCyb < 1) { if (random_range(0.0f,1.0f) < 0.50f) flag_set(&e->entflags,EF_ACTIVE,false); e->damage = 11.0f; }
}

float TakeDamage(u16 self,DamageData dd);
void CyberMineOnTriggerEnter(u16 self, u16 other) { Entity* e = &World.instances[self]; if (other != PLAYER1) return; PlayerTakeDamage(PLAYER1,e->damage); play_wav(sounds[67],1.0f,World.position[self],false); flag_set(&e->entflags,EF_ACTIVE,false); }
void CyberSwitchInitAfterLoad(u16 self) { Entity* e = &World.instances[self]; if (e->iceActive) {flag_set(&e->entflags,EF_ACTIVE,true);} } // TODO Visual subobject parity removed with hierarchy removal.
void CyberSwitchOnTriggerEnter(u16 self, u16 other) { Entity* e = &World.instances[self]; if (e->active || other != PLAYER1) {return;} UICyberSprint((u16)e->textIndex); e->active = true; UseTargets(other,e->target); }
// TeleportTouch
static u16 TeleportTouch_allTeleportTouches[8];
static bool TeleportTouch_initialized;
void TeleportTouchInitAfterLoad(u16 self) {
    Entity* e = &World.instances[self];
    if (!TeleportTouch_initialized) { for (u8 i = 0; i < 8; i++) TeleportTouch_allTeleportTouches[i] = U16_MAX; TeleportTouch_initialized = true; }
    if (e->teleportID >= 8) { DeleteInstance(self); return; }
    TeleportTouch_allTeleportTouches[e->teleportID] = self;
}

void TeleportTouchOnTriggerEnter(u16 self, u16 other) {
    Entity* e = &World.instances[self];
    Entity* player = &World.instances[PLAYER1];
    if (!e->touchEnabled || other != PLAYER1) return;
    if (player->health <= 0.0f || e->justUsed >= World.pauseRelativeTime) return;
    u16 dest = e->targetDestinationID < 8 ? TeleportTouch_allTeleportTouches[e->targetDestinationID] : U16_MAX;
    if (dest == U16_MAX) return;
    World.position[PLAYER1] = World.position[dest];
    World.instances[dest].justUsed = World.pauseRelativeTime + 1.0;
    play_wav(sounds[106],1.0f,World.position[dest],false);
}
// Trigger for Events (trigger_multiple/trigger_once same as Quake 1)
void TriggerDelayedTarget(u16 self, u16 activator) { World.instances[self].delayFireFinished = World.pauseRelativeTime + World.instances[self].delay; UseTargets(activator,World.instances[self].target); }
void TriggerTriggerTripped(u16 self, u16 other) { Entity* e=&World.instances[self]; if(other != PLAYER1 || (e->recentMostActivator && e->ignoreSecondaryTriggers)) return; e->recentMostActivator=other; if(e->onlyOnce){e->allDone=true;} if(e->delay <= 0.0f){UseTargets(other,World.instances[self].target);}else{TriggerDelayedTarget(self,other);} }
void TriggerOnTriggerEnter(u16 self, u16 other) { if (!World.instances[self].allDone) TriggerTriggerTripped(self,other); }
void TriggerOnTriggerStay(u16 self, u16 other) { if (!World.instances[self].allDone) TriggerTriggerTripped(self,other); }
// GravityLift
void GravityLiftInitAfterLoad(u16 self) {
    World.instances[self].strength =                  UsableOrDef(World.instances[self].strength, 12.0f);
    World.instances[self].offStrengthFactor =         UsableOrDef(World.instances[self].offStrengthFactor, 0.3f);
    World.instances[self].distancePaddingToTopPoint = UsableOrDef(World.instances[self].distancePaddingToTopPoint, 0.32f);
    World.instances[self].topPoint = (V3){ 0.0f, World.position[self].y + (World.colliderSize[self].y * 0.5f), 0.0f };
}

void GravityLiftOnForce(u16 self, u16 other, bool initial) {
    float topY = World.position[self].y + (World.colliderSize[self].y * 0.5f);
    float dist = topY - World.position[other].y + 0.48f;
    float velY = World.velocity[other].y < 0.0f ? 0.0f : World.velocity[other].y;
    if (dist < World.instances[self].distancePaddingToTopPoint) AddForce(other,(V3){0.0f,9.81f - velY,0.0f},false); // TODO accel-vs-force parity
    else if (World.velocity[other].y < (World.instances[self].strength * World.mass[other])) {
        float yForce = (World.instances[self].strength * World.mass[other]) - World.velocity[other].y;
        if (initial || World.instances[self].initialBurstFinished > World.pauseRelativeTime) yForce *= 2.0f;
        AddForce(other,(V3){0.0f,yForce,0.0f},false);
    }
}

void GravityLiftOffForce(u16 self, u16 other, bool initial) {
    if (World.velocity[other].y < World.instances[self].offStrengthFactor) {
        float yForce = World.instances[self].offStrengthFactor - World.velocity[other].y;
        if (initial || World.instances[self].initialBurstFinished > World.pauseRelativeTime) yForce *= 2.0f;
        AddForce(other,(V3){0.0f,yForce,0.0f},false);
    }
}

void trigger_gravitylift_touch(u16 self, u16 other) {
    if (vabs(World.gravity[other] - 1.0f) < 0.00001f) World.instances[self].initialBurstFinished = World.pauseRelativeTime + 1.0f;
    if (World.instances[self].active) GravityLiftOnForce(self,other,true);
    else GravityLiftOffForce(self,other,true);
}

void GravityLiftToggle(u16 self) { World.instances[self].active = !World.instances[self].active; }
