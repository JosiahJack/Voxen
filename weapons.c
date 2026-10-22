// weapons.c - Weapon System
#include "common.h"
FootStepType GetFootstepTypeForPrefab(int pid); bool ChangeAmmoType();
void ApplyImpactForce(u16 target, float vel, V3 normal, V3 pt);
void ApplyImpactForceWithSound(u16 target, float vel, V3 normal, V3 pt);
float delayBetweenShotsForWeapon[16]={1.0f,0.6f,0.5f,0.1f,0.8f,1.6f,0.65f,0.8f,0.6f,0.5f,1.2f,0.9f,0.5f,0.08f,1.1f,0.75f}; float delayBetweenShotsForWeapon2[16]={1.0f,4.5f,0.5f,0.15f,4.0f,1.6f,0.75f,0.8f,0.6f,0.5f,1.2f,0.9f,0.5f,0.08f,5.0f,0.75f};
float dmgForWep[16]={75.0f,12.0f,15.0f,10.0f,18.0f,150.0f,15.0f,60.0f,45.0f,22.0f,50.0f,185.0f,6.0f,35.0f,6.0f,2.0f}; float dmgForWep2[16]={160.0f,70.0f,5.0f,22.0f,108.0f,0.0f,0.0f,85.0f,80.0f,33.0f,350.0f,0.0f,0.0f,35.0f,36.0f,15.0f};
float damageOverloadForWeapon[16]={0.0f,115.0f,0.0f,0.0f,180.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,60.0f,0.0f};
float energyDrainLowForWeapon[16]={0.0f,3.0f,0.0f,0.0f,5.0f,0.0f,0.0f,0.0f,0.0f,0.0f,13.0f,0.0f,0.0f,0.0f,2.0f,3.0f}; float energyDrainHiForWeapon[16]={0.0f,15.0f,0.0f,0.0f,30.0f,0.0f,0.0f,0.0f,0.0f,0.0f,130.0f,0.0f,0.0f,0.0f,8.0f,30.0f};
float energyDrainOverloadForWeapon[16]={0.0f,50.0f,0.0f,0.0f,100.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,24.0f,0.0f};
float penetrationWep[16]={50.0f,25.0f,6.0f,35.0f,35.0f,80.0f,40.0f,30.0f,100.0f,20.0f,0.0f,35.0f,0.0f,40.0f,25.0f,0.0f}; float penetrationWep2[16]={70.0f,0.0f,0.0f,32.0f,0.0f,0.0f,0.0f,25.0f,120.0f,30.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
float offenseWep[16]={4.0f,4.0f,2.0f,2.0f,6.0f,5.0f,3.0f,4.0f,4.0f,2.0f,3.0f,6.0f,2.0f,2.0f,3.0f,3.0f}; float offenseWep2[16]={5.0f,0.0f,3.0f,3.0f,0.0f,0.0f,0.0f,5.0f,5.0f,3.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
u8 magazinePitchCountForWeapon[16]={10,0,15,60,0,0,0,12,25,20,0,12,20,50,0,0}; u8 magazinePitchCountForWeapon2[16]={8,0,15,60,0,0,0,12,10,20,0,0,0,100,0,0};
float reloadTime[16]={1.0f,0.8f,1.0f,1.2f,0.8f,0.8f,0.8f,1.3f,1.5f,0.8f,0.8f,1.0f,1.5f,2.0f,0.8f,0.8f};
float recoilForWeapon[16]={1.3f,0.0f,0.1f,0.2f,0.0f,0.0f,0.0f,1.2f,0.8f,0.5f,1.5f,1.0f,0.9f,0.7f,0.0f,0.1f};
float driftForWeapon[16]={5.0f,0.0f,15.0f,50.0f,0.0f,0.0f,0.0f,8.0f,3.0f,3.0f,3.0f,12.0f,10.0f,30.0f,0.0f,3.0f};
AttType attTypeWep[16]={Att_HitS,Att_Beam,Att_PjNd,Att_HitS,Att_Beam,Att_MlEg,Att_HitS,Att_HitS,Att_Magn,Att_HitS,Att_PjBm,Att_Ball,Att_HitS,Att_HitS,Att_Beam,Att_Trnq};
u16 wepFireSound[16]={251,239,240,243,245,246,254,249,250,255,257,259,262,263,264,265}; //(0)Assault (1)Blaster (2)Dartgun (3)Flechette (4)IonBeam (5)Rapier (6)Pipe (7)Magnum (8)Magpulse (9)Pistol (10)Plasma (11)Railgun (12)Riotgun (13)Skorpion (14)SparqBeam (15)Stungun
u16 wepBulletHolePrefab[16]={518,520,522,521,519,520,522,518,519,521,519,519,523,518,520,520};
i8  wepFogInc[16]={2,0,0,1,0,0,0,3,0,1,0,2,4,2,0,0};
u16 wepSmokePrefab[16]={506,0,0,509,0,0,0,507,0,510,0,511,512,513,0,0}; // 0 = no smoke for this weapon
u16 wepModelIndices[16]={646,638,640,642,643,5748,5728,644,645,650,651,652,654,655,656,657}; // [wep16] mk3,blaster,dart,flechette,ion,rapier,pipe,magnum,magpulse,pistol,plasma,rail,riot,skorp,sparq,stun
u16 wepAnimNums[16]={MAXANIM,MAXANIM,MAXANIM,MAXANIM,MAXANIM,50,49,MAXANIM,MAXANIM,MAXANIM,MAXANIM,MAXANIM,MAXANIM,MAXANIM,MAXANIM,MAXANIM}; // [wep16] only rapier(50)/pipe(49)
typedef enum { WC_STD = 0, WC_MELEE = 1, WC_ENERGY = 2 } WepClass;
WepClass wepClass[16] = {WC_STD, WC_ENERGY, WC_STD, WC_STD, WC_ENERGY, WC_MELEE, WC_MELEE,WC_STD, WC_STD, WC_STD, WC_ENERGY, WC_STD, WC_STD, WC_STD, WC_ENERGY, WC_ENERGY};
float magpulseShotForce = 2.2f, stungunShotForce = 2.2f, railgunShotForce = 5.0f, plasmaShotForce = 1.5f;
WeaponFireCtx wfx = { .fireDistance = 200.0f, .overheatedPercent = 80.0f, .reloadContainerHome = {0}, .pendingMeleeFinished = 0.0 };
INLINE Quaternion QuatEulerZ(float degZ) { float r = deg2rad(degZ) * 0.5f; return (Quaternion){0.0f,0.0f,vsinf(r),vcosf(r)}; }
INLINE bool WeaponsHaveAnyHeat() { if (Cheats.redbull) {return false;} for (int i=0;i<7;i++) {if (World.invP1.curWepHeat[i] > 0.0f) {return true;}} return false; }
static bool hudHeatTickOn[9];
void HudHeatBleed(float heat) { float tempFloat=10.0f; for (int i=0;i<9;i++) { hudHeatTickOn[i]=heat >= tempFloat; tempFloat += 10.0f; } }
bool HudHeatTickOn(int i) { return (i>=0&&i<9) ? hudHeatTickOn[i] : false; }
void HeatBleedOff() {
    static double heatTickFinished = 0.0;
    static const float heatTickTime = 0.50f;
    if (heatTickFinished >= World.pauseRelativeTime) return;
    World.fogFac--;
    if (World.fogFac < 0) World.fogFac = 0;
    if (WeaponsHaveAnyHeat() || CurrentWeaponUsesEnergy()) {
        for (int i=0;i<7;i++) {
            World.invP1.curWepHeat[i] -= 10.0f;
            if (World.invP1.curWepHeat[i] <= 0.0f) World.invP1.curWepHeat[i] = 0.0f;
        }
        if (CurrentWeaponUsesEnergy()) HudHeatBleed(World.invP1.curWepHeat[World.invP1.wepCur]);
    }
    heatTickFinished = World.pauseRelativeTime + heatTickTime;
}

static void Recoil(int wep16) {
    float s=recoilForWeapon[wep16]; if(s<=0.0f){return;} if(World.instances[PLAYER1].health>0.0f && World.invP1.fatigue>80.0f){s*=2.0f;} s*=0.25f;
    V3 j={wfx.reloadContainerPos.x - s*0.5f*random_range(-1.0f,1.0f),wfx.reloadContainerPos.y,wfx.reloadContainerHome.z - s}; wfx.reloadContainerPos=(V3){j.x > 999.0f ? 0.0f : j.x, j.y > 999.0f ? 0.0f : j.y, j.z > 999.0f ? 0.0f : j.z }; wfx.recoiling=true;
}

static void Recoiling() { if(!wfx.recoiling){return;} float dt = (float)World.deltaTime * World.timeScale; wfx.reloadContainerPos.x+=(wfx.reloadContainerHome.x - wfx.reloadContainerPos.x)*dt; wfx.reloadContainerPos.z+=(wfx.reloadContainerHome.z - wfx.reloadContainerPos.z)*dt; }
static u8 lerpUp = 0; // 0 idle, 1 lerping up, 2 lerping down
double lerpStartTime = 0.0;
static void WeaponLerpGetTargetUp() { World.invP1.reloadLerpValue = (0.5f - (1.0f - World.invP1.reloadLerpValue)) / 0.5f; wfx.targetY = -1.0f * 0.66f * (1.0f - World.invP1.reloadLerpValue); if (wfx.targetY > wfx.reloadContainerHome.y){wfx.targetY = wfx.reloadContainerHome.y;} }
static void WeaponLerpGetTargetDown() { World.invP1.reloadLerpValue = World.invP1.reloadLerpValue / 0.5f; wfx.targetY = wfx.reloadContainerHome.y - 0.66f; wfx.targetY *= World.invP1.reloadLerpValue; }
void CompleteWeaponChange() {
    if (World.invP1.weaponIndexPending == -1) return;
    World.invP1.wepCur = (u8)World.invP1.wepCurPending;
    if (CurrentWeaponUsesEnergy()) { HudHeatBleed(World.invP1.curWepHeat[World.invP1.wepCur]); }
    World.invP1.weaponIndex = (u16)World.invP1.weaponIndexPending;
    u16 wepIdx = World.invP1.weaponIndex;
    Entity* vwe = &World.instances[World.weaponVModelIndex];
    int wep16 = Get16WeaponIndexFromConstIndex((int)wepIdx);
    if (wep16 >= 0 && wep16 < 16) {
        vwe->modelIndex = wepModelIndices[wep16]; if (vwe->modelIndex >= MAX_MDLS) vwe->modelIndex = MAX_MDLS - 1u;
        vwe->animationNum = wepAnimNums[wep16];
        vwe->texIndex = EDefs[wepIdx].texIndex >= MAXTEX ? 0u : EDefs[wepIdx].texIndex;
        vwe->normIndex = EDefs[wepIdx].normIndex >= MAXTEX ? 0u : EDefs[wepIdx].normIndex;
        vwe->glowIndex = EDefs[wepIdx].glowIndex >= MAXTEX ? 0u : EDefs[wepIdx].glowIndex;
        vwe->specIndex = EDefs[wepIdx].specIndex >= MAXTEX ? 0u : EDefs[wepIdx].specIndex;
        vwe->index = wepIdx; // constIndex, NOT 49/50
    } else { vwe->modelIndex = MAX_MDLS; /*hidden; no weapon equipped*/ }
    World.invP1.wepCurPending = World.invP1.weaponIndexPending = -1;
}

void WeaponFireStartWeaponDip(float t); // Forward declaration from citadel.c
void CycleWeaponSlot(int dir) { // dir: +1 = next, -1 = prev; dead in cyberspace, only 2 weapons so up is down
    if (World.curLev == LEVEL_CYBERSPACE) { World.invP1.isPulserNotDrill = !World.invP1.isPulserNotDrill; play_wav(sounds[80],AppliedFXVol(1.0f),(V3){0},false);/*changeweapon*/ return; }
    if (World.invP1.reloadFinished > World.pauseRelativeTime) return; // Dip in progress
    int curSlot = (int)World.invP1.wepCur;
    int nextSlot = curSlot;
    for (int attempt = 0; attempt < 7; ++attempt) {
        nextSlot += dir;
        if (nextSlot < 0) nextSlot = 6;
        if (nextSlot > 6) nextSlot = 0;
        if (nextSlot == curSlot) break;
        int wi = (int)World.invP1.wepInvIndices[nextSlot];
        if (wi >= 0 && wi < MAX_ENTITIES && Get16WeaponIndexFromConstIndex(wi) >= 0) break; // Valid weapon
    }
    if (nextSlot == curSlot) return;
    int wi = (int)World.invP1.wepInvIndices[nextSlot];
    play_wav(sounds[80],AppliedFXVol(1.0f),(V3){0},false);/*changeweapon*/
    World.invP1.wepCurPending = (i16)nextSlot;
    World.invP1.weaponIndexPending = (i16)wi;
    int w16=Get16WeaponIndexFromConstIndex(wi); WeaponFireStartWeaponDip(w16>=0?reloadTime[w16]:0.5f);
}

void StartWeaponDip(float delay) { if (delay < 0.0f) {delay = 0.0f;} World.invP1.reloadFinished = World.pauseRelativeTime + delay; lerpStartTime = World.pauseRelativeTime; }
extern V3 vWepOfs[16]; extern bool editFieldEditing; extern bool EditPanelPointerHover(void);
void UpdateWeaponReloadDip() {
    int i = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex);
    if (i < 0 || i > 15) {i = 0;}
    if (World.invP1.reloadFinished > World.pauseRelativeTime) {
        double elapsed = World.pauseRelativeTime - lerpStartTime;
        World.invP1.reloadLerpValue = (float)(elapsed / (World.invP1.reloadFinished - lerpStartTime));
        if (World.invP1.reloadLerpValue >= 0.5f) { lerpUp = 1; WeaponLerpGetTargetUp(); CompleteWeaponChange(); }
        else { lerpUp = 2; WeaponLerpGetTargetDown(); }
        wfx.targetY = vclamp(wfx.targetY, -100.0f, 100.0f);
        wfx.reloadContainerPos = (V3){wfx.reloadContainerPos.x, wfx.targetY, wfx.reloadContainerPos.z};
    } else { lerpUp = 0; wfx.reloadContainerPos = (V3){wfx.reloadContainerPos.x, wfx.reloadContainerHome.y + 0.20f, wfx.reloadContainerPos.z}; }
}

static u16 ImpactParticleType(u16 prefab) {
    switch (prefab) {
        case 721: return 13;  /* ef_particle_bloodspurtsmall */       case 722: return 15;  /* ef_particle_bloodspurtsmallgreen */
        case 723: return 16;  /* ef_particle_bloodspurtsmallyellow */ case 724: return 14;  /* ef_particle_bloodspurttiny */
        case 725: return 21;  /* ef_particle_camerahit */             case 726: return 23;  /* ef_particle_darthit */
        case 729: return 35;  /* ef_particle_sparksmall */            case 730: return 37;  /* ef_particle_sparkssmallblue */
        case 731: return 39;  /* ef_particle_sparqhit */              case 739: return 12;  /* ef_particle_blasterhit */
        case 740: return 26;  /* ef_particle_ionhit */              case 756: return 184; /* ef_particle_leafburst */
        case 757: return 185; /* ef_particle_mutationburst */       case 758: return 186; /* ef_particle_graytationburst */
        default: return U16_MAX;
    }
}

static const struct { u16 prefab; u8 type; } impactEffectTypes[] = {
    { 721, 13 },                                                    /* ef_particle_bloodspurtsmall */
    { 722, 14 }, { 722, 15 },                                       /* bloodspurtsmallgreen: tiny + green */
    { 723, 16 },                                                    /* bloodspurtsmallyellow */
    { 724, 17 },                                                    /* bloodspurttiny */
    { 725, 18 }, { 725, 19 }, { 725, 20 }, { 725, 21 }, { 725, 22 }, /* camerahit: Bits, CenterSpatter, DarkBurst, root, CenterSpatter2 */
    { 726, 23 },                                                    /* darthit */
    { 729, 35 }, { 729, 36 },                                       /* sparkssmall: root + centerburst */
    { 730, 37 },                                                    /* sparkssmallblue */
    { 731, 39 }, { 731, 40 }, { 731, 41 }, { 731, 42 }, { 731, 43 }, /* sparqhit: root, CenterSpatter2, BlueSparksSmall, CenterSpatter, Shockwave */
    { 739, 8 }, { 739, 9 }, { 739, 10 }, { 739, 11 }, { 739, 12 },   /* blasterhit: Shockwave, CenterSpatter, CenterSpatter2, BlueSparksSmall, root */
    { 740, 24 }, { 740, 25 }, { 740, 26 }, { 740, 27 }, { 740, 28 }, /* ionhit: Shockwave, CenterSpatter, root, BlueSparksSmall, CenterSpatter2 */
};

bool SpawnImpactParticleForPrefab(u16 prefab, V3 pos, V3 normal) {
    u16 type=ImpactParticleType(prefab); const PSysDef* preset=PSysTypeGet(type); if (!preset) return false;
    PSysDef def=*preset; def.pos=V3_AplusB(pos,V3_ScaleByF(normal,wfx.hitOffset-0.08f)); def.rotation=QuatFromToRotation((V3){0,1,0},normal); PSysAdd(&def);
    for (u32 i = 0; i < sizeof(impactEffectTypes)/sizeof(*impactEffectTypes); ++i) {
        if (impactEffectTypes[i].prefab != prefab || impactEffectTypes[i].type == type) continue;
        const PSysDef* cd = PSysTypeGet(impactEffectTypes[i].type); if (!cd) continue;
        PSysDef cdef = *cd; cdef.pos = def.pos; cdef.rotation = def.rotation; PSysAdd(&cdef);
    }
    return true;
}

void SpawnImpactEffectParticle(u16 prefab, V3 pos, V3 normal) { if (!SpawnImpactParticleForPrefab(prefab,pos,normal)) { u16 fx=SpawnDynamicObject(prefab,-1); if(EntIdxIsValid(fx)){World.position[fx]=V3_AplusB(pos,V3_ScaleByF(normal,wfx.hitOffset)); World.rotation[fx]=QuatFromToRotation((V3){0,1,0},normal);} } }
void SpawnBeamTrail(u16 presetType, V3 from, V3 to, Color c) {
    const PSysDef* preset=PSysTypeGet(presetType); if(!preset)return; V3 delta=V3_AsubB(to,from); float dist=V3_Mag(delta); if(dist<0.01f)return;
    const int BEAM_SEGS=8;
    const float life=0.04f, trailLife=0.40f;
    for (int seg=0; seg<BEAM_SEGS; ++seg) {
        PSysDef def=*preset; float f0=(float)seg/(float)BEAM_SEGS;
        V3 segFrom=V3_AplusB(from,V3_ScaleByF(delta,f0));
        def.pos=segFrom; def.rotation=QuatFromToRotation((V3){0,1,0},V3_Normalize(delta));
        for(int i=0;i<16;++i)def.textures[i]=MAXTEX;
        def.sizeMin=def.sizeMax=0.0f;
        def.shapeType=3;
        def.lifetimeMin=def.lifetimeMax=life;
        def.trailLifetime=trailLife;
        def.trailWidthStart=def.trailWidthEnd=0.05f;
        float segDist=dist/(float)BEAM_SEGS;
        def.speedMin=def.speedMax=segDist*(2.0f/life);
        def.velKeys[0]=1.0f; def.velKeys[1]=0.0f; def.velTimes[0]=0.0f; def.velTimes[1]=1.0f; def.velCount=2;
        def.trailColorStart=c; def.trailColorEnd=c;
        def.rampColors[0]=c; for(int i=1;i<def.rampCount;++i)def.rampColors[i]=c;
        def.emitRate=0.0f; def.burstCount=1; def.duration=life+0.05f; PSysAdd(&def);
    }
}
void SpawnTargetingLaser(V3 from, V3 to) { SpawnBeamTrail(167,from,to,(Color){1.0f,0.15f,0.18f,1.0f}); }
static const struct { u16 proj; u8 type; } projectileImpactTypes[] = {
    { 481, 177 },                                                   /* proj_enemshot2: impact_enemshot2 */
    { 482, 169 }, { 482, 170 },                                     /* magpulse: magpulse_center, sparks_spits */
    { 483, 171 }, { 483, 182 },                                     /* stungun: stungun_impacts, sparkssmall */
    { 484, 172 }, { 484, 173 }, { 484, 174 }, { 484, 175 },         /* railgun: root, centerumbra, shockwave, smoke */
    { 485, 176 }, { 485, 183 },                                     /* plasma: plasmarifle_hit, sparkssmall */
    { 486, 181 }, { 486, 173 }, { 486, 182 }, { 486, 174 },         /* enemshot6: root, centerumbra, sparkssmall, shockwave */
    { 487, 180 },                                                   /* enemshot5: the one AlphaBlend emitter in the game */
    { 488, 178 }, { 488, 179 },                                     /* enemshot4: root, particle_spits */
    { 489, 182 },                                                   /* proj_throwingstar: PoolType.SparksSmall */
    { 490, 169 }, { 490, 170 },                                     /* magpulsenpc: same pool as 482 */
    { 491, 172 }, { 491, 173 }, { 491, 174 }, { 491, 175 },         /* railnpc: same pool as 484 */
};
static u16 ProjectileImpactParticleType(u16 projectile) {
    for (u32 i = 0; i < sizeof(projectileImpactTypes)/sizeof(*projectileImpactTypes); ++i) {
        if (projectileImpactTypes[i].proj == projectile) return projectileImpactTypes[i].type;
    }
    return U16_MAX;
}
void SpawnProjectileImpactParticles(u16 projectile,V3 pos,V3 normal) {
    u16 type=ProjectileImpactParticleType(projectile); const PSysDef* preset=PSysTypeGet(type); if(!preset)return;
    PSysDef def=*preset; def.pos=V3_AplusB(pos,V3_ScaleByF(normal,wfx.hitOffset-0.08f)); def.rotation=QuatFromToRotation((V3){0,1,0},normal); PSysAdd(&def);
    for (u32 i = 0; i < sizeof(projectileImpactTypes)/sizeof(*projectileImpactTypes); ++i) {
        if (projectileImpactTypes[i].proj != projectile || projectileImpactTypes[i].type == type) continue;
        const PSysDef* cd=PSysTypeGet(projectileImpactTypes[i].type); if (!cd) continue;
        PSysDef cdef=*cd; cdef.pos=def.pos; cdef.rotation=def.rotation; PSysAdd(&cdef);
    }
}
static bool DidRayHit(int wep16){wfx.tempHitEnt=0xFFFF;float d=driftForWeapon[wep16];V3 dir=ScreenPointToRayPixels(World.instances[PLAYER1].forward,World.instances[PLAYER1].right,random_range(-d,d),random_range(-d,d));RaycastHit h=Raycast(World.position[PLAYER1],dir,wfx.fireDistance,LMASK_PLAYER_ATTACK);wfx.tempHit=h;if(h.hit){wfx.tempHitEnt=h.hitInstanceIndex;return true;}return false;}
static const float bulletHoleSize[6] = {0.06f,0.2f,0.1f,0.04f,0.15f,0.3f};
void CreateStandardImpactMarks(int wep16) {
    if (!wfx.tempHit.hit) return;
    Entity* e = &World.instances[wfx.tempHit.hitInstanceIndex];
    if ((e->entflags & EF_RIGIDBODY) || IdxIsDoor(e->index) || IdxIsNPC(e->index) || e->index==279/*chunk_screen*/ || e->index==477/*sec_camera*/) return;
    V3 pos = V3_AplusB(wfx.tempHit.point, V3_ScaleByF(wfx.tempHit.normal, 0.16f));
    u16 hole = wepBulletHolePrefab[wep16]; u16 markInst = SpawnDynamicObject(hole, -1); if (!EntIdxIsValid(markInst)) return;
    World.position[markInst] = pos;
    World.rotation[markInst] = quat_multiply(QuatFromToRotation((V3){0,1,0},V3_ScaleByF(wfx.tempHit.normal,-1.0f)),QuatEulerZ((float)(int)random_range(0.0f,3.99f) * 90.0f));
    if (hole >= 518 && hole <= 523) World.scale[markInst] = V3_ScaleByF((V3){1,1,1}, bulletHoleSize[hole - 518] / 1.28f);
}

void CreateStandardImpactEffects(){if(wfx.tempHitEnt==0xFFFF)return;u16 ent=wfx.tempHitEnt;if(ent>=World.instCount)return;u16 prefab=GetImpactType(ent);if(prefab==0||prefab>=MAX_ENTITIES)prefab=731;V3 pos=wfx.tempHit.hit?V3_AplusB(wfx.tempHit.point,V3_ScaleByF(wfx.tempHit.normal,wfx.hitOffset)):World.position[ent];V3 n=wfx.tempHit.hit?wfx.tempHit.normal:(V3){0,1,0};if(SpawnImpactParticleForPrefab(prefab,pos,n))return;u16 fx=SpawnDynamicObject(prefab,-1);if(EntIdxIsValid(fx)){World.position[fx]=pos;World.rotation[fx]=QuatFromToRotation((V3){0,1,0},n);}}
static void CreateBeamImpactEffects(int wep16) {
    int impactConstdex=731;/*Cyan sparq*/ if(wep16 == 1){impactConstdex=739;/*Red laser (blaster)*/}else if(wep16 == 4){impactConstdex=740;/*Yellow laser (ion)*/}
    if (SpawnImpactParticleForPrefab((u16)impactConstdex, wfx.tempHit.point, wfx.tempHit.normal)) return;
    u16 fx = SpawnDynamicObject((u16)impactConstdex, -1); if(!EntIdxIsValid(fx)){return;}
    World.position[fx] = wfx.tempHit.point; World.rotation[fx] = QuatFromToRotation((V3){0,1,0},wfx.tempHit.normal);
}

static void CreateBeamEffects(int wep16) {
    Color c=(wep16==1)?(Color){1.f,0.18f,0.12f,1.f}:(wep16==4)?(Color){1.f,0.9f,0.15f,1.f}:(Color){0.3f,1.f,1.f,1.f};
    V3 start=World.position[PLAYER1]; start.y-=0.2f;
    SpawnBeamTrail(166,start,wfx.tempHit.point,c);
}

static float DamageForPower(int w) { // Slope-of-slopes curve: interpolates damage/energy ratio across the energy setting, then scales by the interpolated energy drain itself. See design spreadsheet.
    if(World.invP1.overloadEnabled){return damageOverloadForWeapon[w];}
    float d=dmgForWep[w],D=dmgForWep2[w],e=energyDrainLowForWeapon[w],E=energyDrainHiForWeapon[w]; float pct=World.invP1.weaponEnergySetting[w]*0.01f; return (pct*(D/E - d/e) + 3.0f)*(pct*(E - e) + e);
}

float TakeDamage(u16 self,DamageData dd);
void HitScanFire(int wep16) {
    u16 ent = wfx.tempHitEnt;
    bool b = ent != 0xFFFF, isBeam=(wep16 == 1 || wep16 == 4 || wep16 == 14), npc = b && IdxIsNPC(World.instances[ent].index), alt = World.invP1.wepLoadedWithAlternate[World.invP1.wepCur];
    DamageData dd = {.isOtherNPC = npc, .hitpoint = wfx.tempHit.point, .attacknormal = ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right), .owner = PLAYER1,
                     .impactVelocity = wep16 == 12 ? 120.0f/*riotgun kicks harder*/ : 80.0f,.attackType=(wep16 == 2 && alt) ? Att_Trnq/*dartgun tranqs*/ : attTypeWep[wep16] };
    if (isBeam) CreateBeamImpactEffects(wep16);
    else { CreateStandardImpactEffects(); if (!npc && b && IdxIsGeometry(World.instances[ent].index)) CreateStandardImpactMarks(wep16); }
    float tranq = -1.0f; if (npc && dd.attackType == Att_Trnq){tranq=Tranquilize(ent,3.0f + random_range(0.0f,4.0f),false);}
    dd.damage=alt ? dmgForWep2[wep16] : (CurrentWeaponUsesEnergy() ? DamageForPower(wep16) : dmgForWep[wep16]); dd.offense=alt ? offenseWep2[wep16] : offenseWep[wep16]; dd.penetration=alt ? penetrationWep2[wep16] : penetrationWep[wep16];
    float dmgFinal = 0.0f;
    if (b && World.instances[ent].health > 0.0f) {
        dd.damage*=0.8f;/*rebalancing factor*/ dmgFinal=TakeDamage(ent,dd); dd.impactVelocity+=dd.damage;
        if (npc && !(World.instances[wfx.tempHitEnt].entflags & EF_ASLEEP)) { World.invP1.justFired = World.pauseRelativeTime; }
    }
    if (dmgFinal < 0.0f) dmgFinal = 0.0f;
    CreateTargetIDInstance(dmgFinal,ent,tranq);
    if (b && (!dd.isOtherNPC || wep16==12)) { float force=(wep16==12 ? 10.0f : 1.0f)*dd.impactVelocity; if (isBeam) ApplyImpactForce(ent,force,dd.attacknormal,dd.hitpoint); else ApplyImpactForceWithSound(ent,force,dd.attacknormal,dd.hitpoint); }
    if (isBeam){CreateBeamEffects(wep16);}
}

void BiomonitorEnergyPulse(float take);
void MeleeHitUpdate(void) {
    if (wfx.pendingMeleeFinished <= 0.0 || World.pauseRelativeTime < wfx.pendingMeleeFinished) return;
    
    wfx.pendingMeleeFinished = 0.0;
    int wep16 = wfx.pendingMeleeWep16;
    u16 targ = wfx.pendingMeleeTarget;
    bool isRapier = wfx.pendingMeleeIsRapier;
    bool silent = wfx.pendingMeleeSilent;
    if (targ == PLAYER1) return; // can't hit self
    
    DamageData dd = {.isOtherNPC = IdxIsNPC(World.instances[targ].index),.attacknormal = ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right),.damage = dmgForWep[wep16],.offense = offenseWep[wep16],
                     .penetration = penetrationWep[wep16],.owner = PLAYER1,.attackType = Att_Melee};
    if (isRapier) { dd.attackType = Att_MlEg; if (World.invP1.energy < 4.0f) dd.damage = dmgForWep[wep16] / 2.0f; /*half power on low energy*/}
    wfx.tempHitEnt = targ;
    CreateStandardImpactEffects();
    if (IdxIsGeometry(World.instances[targ].index)) CreateStandardImpactMarks(wep16);
    if(!IsDamageable(&World.instances[targ])){if(!silent){if(!isRapier){FootStepType fstp=GetFootstepTypeForPrefab(World.instances[targ].index);play_wav(JumpLandSound(fstp),AppliedFXVol(1.0f),World.position[PLAYER1],false);play_wav(sounds[wfx.pendingMeleeHitSnd],AppliedFXVol(0.65f),World.position[PLAYER1],false);}else{play_wav(sounds[wfx.pendingMeleeHitSnd],AppliedFXVol(1.0f),World.position[PLAYER1],false);}World.invP1.makingNoise=true;World.invP1.noiseFinished=World.pauseRelativeTime+0.5;}return;}/*no health manager: material footstep at 1.0 + hit sound at 0.65 (rapier: hit sound at 1.0)*/
    dd.impactVelocity = 80.0f + dd.damage;
    if ((!dd.isOtherNPC || wep16==12) && (!isRapier || World.invP1.energy >= 4.0f)) { ApplyImpactForceWithSound(targ,dd.impactVelocity,dd.attacknormal,World.position[targ]); }
    float dmgFinal = TakeDamage(targ,dd);
    if (dmgFinal < 0.0f) {dmgFinal = 0.0f;}
    CreateTargetIDInstance(dmgFinal,targ,-1.0f);
    if(!silent){World.invP1.makingNoise=true;World.invP1.noiseFinished=World.pauseRelativeTime+0.5;BloodType bt=World.instances[targ].bloodType;if(bt==BloodType_Red||bt==BloodType_Yellow||bt==BloodType_Green)play_wav(sounds[wfx.pendingMeleeFleshSnd], AppliedFXVol(1.0f), (V3){0}, false);else if(isRapier&&World.invP1.energy<4.0f)play_wav(sounds[67], AppliedFXVol(1.0f), (V3){0}, false);else play_wav(sounds[wfx.pendingMeleeHitSnd], AppliedFXVol(1.0f), (V3){0}, false);}
    if (isRapier) { TakeEnergy(3.666f); BiomonitorEnergyPulse(3.666f); } // 3 hits per energy tick
}

void PlayAnim(u8 player, u8 animClip); // stub: player animation playback
void PlayAnim(u8 player, u8 animClip) { (void)player; (void)animClip; /* stub: player animation playback deferred */ } // stub declaration
void FireMelee(int wep16, bool isRapier, bool silent, u16 hitSnd, u16 missSnd, u16 fleshSnd) {
    wfx.fireDistance = 3.2f;
    bool hit = DidRayHit(wep16); wfx.fireDistance = 200.0f;
    u16 t = wfx.tempHitEnt; double dt = World.pauseRelativeTime + (isRapier ? 0.28 : 0.15);
    if (hit) {
        PlayAnim(PLAYER1, isRapier ? A_ATTACK_HIT : A_ATTACK_HIT);
        u16 wvi = World.weaponVModelIndex; if (wvi > 0 && wvi < INSTANCE_COUNT) { World.instances[wvi].animationNum = isRapier ? 50 : 49; World.instances[wvi].clip = A_ATTACK_HIT; }
        wfx.pendingMeleeWep16 = wep16; wfx.pendingMeleeTarget = t; wfx.pendingMeleeIsRapier = isRapier; wfx.pendingMeleeSilent = silent; wfx.pendingMeleeHitSnd = hitSnd;
        wfx.pendingMeleeFleshSnd = fleshSnd; wfx.pendingMeleeFinished = dt; return;
    }
    V3 p = World.position[PLAYER1], look = V3_Normalize(ScreenPointToRay(World.instances[PLAYER1].forward, World.instances[PLAYER1].right));
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; i++) {
        Entity *in = &World.instances[i]; if(!(in->entflags & EF_ACTIVE) || in->health <= 0.0f || V3_Dist(World.position[i],p) >= 3.2f || V3_dot(look,V3_Normalize(V3_AsubB(World.position[i],p))) <= 0.666f){continue;}/*outside ~+-48deg cone*/
        PlayAnim(PLAYER1, A_ATTACK2);
        u16 wvi = World.weaponVModelIndex; if (wvi > 0 && wvi < INSTANCE_COUNT) { World.instances[wvi].animationNum = isRapier ? 50 : 49; World.instances[wvi].clip = A_ATTACK_HIT; }
        wfx.pendingMeleeWep16 = wep16; wfx.pendingMeleeTarget = i; wfx.pendingMeleeIsRapier = isRapier; wfx.pendingMeleeSilent = silent; wfx.pendingMeleeHitSnd = hitSnd;
        wfx.pendingMeleeFleshSnd = fleshSnd; wfx.pendingMeleeFinished = dt; return;
    }
    if(!silent)play_wav(sounds[missSnd], AppliedFXVol(1.0f), World.position[PLAYER1], false);PlayAnim(PLAYER1, A_ATTACK_MISS);
    u16 wvi = World.weaponVModelIndex; if (wvi > 0 && wvi < INSTANCE_COUNT) { World.instances[wvi].animationNum = isRapier ? 50 : 49; World.instances[wvi].clip = isRapier ? A_ATTACK_MISS : A_ATTACK_MISS; }
}

void FireRapier(int wep16) { FireMelee(wep16, true,  false, 246, 247, 246); } // wlaserrapier_hit/swing
void FirePipe(int wep16)   { FireMelee(wep16, false, false, 253, 254, 252); } // wpipe_hit/swing/dmg
static void AttachProjectileRibbon(u16 prefabID, u16 ball) {
    static const struct { u16 prefab, root, child; } ribbons[] = {
        { 482, 154, 155      }, /* proj_magpulse_shot:    bolt glow + sparks_spits */
        { 490, 154, 155      }, /* proj_magpulsenpc_shot: same pair */
        { 485, 156, U16_MAX },  /* proj_plasmarifle_shot: plasma core, no child emitter   */
        { 483, 159, U16_MAX },  /* proj_stungun_shot:     same pairing, not yet audited   */
    };
    for (u32 i = 0; i < sizeof(ribbons)/sizeof(*ribbons); ++i) {
        if (ribbons[i].prefab != prefabID) continue;
        if (ribbons[i].root != U16_MAX) { const PSysDef* root=PSysTypeGet(ribbons[i].root); if (root) { PSysDef d=*root; d.pos=World.position[ball]; PSysAddFollow(&d,ball); } }
        if (ribbons[i].child != U16_MAX) { const PSysDef* ch=PSysTypeGet(ribbons[i].child); if (ch) { PSysDef d=*ch; d.pos=World.position[ball]; PSysAddFollow(&d,ball); } }
    }
}
void FireBeachball(int wep16, float shoveForce, u16 prefabID) { // Acts like a beachball for NPC collisions, but a baseball for walls/floor (prevents corner-catching); handled by the projectile's own collider setup.
    u16 ball = SpawnDynamicObject(prefabID,1); if (!EntIdxIsValid(ball)) return;
    Entity* proj = &World.instances[ball]; u16 wc = World.invP1.wepCur; bool alt = World.invP1.wepLoadedWithAlternate[wc];
    World.layer[ball] = L_PlayerBullets; proj->forward = V3_Normalize(ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right));
    proj->damage = alt ? dmgForWep2[wep16] : (CurrentWeaponUsesEnergy() ? DamageForPower(wep16) : dmgForWep[wep16]); proj->strength = alt ? penetrationWep2[wep16] : penetrationWep[wep16]; proj->speed = alt ? offenseWep2[wep16] : offenseWep[wep16]; proj->attackType = attTypeWep[wep16]; proj->recentMostActivator = PLAYER1; ProjectileEffectImpactInitAfterLoad(ball);
    V3 spawn = World.position[PLAYER1];
    World.position[ball] = spawn; World.velocity[ball] = (V3){0}; // clear any stale velocity before the impulse
    AddForce(ball, V3_ScaleByF(proj->forward, shoveForce), true); flag_set(&proj->entflags,EF_ACTIVE | EF_RIGIDBODY,true);
    AttachProjectileRibbon(prefabID, ball);
}
void FireCyberBeachball(bool isPulser, float shoveForce, u16 prefabID) { // Same beachball/baseball split, but damage and attack type come from the held cyber software version
    u16 ball = SpawnDynamicObject(prefabID,1); if (!EntIdxIsValid(ball)) return;
    Entity* proj = &World.instances[ball];
    World.layer[ball] = L_PlayerBullets; proj->forward = V3_Normalize(ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right));
    proj->damage = isPulser ? (1.0f + 0.25f * (float)World.invP1.softVersions[SW_PULSER]) : 10.0f * (float)World.invP1.softVersions[SW_DRILL];
    proj->strength = proj->speed = 0.0f;/*Citadel leaves DamageData.penetration/offense at 0 on the cyber path*/
    proj->attackType = isPulser ? Att_Ball/*ProjectileLaunched*/ : Att_Drill; proj->recentMostActivator = PLAYER1; ProjectileEffectImpactInitAfterLoad(ball);
    World.position[ball] = World.position[PLAYER1]; World.velocity[ball] = (V3){0};
    AddForce(ball, V3_ScaleByF(proj->forward, shoveForce), true); flag_set(&proj->entflags,EF_ACTIVE | EF_RIGIDBODY,true);
}
void FireCyberWeapon(void) { // Reuses waitTilNextFire: cyberspace has no equipped weapon, so it is free to carry the cyber fire rate
    if (World.invP1.waitTilNextFire >= World.pauseRelativeTime) return;
    if (World.invP1.isPulserNotDrill) { if (!(World.invP1.hasSoft & (1u<<SW_PULSER))) return; World.shotsFired++; FireCyberBeachball(true,railgunShotForce,492); play_wav(sounds[258],AppliedFXVol(1.0f),World.position[PLAYER1],false);/*wpulser*/ World.invP1.waitTilNextFire = World.pauseRelativeTime + 0.08; }
    else { if (!(World.invP1.hasSoft & (1u<<SW_DRILL))) return; World.shotsFired++; FireCyberBeachball(false,plasmaShotForce,495); play_wav(sounds[241],AppliedFXVol(1.0f),World.position[PLAYER1],false);/*wdrill*/ World.invP1.waitTilNextFire = World.pauseRelativeTime + 0.5; }
    World.invP1.justFired = World.pauseRelativeTime;
}

void FirePlasma(int w){FireBeachball(w,plasmaShotForce,485);} void FireRailgun(int w){FireBeachball(w,railgunShotForce,484);} void FireMagpulse(int w){FireBeachball(w,magpulseShotForce,482);} void FireStungun(int w){FireBeachball(w,stungunShotForce,483);}
typedef void (*FireFn)(int); FireFn wepSpecialFire[16]={0,0,0,0,0,FireRapier,FirePipe,0,FireMagpulse,0,FirePlasma,FireRailgun,0,0,0,FireStungun};
void FireWeapon(int wep16,bool isSilent){if(wep16<0||wep16>15)return;if(wep16 != 5 && wep16 != 6){World.invP1.makingNoise=true; World.invP1.noiseFinished=World.pauseRelativeTime+0.5;} if(!isSilent&&wepClass[wep16] != WC_MELEE)play_wav(sounds[wepFireSound[wep16]], AppliedFXVol(1.0f), World.position[PLAYER1], false);if(wepClass[wep16]!=WC_MELEE){const PSysDef* mfp=PSysTypeGet(53/*centerFlash*/);if(mfp){PSysDef mfd=*mfp;mfd.pos=V3_AplusB(wfx.reloadContainerPos,V3_ScaleByF(World.instances[PLAYER1].forward,0.5f));mfd.lifetimeMin=mfd.lifetimeMax=0.05f;PSysAdd(&mfd);}} bool didHit=false;if(wepSpecialFire[wep16])wepSpecialFire[wep16](wep16);else{didHit=DidRayHit(wep16);if(didHit)HitScanFire(wep16);}if(wepSmokePrefab[wep16]){u16 smk=SpawnDynamicObject(wepSmokePrefab[wep16],-1);if(EntIdxIsValid(smk)){World.position[smk]=wfx.reloadContainerPos;World.rotation[smk]=World.rotation[PLAYER1];flag_set(&World.instances[smk].entflags,EF_ACTIVE,true);World.instances[smk].tickFinished=World.pauseRelativeTime+1.0;}}
    World.fogFac += wepFogInc[wep16]; u16 wc = World.invP1.wepCur;
    if (wepClass[wep16] == WC_ENERGY) {
        float setting = World.invP1.weaponEnergySetting[wep16];
        if (World.invP1.overloadEnabled) World.invP1.curWepHeat[wc] = 100.0f;
        else { World.invP1.curWepHeat[wc] += setting; if (World.invP1.curWepHeat[wc] > 100.0f) {World.invP1.curWepHeat[wc] = 100.0f;} }
    }
    bool wasOverload = World.invP1.overloadEnabled;
    if (wepClass[wep16] != WC_MELEE) { // Take ammo/energy. Melee weapons consume neither and don't count towards shotsFired.
        if (wepClass[wep16] == WC_ENERGY) {
            if (wasOverload) { World.invP1.overloadEnabled = false; if (!Cheats.bottomless && !Cheats.redbull) { TakeEnergy(energyDrainOverloadForWeapon[wep16]); BiomonitorEnergyPulse(energyDrainOverloadForWeapon[wep16]); } }
            else { float takeEnerg = (World.invP1.weaponEnergySetting[wep16]/100.0f) * (energyDrainHiForWeapon[wep16]-energyDrainLowForWeapon[wep16]); if (!Cheats.bottomless && !Cheats.redbull) { TakeEnergy(takeEnerg); BiomonitorEnergyPulse(takeEnerg); } }
        } else { if (World.invP1.wepLoadedWithAlternate[wc]) { if (!Cheats.bottomless) World.invP1.currentMagazineAmount2[wc]--; } else { if (!Cheats.bottomless) World.invP1.currentMagazineAmount[wc]--; } }
        World.shotsFired++;
    }
    Recoil(wep16);
    if (World.invP1.wepLoadedWithAlternate[wc] || wasOverload) { World.invP1.overloadEnabled = false; World.invP1.waitTilNextFire = World.pauseRelativeTime + delayBetweenShotsForWeapon2[wep16]; }
    else { World.invP1.waitTilNextFire = World.pauseRelativeTime + delayBetweenShotsForWeapon[wep16]; }
}

static bool pendingAttackWep16Valid = false; static int  pendingAttackWep16 = -1; void DropHeldItem(); void ResetHeldItem(); void AddItemToInventory(int index, int custIdx);
void StartNormalAttack(int wep16) { if ((wep16 < 0 || wep16 > 15) || (World.invP1.waitTilNextFire >= World.pauseRelativeTime) || (World.invP1.reloadFinished >= World.pauseRelativeTime)) return; pendingAttackWep16=wep16; pendingAttackWep16Valid=true; }
extern u16 editModeTestEntityDefinition;
void CheckAttackInput(void) {
    if(!Attack()){return;} if(World.Sys_UI.vmailActive) { World.Sys_UI.vmailActive=0; World.invP1.waitTilNextFire=World.pauseRelativeTime+0.8f; return;}
    if (World.curLev == LEVEL_CYBERSPACE) { FireCyberWeapon(); return; }
    if (editFieldEditing || EditPanelPointerHover()) { return; }
    if (Cheats.editMode && Cheats.editSubMode==1){World.invP1.holdingObject = true; World.invP1.heldObjectIndex = editModeTestEntityDefinition;}
    if (World.invP1.holdingObject && !World.mouseClickHeldOverGUI) { if (!World.uiIsBlocking) { if (FrobHeldItemIntoPanel(World.position[PLAYER1],World.instances[PLAYER1].forward,World.instances[PLAYER1].right)) { return; }/*frob-use items go to whatever UseHandler is under the cursor (InteractablePanel), and are consumed there*/ DropHeldItem(); return; } AddItemToInventory(World.invP1.heldObjectIndex,World.invP1.heldObjectCustIdx); ResetHeldItem(); return; }
    int w = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex);
    if (w == -1 || World.invP1.holdingObject || World.mouseClickHeldOverGUI){return; /*No weapon*/} StartNormalAttack(w);
}

void CheckUIStateAndAttack(void) {
    if (!pendingAttackWep16Valid) return;
    int wepdex = pendingAttackWep16; pendingAttackWep16Valid = false;
    if (World.uiIsBlocking || World.invP1.holdingObject || World.mouseClickHeldOverGUI || World.invP1.reloadFinished >= World.pauseRelativeTime || World.invP1.waitTilNextFire >= World.pauseRelativeTime || wepdex < 0 || wepdex > 15 || World.Sys_UI.fullMapOpen[0] || World.Sys_UI.fullMapOpen[1]) return;
    World.invP1.justFired = World.pauseRelativeTime; if (wepClass[wepdex] == WC_MELEE) { FireWeapon(wepdex,false); return; }
    if (wepClass[wepdex] == WC_ENERGY) {
        u16 wc = World.invP1.wepCur;
        if (World.invP1.energy > 0.0f || Cheats.bottomless || Cheats.redbull) {
            if(World.invP1.curWepHeat[wc]>wfx.overheatedPercent && !Cheats.bottomless && !Cheats.redbull){play_wav(sounds[238],AppliedFXVol(1.0f),(V3){0},false);/*noammo*/ World.invP1.waitTilNextFire=World.pauseRelativeTime + 0.8f; CenterStatusPrint("%s",Sys_Text.stringTable[11]);} 
            else { FireWeapon(wepdex,false); }
        } else { CenterStatusPrint("%s", Sys_Text.stringTable[207]);/*not enough energy*/ }
        return;
    }
    u16 wc = World.invP1.wepCur; bool alt = World.invP1.wepLoadedWithAlternate[wc];
    u16 amount = alt ? World.invP1.currentMagazineAmount2[wc] : World.invP1.currentMagazineAmount[wc];
    if (amount > 0 || Cheats.bottomless) { FireWeapon(wepdex, false); } else {play_wav(sounds[238],AppliedFXVol(1.0f),(V3){0},false);/*noammo*/ World.invP1.waitTilNextFire = World.pauseRelativeTime + 0.8f; }
}

void Unload(bool isSilent) {
    if (World.invP1.weaponIndex >= 0x8000 || (i32)World.invP1.weaponIndex < 0) return;
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); if (wep16 < 0 || wepClass[wep16] == WC_MELEE) return;
    u16 wc = World.invP1.wepCur;
    if(World.invP1.wepLoadedWithAlternate[wc]){World.invP1.wepAmmoSecondary[wep16]+=World.invP1.currentMagazineAmount2[wc]; World.invP1.currentMagazineAmount2[wc]=0;}else{World.invP1.wepAmmo[wep16]+=World.invP1.currentMagazineAmount[wc]; World.invP1.currentMagazineAmount[wc]=0;}
    (void)isSilent; if (!isSilent) play_wav(sounds[260], AppliedFXVol(1.0f), (V3){0}, false); // wreload
}

void LoadPrimaryAmmoType(bool isSilent) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); u16 wc = World.invP1.wepCur;
    if (!World.invP1.wepLoadedWithAlternate[wc]) {
        if (World.invP1.currentMagazineAmount[wc] == magazinePitchCountForWeapon[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[191]); return; }
        if (World.invP1.currentMagazineAmount[wc] == World.invP1.wepAmmo[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[535]); return; }
    }
    Unload(true); World.invP1.wepLoadedWithAlternate[wc] = false; 
    World.invP1.currentMagazineAmount[wc] = (World.invP1.wepAmmo[wep16] >= magazinePitchCountForWeapon[wep16]) ? magazinePitchCountForWeapon[wep16] : (u8)World.invP1.wepAmmo[wep16];
    World.invP1.wepAmmo[wep16] -= World.invP1.currentMagazineAmount[wc];
    (void)isSilent; if (!isSilent) play_wav(sounds[(wep16==0 || wep16==3) ? 248 : 260], AppliedFXVol(1.0f), (V3){0}, false); // wlocknload / wreload
    StartWeaponDip(reloadTime[wep16]); wfx.reloadContainerPos = wfx.reloadContainerHome;
}

void LoadSecondaryAmmoType(bool isSilent) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); u16 wc = World.invP1.wepCur;
    if (World.invP1.wepLoadedWithAlternate[wc]) {
        if (World.invP1.currentMagazineAmount2[wc] == magazinePitchCountForWeapon2[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[191]); return; }
        if (World.invP1.currentMagazineAmount2[wc] == World.invP1.wepAmmoSecondary[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[535]); return; }
    }
    Unload(true); World.invP1.wepLoadedWithAlternate[wc] = true;
    World.invP1.currentMagazineAmount2[wc] = (World.invP1.wepAmmoSecondary[wep16] >= magazinePitchCountForWeapon2[wep16]) ? magazinePitchCountForWeapon2[wep16] : (u8)World.invP1.wepAmmoSecondary[wep16];
    World.invP1.wepAmmoSecondary[wep16] -= World.invP1.currentMagazineAmount2[wc];
    (void)isSilent; if (!isSilent) play_wav(sounds[(wep16==0 || wep16==3) ? 248 : 260], AppliedFXVol(1.0f), (V3){0}, false); // wlocknload / wreload
    StartWeaponDip(reloadTime[wep16]); wfx.reloadContainerPos = wfx.reloadContainerHome;
}

void ReloadSecret(bool isSilent) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); if (wep16 < 0) {return;}
    if (wepClass[wep16] == WC_MELEE) { CenterStatusPrint("%s", Sys_Text.stringTable[315]); return; }
    if (wepClass[wep16] == WC_ENERGY) { CenterStatusPrint("%s", Sys_Text.stringTable[538]); return; }
    u16 wc = World.invP1.wepCur;
    if (World.invP1.wepLoadedWithAlternate[wc]) {
        if (World.invP1.currentMagazineAmount2[wc] == magazinePitchCountForWeapon2[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[191]); return; }
        if (World.invP1.wepAmmoSecondary[wep16] <= 0) { if (World.invP1.wepAmmo[wep16] <= 0) { CenterStatusPrint("%s", Sys_Text.stringTable[305]); return; } CenterStatusPrint("%s", Sys_Text.stringTable[192]); LoadPrimaryAmmoType(isSilent); return; }
        LoadSecondaryAmmoType(isSilent);
    } else {
        if (World.invP1.currentMagazineAmount[wc] == magazinePitchCountForWeapon[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[191]); return; }
        if (World.invP1.wepAmmo[wep16] <= 0) { if (World.invP1.wepAmmoSecondary[wep16] <= 0) { CenterStatusPrint("%s", Sys_Text.stringTable[305]); return; } CenterStatusPrint("%s", Sys_Text.stringTable[192]); LoadSecondaryAmmoType(isSilent); return; }
        LoadPrimaryAmmoType(isSilent);
    }
}

void CheckReloadInput(void) {
    if (World.invP1.reloadFinished >= World.pauseRelativeTime || !Reload() || (int)World.invP1.wepCur < 0) return;
    if (Sys_Settings.QuickReloadWeapons) { ReloadSecret(false); return; }
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); if (wep16 < 0) return;
    u16 wc = World.invP1.wepCur;
    bool alt = World.invP1.wepLoadedWithAlternate[wc];
    if (alt ? (World.invP1.currentMagazineAmount2[wc] <= 0 || World.invP1.wepAmmoSecondary[wep16] <= 0) : (World.invP1.currentMagazineAmount[wc] <= 0 || World.invP1.wepAmmo[wep16] <= 0)) ReloadSecret(false); else Unload(false);
}

void ActualChangeAmmoType(void) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); if (wep16 < 0) {return;}
    if (wepClass[wep16] == WC_MELEE) { CenterStatusPrint("%s", Sys_Text.stringTable[315]); return; }
    if (wepClass[wep16] == WC_ENERGY) {
        u16 wc = World.invP1.wepCur;
        if (World.invP1.curWepHeat[wc] > 25.0f) { CenterStatusPrint("%s", Sys_Text.stringTable[12]); return; }
        if (World.invP1.overloadEnabled) { CenterStatusPrint("%s", Sys_Text.stringTable[13]); World.invP1.overloadEnabled = false; } else { CenterStatusPrint("%s", Sys_Text.stringTable[17]); World.invP1.overloadEnabled = true; }
        return;
    }
    u16 wc = World.invP1.wepCur;
    if (World.invP1.wepLoadedWithAlternate[wc]) {
        if (World.invP1.wepAmmo[wep16] > 0) {
            World.invP1.wepLoadedWithAlternate[wc] = false;
            World.invP1.wepAmmoSecondary[wep16] += World.invP1.currentMagazineAmount2[wc];
            World.invP1.currentMagazineAmount2[wc] = 0;
            LoadPrimaryAmmoType(false);
        } else { CenterStatusPrint("%s", Sys_Text.stringTable[535]); }
    } else {
        if (World.invP1.wepAmmoSecondary[wep16] > 0) {
            World.invP1.wepLoadedWithAlternate[wc] = true;
            World.invP1.wepAmmo[wep16] += World.invP1.currentMagazineAmount[wc];
            World.invP1.currentMagazineAmount[wc] = 0;
            LoadSecondaryAmmoType(false);
        } else { CenterStatusPrint("%s", Sys_Text.stringTable[535]); }
    }
}

void WeaponsUpdate(void) { HeatBleedOff(); if (World.fogFac > 255) {World.fogFac = 255;} UpdateWeaponReloadDip(); Recoiling(); CheckAttackInput(); CheckUIStateAndAttack(); CheckReloadInput(); if (ChangeAmmoType()){ActualChangeAmmoType();} MeleeHitUpdate(); }
