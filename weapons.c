// weapons.c - Weapon System
#include "common.h"
FootStepType GetFootstepTypeForPrefab(int pid); bool ChangeAmmoType();
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
u16 wepAnimNums[16]={MAX_ANIMS,MAX_ANIMS,MAX_ANIMS,MAX_ANIMS,MAX_ANIMS,50,49,MAX_ANIMS,MAX_ANIMS,MAX_ANIMS,MAX_ANIMS,MAX_ANIMS,MAX_ANIMS,MAX_ANIMS,MAX_ANIMS,MAX_ANIMS}; // [wep16] only rapier(50)/pipe(49)
typedef enum { WC_STD=0, WC_MELEE=1, WC_ENERGY=2 } WepClass;
WepClass wepClass[16]={WC_STD,WC_ENERGY,WC_STD,WC_STD,WC_ENERGY,WC_MELEE,WC_MELEE,WC_STD,WC_STD,WC_STD,WC_ENERGY,WC_STD,WC_STD,WC_STD,WC_ENERGY,WC_ENERGY};
float magpulseShotForce=2.2f, stungunShotForce=2.2f, railgunShotForce=5.0f, plasmaShotForce=1.5f;
WeaponFireCtx wfx = { .verticalOffset=-0.2f,.fireDistance=200.0f,.overheatedPercent=80.0f,.reloadContainerHome={0},.pendingMeleeFinished=0.0 };
INLINE Quaternion QuatEulerY(float degY) { float r=deg2rad(degY)*0.5f; return (Quaternion){0.0f,vsinf(r),0.0f,vcosf(r)}; }
INLINE Quaternion QuatEulerZ(float degZ) { float r=deg2rad(degZ)*0.5f; return (Quaternion){0.0f,0.0f,vsinf(r),vcosf(r)}; }
INLINE Quaternion QuatFromToRotation(V3 from,V3 to) {
    V3 f=V3_Normalize(from),t=V3_Normalize(to);
    float d=V3_dot(f,t);
    if (d > 0.999999f) return QUAT_IDENTITY;
    if (d < -0.999999f) {
        V3 ax=V3_Cross((V3){1.0f,0.0f,0.0f},f);
        if (V3_Mag(ax) < 0.000001f) ax=V3_Cross((V3){0.0f,1.0f,0.0f},f);
        ax=V3_Normalize(ax);
        return (Quaternion){ax.x,ax.y,ax.z,0.0f};
    }
    V3 ax=V3_Cross(f,t);
    float s=vsqrtf((1.0f+d)*2.0f), invs=1.0f/s;
    return (Quaternion){ax.x*invs,ax.y*invs,ax.z*invs,s*0.5f};
}

INLINE bool WeaponsHaveAnyHeat() { if (Cheats.redbull) {return false;} for (int i=0;i<7;i++) {if (World.invP1.currentEnergyWeaponHeat[i] > 0.0f) {return true;}} return false; }
void HeatBleedOff() {
    static double heatTickFinished = 0.0;
    static const float heatTickTime = 0.50f;
    if (heatTickFinished >= World.pauseRelativeTime) return;
    World.fogFac--;
    if (World.fogFac < 0) World.fogFac = 0;
    if (WeaponsHaveAnyHeat() || CurrentWeaponUsesEnergy()) {
        for (int i=0;i<7;i++) {
            World.invP1.currentEnergyWeaponHeat[i] -= 10.0f;
            if (World.invP1.currentEnergyWeaponHeat[i] <= 0.0f) World.invP1.currentEnergyWeaponHeat[i] = 0.0f;
        }
        // if (CurrentWeaponUsesEnergy()) HudHeatBleed(World.invP1.currentEnergyWeaponHeat[World.invP1.weaponCurrent]); TODO
    }
    heatTickFinished = World.pauseRelativeTime + heatTickTime;
}

static void Recoil(int wep16) {
    float s=recoilForWeapon[wep16]; if(s<=0.0f){return;} if(World.instances[PLAYER1].health>0.0f && World.invP1.fatigue>80.0f){s*=2.0f;} s*=0.25f;
    V3 j={wfx.reloadContainerPos.x - s*0.5f*random_range(-1.0f,1.0f),wfx.reloadContainerPos.y,wfx.reloadContainerHome.z - s}; wfx.reloadContainerPos=(V3){j.x > 999.0f ? 0.0f : j.x, j.y > 999.0f ? 0.0f : j.y, j.z > 999.0f ? 0.0f : j.z }; wfx.recoiling=true;
}

static void Recoiling() { if(!wfx.recoiling){return;} float dt = (float)World.deltaTime; wfx.reloadContainerPos.x+=(wfx.reloadContainerHome.x - wfx.reloadContainerPos.x)*dt; wfx.reloadContainerPos.z+=(wfx.reloadContainerHome.z - wfx.reloadContainerPos.z)*dt; }
// ---- Weapon dip (reload/swap "animation") ----------------------------------
static float reloadLerpValue = 0.0f;
static u8 lerpUp = 0; // 0 idle, 1 lerping up, 2 lerping down
double lerpStartTime = 0.0;
static void WeaponLerpGetTargetUp() { reloadLerpValue = (0.5f - (1.0f - reloadLerpValue)) / 0.5f; wfx.targetY = -1.0f * 0.66f * (1.0f - reloadLerpValue); if (wfx.targetY > wfx.reloadContainerHome.y){wfx.targetY = wfx.reloadContainerHome.y;} }
static void WeaponLerpGetTargetDown() { reloadLerpValue = reloadLerpValue / 0.5f; wfx.targetY = wfx.reloadContainerHome.y - 0.66f; wfx.targetY *= reloadLerpValue; }
void CompleteWeaponChange() {
    if (World.invP1.weaponIndexPending == -1) return;
    World.invP1.weaponCurrent = (u8)World.invP1.weaponCurrentPending;
    if (CurrentWeaponUsesEnergy()) { /*HudHeatBleed(World.invP1.currentEnergyWeaponHeat[World.invP1.weaponCurrent]);*/ }
    World.invP1.weaponIndex = (u16)World.invP1.weaponIndexPending;
    // Set the single ad-hoc view-model instance's appearance.  Model indices come from ./Data/models.txt:
    // only pipe/rapier use the v_pipe/v_rapier models (5728/5748); all others reuse the matching weapon_ EDefs model.
    // animationNum is 49 (v_pipe) / 50 (v_rapier); everything else has no animation (MAX_ANIMS).
    u16 wepIdx = World.invP1.weaponIndex;
    Entity* vwe = &World.instances[World.weaponVModelIndex];
    int wep16 = Get16WeaponIndexFromConstIndex((int)wepIdx);
    if (wep16 >= 0 && wep16 < 16) {
        vwe->modelIndex = wepModelIndices[wep16]; if (vwe->modelIndex >= MAX_MDLS) vwe->modelIndex = MAX_MDLS - 1u;
        vwe->animationNum = wepAnimNums[wep16];
        vwe->texIndex = EDefs[wepIdx].texIndex >= MAX_TXRS ? 0u : EDefs[wepIdx].texIndex;
        vwe->normIndex = EDefs[wepIdx].normIndex >= MAX_TXRS ? 0u : EDefs[wepIdx].normIndex;
        vwe->glowIndex = EDefs[wepIdx].glowIndex >= MAX_TXRS ? 0u : EDefs[wepIdx].glowIndex;
        vwe->specIndex = EDefs[wepIdx].specIndex >= MAX_TXRS ? 0u : EDefs[wepIdx].specIndex;
        vwe->index = wepIdx; // constIndex, NOT 49/50
    } else {
        vwe->modelIndex = MAX_MDLS; // hidden; no weapon equipped
    }
    World.invP1.weaponCurrentPending = World.invP1.weaponIndexPending = -1;
    int ind = World.invP1.weaponIndex;
    bool alt = (ind >= 0 && ind < 16) ? World.invP1.wepLoadedWithAlternate[ind] : false;
    (void)alt; /*SetAmmoIcons(ind, alt);*/ /*SetWepInfo(World.invP1.weaponIndex);*/
}

void WeaponFireStartWeaponDip(float t); // Forward declaration from citadel.c
void CycleWeaponSlot(int dir) { // dir: +1 = next, -1 = prev
    if (World.invP1.reloadFinished > World.pauseRelativeTime) return; // Dip in progress
    int curSlot = (int)World.invP1.weaponCurrent;
    int nextSlot = curSlot;
    for (int attempt = 0; attempt < 7; ++attempt) {
        nextSlot += dir;
        if (nextSlot < 0) nextSlot = 6;
        if (nextSlot > 6) nextSlot = 0;
        if (nextSlot == curSlot) break;
        int wi = (int)World.invP1.weaponInventoryIndices[nextSlot];
        if (wi >= 0 && wi < MAX_ENTITIES && Get16WeaponIndexFromConstIndex(wi) >= 0) break; // Valid weapon
    }
    if (nextSlot == curSlot) return;
    int wi = (int)World.invP1.weaponInventoryIndices[nextSlot];
    World.invP1.weaponCurrentPending = (i16)nextSlot;
    World.invP1.weaponIndexPending = (i16)wi;
    WeaponFireStartWeaponDip(0.5f);
}

void StartWeaponDip(float delay) { if (delay < 0.0f) {delay = 0.0f;} World.invP1.reloadFinished = World.pauseRelativeTime + delay; lerpStartTime = World.pauseRelativeTime; }
extern V3 vWepOfs[16];
void UpdateWeaponReloadDip() {
    int i = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex);
    if (i < 0 || i > 15) {i = 0;}
    if (World.invP1.reloadFinished > World.pauseRelativeTime) {
        double elapsed = World.pauseRelativeTime - lerpStartTime;
        reloadLerpValue = (float)(elapsed / (World.invP1.reloadFinished - lerpStartTime));
        if (reloadLerpValue >= 0.5f) { lerpUp = 1; WeaponLerpGetTargetUp(); CompleteWeaponChange(); }
        else { lerpUp = 2; WeaponLerpGetTargetDown(); }
        wfx.targetY = vclamp(wfx.targetY, -100.0f, 100.0f);
        wfx.reloadContainerPos = (V3){wfx.reloadContainerPos.x, wfx.targetY, wfx.reloadContainerPos.z};
    } else { lerpUp = 0; wfx.reloadContainerPos = (V3){wfx.reloadContainerPos.x, wfx.reloadContainerHome.y + 0.20f, wfx.reloadContainerPos.z}; }
}

static void RotateViewWeapon() { if(!World.inventoryMode) {wfx.reloadContainerRot=QUAT_IDENTITY; return;} float h = (float)Sys_Settings.ScreenWidth * 0.5f; wfx.reloadContainerRot = QuatEulerY(wfx.wepYRot = ((wfx.tempVec.x - h) / h) * 48.0f); }
static bool DidRayHit(int wep16){wfx.tempHitEnt=0xFFFF;float d=driftForWeapon[wep16];V3 dir=ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right);dir.x+=random_range(-d,d);dir.y+=random_range(-d,d);RaycastHit h=Raycast(World.position[PLAYER1],dir,wfx.fireDistance,LMASK_PLAYER_ATTACK);wfx.tempHit=h;if(h.hit){wfx.tempHitEnt=h.hitInstanceIndex;return true;}return false;}
void CreateStandardImpactMarks(int wep16) {
    if (!wfx.tempHit.hit) return;
    Entity* e = &World.instances[wfx.tempHit.hitInstanceIndex];
    if ((e->entflags & EF_RIGIDBODY) || IdxIsDoor(e->index)) return; // Don't create bullet holes on objects that move.
    V3 pos = V3_AplusB(wfx.tempHit.point, V3_ScaleByF(wfx.tempHit.normal, 0.16f));
    u16 markInst = SpawnDynamicObject(wepBulletHolePrefab[wep16], -1); if (markInst == 0xFFFF) return;
    World.position[markInst] = pos;
    World.rotation[markInst] = quat_multiply(QuatFromToRotation((V3){0,1,0},V3_ScaleByF(wfx.tempHit.normal,-1.0f)),QuatEulerZ((float)(int)random_range(0.0f,3.99f) * 90.0f));
}

void CreateStandardImpactEffects(){if(wfx.tempHitEnt==0xFFFF)return;u16 ent=wfx.tempHitEnt;if(ent>=World.instCount)return;u16 prefab=GetImpactType(ent);if(prefab==0||prefab>=MAX_ENTITIES)prefab=731;V3 pos=wfx.tempHit.hit?V3_AplusB(wfx.tempHit.point,V3_ScaleByF(wfx.tempHit.normal,wfx.hitOffset)):World.position[ent];V3 n=wfx.tempHit.hit?wfx.tempHit.normal:(V3){0,1,0};u16 fx=SpawnDynamicObject(prefab,-1);if(fx!=0xFFFF&&fx<INSTANCE_COUNT){World.position[fx]=pos;World.rotation[fx]=QuatFromToRotation((V3){0,1,0},n);}}
static void CreateBeamImpactEffects(int wep16) {
    int impactConstdex=731;/*Cyan sparq*/ if(wep16 == 1){impactConstdex=739;/*Red laser (blaster)*/}else if(wep16 == 4){impactConstdex=740;/*Yellow laser (ion)*/}
    u16 fx = SpawnDynamicObject((u16)impactConstdex, -1); if(fx == 0xFFFF){return;}
    World.position[fx] = wfx.tempHit.point; World.rotation[fx] = QuatFromToRotation((V3){0,1,0},wfx.tempHit.normal);
}

static void CreateBeamEffects(int wep16) {
    u16 laserPrefab=405;/*sparq*/ if(wep16 == 1){laserPrefab=406;/*blaster*/} else if(wep16 == 4){laserPrefab=407;/*ion*/}
    u16 beam = SpawnDynamicObject(laserPrefab, -1); if (beam == 0xFFFF) return;
    World.position[beam] = wfx.reloadContainerPos; // muzzle-relative start point, LaserDrawing.startPoint/endPoint equivalent handled by the beam entity's own update, using its position and a linked end-point field (not modeled here).
}

static float DamageForPower(int w) { // Slope-of-slopes curve: interpolates damage/energy ratio across the energy setting, then scales by the interpolated energy drain itself. See design spreadsheet.
    if(World.invP1.overloadEnabled){return damageOverloadForWeapon[w];}
    float d=dmgForWep[w],D=dmgForWep2[w],e=energyDrainLowForWeapon[w],E=energyDrainHiForWeapon[w]; float pct=World.invP1.weaponEnergySetting[World.invP1.weaponCurrent]*0.01f; return (pct*(D/E - d/e) + 3.0f)*(pct*(E - e) + e);
}

float TakeDamage(u16 self,DamageData dd);
void HitScanFire(int wep16) {
    u16 ent=wfx.tempHitEnt; bool b=ent != 0xFFFF, isBeam=(wep16 == 1 || wep16 == 4 || wep16 == 14), npc=b && IdxIsNPC(World.instances[ent].index), alt=World.invP1.wepLoadedWithAlternate[World.invP1.weaponCurrent];
    DamageData dd = {.hitIdx=ent,.isOtherNPC=npc,.hitpoint=wfx.tempHit.point,.attacknormal=ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right),.owner=PLAYER1,
                     .impactVelocity=wep16 == 12 ? 120.0f/*riotgun kicks harder*/ : 80.0f,.attackType=(wep16 == 2 && alt) ? Att_Trnq/*dartgun tranqs*/ : attTypeWep[wep16] };
    if (isBeam) CreateBeamImpactEffects(wep16);
    else { CreateStandardImpactEffects(); if (!npc && b && IdxIsGeometry(World.instances[ent].index)) CreateStandardImpactMarks(wep16); }
    float tranq = -1.0f; if (npc && dd.attackType == Att_Trnq){tranq=Tranquilize(ent,3.0f + random_range(0.0f,4.0f),false);}
    dd.damage=alt ? dmgForWep2[wep16] : (CurrentWeaponUsesEnergy() ? DamageForPower(wep16) : dmgForWep[wep16]); dd.offense=alt ? offenseWep2[wep16] : offenseWep[wep16]; dd.penetration=alt ? penetrationWep2[wep16] : penetrationWep[wep16];
    float dmgFinal = 0.0f;
    if (b && World.instances[ent].health > 0.0f) {
        dd.damage*=0.8f;/*rebalancing factor*/ dmgFinal=TakeDamage(ent,dd);
        if (!dd.isOtherNPC || wep16==12) { AddForce((u16)wfx.tempHitEnt, V3_ScaleByF(dd.attacknormal, dd.impactVelocity * 0.01f), false); } // impact force linked to physics
        if (npc && !(World.instances[wfx.tempHitEnt].entflags & EF_ASLEEP)) { /* Music combat state set; function deferred */ }
    if (dmgFinal < 0.0f) dmgFinal = 0.0f;
    (void)dmgFinal; (void)tranq; // CreateTargetIDInstance placeholder: data captured but instance tracking deferred
    // TODO: CreateTargetIDInstance for deferred impact tracking when HealthManager ported
    } if (isBeam){CreateBeamEffects(wep16);}
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
    DamageData dd = {.hitIdx = targ,.isOtherNPC = IdxIsNPC(World.instances[targ].index),.attacknormal = ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right),.damage = dmgForWep[wep16],.offense = offenseWep[wep16],
                     .penetration = penetrationWep[wep16],.owner = PLAYER1,.attackType = Att_Melee};
    if (isRapier) { dd.attackType = Att_MlEg; if (World.invP1.energy < 4.0f) dd.damage = dmgForWep[6] / 2.0f; /*half power on low energy*/}
    // uou.HitForce(dd); // knock physics objects around - hook up to physics engine TODO
    wfx.tempHitEnt = targ;
    CreateStandardImpactEffects();
    if (IdxIsGeometry(World.instances[targ].index)) CreateStandardImpactMarks(wep16);
    if(World.instances[targ].health<=0.0f&&!dd.isOtherNPC){if(!silent){play_wav(sounds[wfx.pendingMeleeHitSnd],1.0f,World.position[targ],false);World.invP1.noiseFinished=World.pauseRelativeTime+0.5;}return;}
    dd.impactVelocity = 80.0f + dd.damage; //if ((!dd.isOtherNPC || wep16==12) && (!isRapier || World.invP1.energy >= 4.0f)) { ApplyImpactForce(targ,dd.impactVelocity,dd.attacknormal,dd.hitpoint); } TODO
    float dmgFinal = TakeDamage(targ,dd);
    if (dmgFinal < 0.0f) {dmgFinal = 0.0f;}
    (void)dmgFinal; // CreateTargetIDInstance(dmgFinal, targ, -1.0f); TODO
    if(!silent){World.invP1.noiseFinished=World.pauseRelativeTime+0.5;BloodType bt=World.instances[targ].bloodType;if(bt==BloodType_Red||bt==BloodType_Yellow||bt==BloodType_Green)play_wav(sounds[wfx.pendingMeleeFleshSnd],1.0f,(V3){0,0,0},false);else if(isRapier&&World.invP1.energy<4.0f)play_wav(sounds[67],1.0f,(V3){0,0,0},false);else play_wav(sounds[wfx.pendingMeleeHitSnd],1.0f,(V3){0,0,0},false);}
    if (isRapier) { TakeEnergy(3.666f); BiomonitorEnergyPulse(3.666f); } // 3 hits per energy tick
}

void PlayAnim(u8 player, u8 animClip); // stub: player animation playback
void PlayAnim(u8 player, u8 animClip) { (void)player; (void)animClip; /* stub: player animation playback deferred */ } // stub declaration
void FireMelee(int wep16, bool isRapier, bool silent, u16 hitSnd, u16 missSnd, u16 fleshSnd) {
    wfx.fireDistance = 3.2f;
    bool hit = DidRayHit(wep16); wfx.fireDistance = 200.0f;
    u16 t = wfx.tempHitEnt; double dt = World.pauseRelativeTime + (isRapier ? 0.28 : 0.15);
    // Use appropriate animation clips from models.c: v_rapier (50) uses A_ATTACK_HIT (4), v_pipe (49) uses A_ATTACK_HIT (18) for hit
    if (hit) {
        PlayAnim(PLAYER1, isRapier ? A_ATTACK_HIT : A_ATTACK_HIT);
        wfx.pendingMeleeWep16 = wep16; wfx.pendingMeleeTarget = t; wfx.pendingMeleeIsRapier = isRapier; wfx.pendingMeleeSilent = silent; wfx.pendingMeleeHitSnd = hitSnd; wfx.pendingMeleeMissSnd = missSnd;
        wfx.pendingMeleeFleshSnd = fleshSnd; wfx.pendingMeleeFinished = dt; return;
    }
    V3 p = World.position[PLAYER1], look = V3_Normalize(ScreenPointToRay(World.instances[PLAYER1].forward, World.instances[PLAYER1].right));
    for (u16 i = INSTS_1ST_IDX; i < World.instCount; i++) {
        Entity *in = &World.instances[i]; if(!(in->entflags & EF_ACTIVE) || in->health <= 0.0f || V3_Dist(World.position[i],p) >= 3.2f || V3_dot(look,V3_Normalize(V3_AsubB(World.position[i],p))) <= 0.666f){continue;}/*outside ~+-48deg cone*/
        PlayAnim(PLAYER1, A_ATTACK2);
        wfx.pendingMeleeWep16 = wep16; wfx.pendingMeleeTarget = i; wfx.pendingMeleeIsRapier = isRapier; wfx.pendingMeleeSilent = silent; wfx.pendingMeleeHitSnd = hitSnd; wfx.pendingMeleeMissSnd = missSnd;
        wfx.pendingMeleeFleshSnd = fleshSnd; wfx.pendingMeleeFinished = dt; return;
    }
    if(!silent)play_wav(sounds[missSnd],1.0f,World.position[PLAYER1],false);PlayAnim(PLAYER1,isRapier?A_ATTACK2:A_ATTACK1);
}

void FireRapier(int wep16) { FireMelee(wep16, true,  false, 246, 247, 246); } // wlaserrapier_hit/swing
void FirePipe(int wep16)   { FireMelee(wep16, false, false, 253, 254, 252); } // wpipe_hit/swing/dmg
void FireBeachball(int wep16, float shoveForce, u16 prefabID) { // Acts like a beachball for NPC collisions, but a baseball for walls/floor (prevents corner-catching); handled by the projectile's own collider setup.
    u16 ball = SpawnDynamicObject(prefabID,1);
    DamageData dd={.damage=CurrentWeaponUsesEnergy() ? DamageForPower(wep16) : dmgForWep[wep16],.owner=PLAYER1,.attackType=attTypeWep[wep16],.offense=offenseWep[wep16],.penetration=penetrationWep[wep16]};
    (void)dd;// ProjectileEffectImpact.dd = dd; // attach damage payload to the projectile instance TODO
    World.position[ball] = World.position[PLAYER1];
    V3 fwd = V3_Normalize(ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right));
    World.instances[ball].forward = fwd;
    World.velocity[ball] = (V3){0,0,0}; // clear any stale velocity before the impulse
    World.velocity[ball] = V3_AplusB(World.velocity[ball], V3_ScaleByF(fwd, shoveForce / vmax(World.mass[ball],0.0001f)));
}

void FirePlasma(int w){FireBeachball(w,plasmaShotForce,485);} void FireRailgun(int w){FireBeachball(w,railgunShotForce,484);} void FireMagpulse(int w){FireBeachball(w,magpulseShotForce,482);} void FireStungun(int w){FireBeachball(w,stungunShotForce,483);}
typedef void (*FireFn)(int); FireFn wepSpecialFire[16]={0,0,0,0,0,FireRapier,FirePipe,0,FireMagpulse,0,FirePlasma,FireRailgun,0,0,0,FireStungun};
void FireWeapon(int wep16,bool isSilent){if(wep16<0||wep16>15)return;World.invP1.noiseFinished=World.pauseRelativeTime+0.5;if(!isSilent&&wep16>=0&&wep16<16)play_wav(sounds[wepFireSound[wep16]],1.0f,World.position[PLAYER1],false);bool didHit=false;if(wepSpecialFire[wep16])wepSpecialFire[wep16](wep16);else{didHit=DidRayHit(wep16);if(didHit)HitScanFire(wep16);}if(wepSmokePrefab[wep16]){u16 smk=SpawnDynamicObject(wepSmokePrefab[wep16],-1);if(smk!=0xFFFF){World.position[smk]=wfx.reloadContainerPos;World.rotation[smk]=World.rotation[PLAYER1];flag_set(&World.instances[smk].entflags,EF_ACTIVE,true);World.instances[smk].tickFinished=World.pauseRelativeTime+1.0;}}
    World.fogFac += wepFogInc[wep16]; u16 wc = World.invP1.weaponCurrent;
    if (wepClass[wep16] == WC_ENERGY) {
        float setting = World.invP1.weaponEnergySetting[wc];
        if (World.invP1.overloadEnabled) World.invP1.currentEnergyWeaponHeat[wc] = 100.0f;
        else { World.invP1.currentEnergyWeaponHeat[wc] += setting; if (World.invP1.currentEnergyWeaponHeat[wc] > 100.0f) {World.invP1.currentEnergyWeaponHeat[wc] = 100.0f;} }
    }
    if (wepClass[wep16] != WC_MELEE) { // Take ammo/energy. Melee weapons consume neither and don't count towards shotsFired.
        if (wepClass[wep16] == WC_ENERGY) {
            if (World.invP1.overloadEnabled) { World.invP1.overloadEnabled = false; if (!Cheats.bottomless && !Cheats.redbull) { TakeEnergy(energyDrainOverloadForWeapon[wep16]); BiomonitorEnergyPulse(energyDrainOverloadForWeapon[wep16]); } }
            else { float takeEnerg = (World.invP1.weaponEnergySetting[wc]/100.0f) * (energyDrainHiForWeapon[wep16]-energyDrainLowForWeapon[wep16]); if (!Cheats.bottomless && !Cheats.redbull) { TakeEnergy(takeEnerg); BiomonitorEnergyPulse(takeEnerg); } }
        } else { if (World.invP1.wepLoadedWithAlternate[wc]) { if (!Cheats.bottomless) World.invP1.currentMagazineAmount2[wc]--; } else { if (!Cheats.bottomless) World.invP1.currentMagazineAmount[wc]--; } }
        World.shotsFired++;
    }
    Recoil(wep16);
    if (World.invP1.wepLoadedWithAlternate[wc] || World.invP1.overloadEnabled) { World.invP1.overloadEnabled = false; World.invP1.waitTilNextFire = World.pauseRelativeTime + delayBetweenShotsForWeapon2[wep16]; }
    else { World.invP1.waitTilNextFire = World.pauseRelativeTime + delayBetweenShotsForWeapon[wep16]; }
}

static bool pendingAttackWep16Valid = false; static int  pendingAttackWep16 = -1; void DropHeldItem(); void ResetHeldItem(); void AddItemToInventory(int index, int custIdx);
void StartNormalAttack(int wep16) { if ((wep16 < 0 || wep16 > 15) || (World.invP1.waitTilNextFire >= World.pauseRelativeTime) || (World.invP1.reloadFinished >= World.pauseRelativeTime)) return; pendingAttackWep16=wep16; pendingAttackWep16Valid=true; }
extern u16 editModeTestEntityDefinition;
void CheckAttackInput(void) {
    if(!Attack()){return;} if(World.Sys_UI.vmailActive) { World.Sys_UI.vmailActive=0; return;}
    if (Cheats.editMode/*TODO use submode of editMode instead for spamming physobjects for fun and testing physics*/){World.invP1.holdingObject = true; World.invP1.heldObjectIndex = editModeTestEntityDefinition;}
    if (World.invP1.holdingObject && !World.mouseClickHeldOverGUI) { if (!World.uiIsBlocking) { DropHeldItem(); return; } AddItemToInventory(World.invP1.heldObjectIndex,World.invP1.heldObjectCustIdx); ResetHeldItem(); return; }
    int w = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex);
    if (w == -1 || World.invP1.holdingObject || World.mouseClickHeldOverGUI){return; /*No weapon*/} StartNormalAttack(w);
}

void CheckUIStateAndAttack(void) {
    if (!pendingAttackWep16Valid) return;
    int wepdex = pendingAttackWep16; pendingAttackWep16Valid = false;
    if (World.uiIsBlocking || World.invP1.holdingObject || World.mouseClickHeldOverGUI || World.invP1.reloadFinished >= World.pauseRelativeTime || World.invP1.waitTilNextFire >= World.pauseRelativeTime || wepdex < 0 || wepdex > 15) return;
    World.invP1.justFired = World.pauseRelativeTime; if (wepClass[wepdex] == WC_MELEE) { FireWeapon(wepdex,false); return; }
    if (wepClass[wepdex] == WC_ENERGY) {
        u16 wc = World.invP1.weaponCurrent;
        if (World.invP1.energy > 0.0f || Cheats.bottomless || Cheats.redbull) {
            if(World.invP1.currentEnergyWeaponHeat[wc]>wfx.overheatedPercent && !Cheats.bottomless && !Cheats.redbull){/*TODO PlayUIOneShotSavable(238);*//*noammo*/ World.invP1.waitTilNextFire=World.pauseRelativeTime + 0.8f; CenterStatusPrint("%s",Sys_Text.stringTable[11]);} 
            else { FireWeapon(wepdex,false); }
        } else { CenterStatusPrint("%s", Sys_Text.stringTable[207]);/*not enough energy*/ }
        return;
    }
    u16 wc = World.invP1.weaponCurrent; bool alt = World.invP1.wepLoadedWithAlternate[wc];
    u16 amount = alt ? World.invP1.currentMagazineAmount2[wc] : World.invP1.currentMagazineAmount[wc];
    if (amount > 0 || Cheats.bottomless) { FireWeapon(wepdex, false); } else {/* TODO PlayUIOneShotSavable(238);*//*noammo*/ World.invP1.waitTilNextFire = World.pauseRelativeTime + 0.8f; }
}

void Unload(bool isSilent) {
    if (World.invP1.weaponIndex >= 0x8000 || (i32)World.invP1.weaponIndex < 0) return;
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); if (wep16 < 0 || wepClass[wep16] == WC_MELEE) return;
    u16 wc = World.invP1.weaponCurrent;
    if(World.invP1.wepLoadedWithAlternate[wc]){World.invP1.wepAmmoSecondary[wep16]+=World.invP1.currentMagazineAmount2[wc]; World.invP1.currentMagazineAmount2[wc]=0;}else{World.invP1.wepAmmo[wep16]+=World.invP1.currentMagazineAmount[wc]; World.invP1.currentMagazineAmount[wc]=0;}
    (void)isSilent; if (!isSilent) play_wav(sounds[260], 1.0f, (V3){0,0,0}, false); // wreload
}

void LoadPrimaryAmmoType(bool isSilent) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); u16 wc = World.invP1.weaponCurrent;
    if (!World.invP1.wepLoadedWithAlternate[wc]) {
        if (World.invP1.currentMagazineAmount[wc] == magazinePitchCountForWeapon[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[191]); return; }
        if (World.invP1.currentMagazineAmount[wc] == World.invP1.wepAmmo[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[535]); return; }
    }
    Unload(true); World.invP1.wepLoadedWithAlternate[wc] = false; 
    World.invP1.currentMagazineAmount[wc] = (World.invP1.wepAmmo[wep16] >= magazinePitchCountForWeapon[wep16]) ? magazinePitchCountForWeapon[wep16] : (u8)World.invP1.wepAmmo[wep16];
    World.invP1.wepAmmo[wep16] -= World.invP1.currentMagazineAmount[wc];
    (void)isSilent; if (!isSilent) play_wav(sounds[(wep16==0 || wep16==3) ? 248 : 260], 1.0f, (V3){0,0,0}, false); // wlocknload / wreload
    StartWeaponDip(reloadTime[wep16]); wfx.reloadContainerPos = wfx.reloadContainerHome;
}

void LoadSecondaryAmmoType(bool isSilent) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); u16 wc = World.invP1.weaponCurrent;
    if (World.invP1.wepLoadedWithAlternate[wc]) {
        if (World.invP1.currentMagazineAmount2[wc] == magazinePitchCountForWeapon2[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[191]); return; }
        if (World.invP1.currentMagazineAmount2[wc] == World.invP1.wepAmmoSecondary[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[535]); return; }
    }
    Unload(true); World.invP1.wepLoadedWithAlternate[wc] = true;
    World.invP1.currentMagazineAmount2[wc] = (World.invP1.wepAmmoSecondary[wep16] >= magazinePitchCountForWeapon2[wep16]) ? magazinePitchCountForWeapon2[wep16] : (u8)World.invP1.wepAmmoSecondary[wep16];
    World.invP1.wepAmmoSecondary[wep16] -= World.invP1.currentMagazineAmount2[wc];
    (void)isSilent;//if (!isSilent) PlayUIOneShotSavable((wep16==0 || wep16==3) ? 248 : 260); TODO
    StartWeaponDip(reloadTime[wep16]); wfx.reloadContainerPos = wfx.reloadContainerHome;
}

void ReloadSecret(bool isSilent) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); if (wep16 < 0) {return;}
    if (wepClass[wep16] == WC_MELEE) { CenterStatusPrint("%s", Sys_Text.stringTable[315]); return; }
    if (wepClass[wep16] == WC_ENERGY) { CenterStatusPrint("%s", Sys_Text.stringTable[538]); return; }
    u16 wc = World.invP1.weaponCurrent;
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
    if (World.invP1.reloadFinished >= World.pauseRelativeTime || !Reload() || (int)World.invP1.weaponCurrent < 0) return;
    if (Sys_Settings.QuickReloadWeapons) { ReloadSecret(false); return; }
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); if (wep16 < 0) return;
    u16 wc = World.invP1.weaponCurrent;
    bool alt = World.invP1.wepLoadedWithAlternate[wc];
    if (alt ? (World.invP1.currentMagazineAmount2[wc] <= 0 || World.invP1.wepAmmoSecondary[wep16] <= 0) : (World.invP1.currentMagazineAmount[wc] <= 0 || World.invP1.wepAmmo[wep16] <= 0)) ReloadSecret(false); else Unload(false);
}

void ActualChangeAmmoType(void) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); if (wep16 < 0) {return;}
    if (wepClass[wep16] == WC_MELEE) { CenterStatusPrint("%s", Sys_Text.stringTable[315]); return; }
    if (wepClass[wep16] == WC_ENERGY) {
        u16 wc = World.invP1.weaponCurrent;
        if (World.invP1.currentEnergyWeaponHeat[wc] > 25.0f) { CenterStatusPrint("%s", Sys_Text.stringTable[12]); return; }
        if (World.invP1.overloadEnabled) { CenterStatusPrint("%s", Sys_Text.stringTable[13]); World.invP1.overloadEnabled = false; } else { CenterStatusPrint("%s", Sys_Text.stringTable[17]); World.invP1.overloadEnabled = true; }
        return;
    }
    u16 wc = World.invP1.weaponCurrent;
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

void WeaponsUpdate(void) { HeatBleedOff(); if (World.fogFac > 255) {World.fogFac = 255;} UpdateWeaponReloadDip(); RotateViewWeapon(); Recoiling(); CheckAttackInput(); CheckUIStateAndAttack(); CheckReloadInput(); if (ChangeAmmoType()){ActualChangeAmmoType();} MeleeHitUpdate(); }
