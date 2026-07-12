// weapons.c - Weapon System
float delayBetweenShotsForWeapon[16]={1.0f,0.6f,0.5f,0.1f,0.8f,1.6f,0.65f,0.8f,0.6f,0.5f,1.2f,0.9f,0.5f,0.08f,1.1f,0.75f};
float delayBetweenShotsForWeapon2[16]={1.0f,4.5f,0.5f,0.15f,4.0f,1.6f,0.75f,0.8f,0.6f,0.5f,1.2f,0.9f,0.5f,0.08f,5.0f,0.75f};
float damagePerHitForWeapon[16]={75.0f,12.0f,15.0f,10.0f,18.0f,150.0f,15.0f,60.0f,45.0f,22.0f,50.0f,185.0f,6.0f,35.0f,6.0f,2.0f};
float damagePerHitForWeapon2[16]={160.0f,70.0f,5.0f,22.0f,108.0f,0.0f,0.0f,85.0f,80.0f,33.0f,350.0f,0.0f,0.0f,35.0f,36.0f,15.0f};
float damageOverloadForWeapon[16]={0.0f,115.0f,0.0f,0.0f,180.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,60.0f,0.0f};
float energyDrainLowForWeapon[16]={0.0f,3.0f,0.0f,0.0f,5.0f,0.0f,0.0f,0.0f,0.0f,0.0f,13.0f,0.0f,0.0f,0.0f,2.0f,3.0f};
float energyDrainHiForWeapon[16]={0.0f,15.0f,0.0f,0.0f,30.0f,0.0f,0.0f,0.0f,0.0f,0.0f,130.0f,0.0f,0.0f,0.0f,8.0f,30.0f};
float energyDrainOverloadForWeapon[16]={0.0f,50.0f,0.0f,0.0f,100.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,24.0f,0.0f};
float penetrationForWeapon[16]={50.0f,25.0f,6.0f,35.0f,35.0f,80.0f,40.0f,30.0f,100.0f,20.0f,0.0f,35.0f,0.0f,40.0f,25.0f,0.0f};
float penetrationForWeapon2[16]={70.0f,0.0f,0.0f,32.0f,0.0f,0.0f,0.0f,25.0f,120.0f,30.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
float offenseForWeapon[16]={4.0f,4.0f,2.0f,2.0f,6.0f,5.0f,3.0f,4.0f,4.0f,2.0f,3.0f,6.0f,2.0f,2.0f,3.0f,3.0f};
float offenseForWeapon2[16]={5.0f,0.0f,3.0f,3.0f,0.0f,0.0f,0.0f,5.0f,5.0f,3.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
u8 magazinePitchCountForWeapon[16]={10,0,15,60,0,0,0,12,25,20,0,12,20,50,0,0};
u8 magazinePitchCountForWeapon2[16]={8,0,15,60,0,0,0,12,10,20,0,0,0,100,0,0};
float reloadTime[16]={1.0f,0.8f,1.0f,1.2f,0.8f,0.8f,0.8f,1.3f,1.5f,0.8f,0.8f,1.0f,1.5f,2.0f,0.8f,0.8f};
float recoilForWeapon[16]={1.3f,0.0f,0.1f,0.2f,0.0f,0.0f,0.0f,1.2f,0.8f,0.5f,1.5f,1.0f,0.9f,0.7f,0.0f,0.1f};
float driftForWeapon[16]={5.0f,0.0f,15.0f,50.0f,0.0f,0.0f,0.0f,8.0f,3.0f,3.0f,3.0f,12.0f,10.0f,30.0f,0.0f,3.0f};
AttType attackTypeForWeapon[16]={Att_HitS,Att_Beam,Att_PjNd,Att_HitS,Att_Beam,Att_MlEg,Att_HitS,Att_HitS,Att_Magn,Att_HitS,Att_PjBm,Att_Ball,Att_HitS,Att_HitS,Att_Beam,Att_Trnq};
u16 wepFireSound[16]={251,239,240,243,245,246,254,249,250,255,257,259,262,263,264,265}; //(0)Assault (1)Blaster (2)Dartgun (3)Flechette (4)IonBeam (5)Rapier (6)Pipe (7)Magnum (8)Magpulse (9)Pistol (10)Plasma (11)Railgun (12)Riotgun (13)Skorpion (14)SparqBeam (15)Stungun
u16 wepBulletHolePrefab[16]={518,520,522,521,519,520,522,518,519,521,519,519,523,518,520,520};
i8  wepFogInc[16]={2,0,0,1,0,0,0,3,0,1,0,2,4,2,0,0};
u16 wepSmokePrefab[16]={506,0,0,509,0,0,0,507,0,510,0,511,512,513,0,0}; // 0 = no smoke for this weapon
typedef enum { WC_STD=0, WC_MELEE=1, WC_ENERGY=2 } WepClass;
WepClass wepClass[16]={WC_STD,WC_ENERGY,WC_STD,WC_STD,WC_ENERGY,WC_MELEE,WC_MELEE,WC_STD,WC_STD,WC_STD,WC_ENERGY,WC_STD,WC_STD,WC_STD,WC_ENERGY,WC_ENERGY};
float magpulseShotForce=2.2f, stungunShotForce=2.2f, railgunShotForce=5.0f, plasmaShotForce=1.5f;
typedef struct { float hitOffset,verticalOffset,fireDistance,hitscanDistance,meleescanDistance,overheatedPercent,wepYRot,targetY; V3 reloadContainerHome,reloadContainerPos,tempVec; Quaternion reloadContainerRot;
                 bool recoiling,pendingMeleeIsRapier,pendingMeleeSilent; RaycastHit tempHit; u16 tempHitEnt; double pendingMeleeFinished; i32 pendingMeleeWep16; u16 pendingMeleeTarget; u16 pendingMeleeHitSnd,pendingMeleeMissSnd,pendingMeleeFleshSnd; } WeaponFireCtx;
WeaponFireCtx wfx = { .verticalOffset=-0.2f,.fireDistance=200.0f,.hitscanDistance=200.0f,.meleescanDistance=3.2f, .overheatedPercent=80.0f,.reloadContainerHome={0},.pendingMeleeFinished=0.0 };
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

int Get16WeaponIndexFromConstIndex(int index) {
    switch (index) {
        case 36: return 0; // Mark3 Assault Rifle
        case 37: return 1; // ER-90 Blaster
        case 38: return 2; // SV-23 Dartgun
        case 39: return 3; // AM-27 Flechette
        case 40: return 4; // RW-45 Ion Beam
        case 41: return 5; // TS-04 Laser Rapier
        case 42: return 6; // Lead Pipe
        case 43: return 7; // Magnum 2100
        case 44: return 8; // SB-20 Magpulse
        case 45: return 9; // ML-41 Pistol
        case 46: return 10;// LG-XX Plasma Rifle
        case 47: return 11;// MM-76 Railgun
        case 48: return 12;// DC-05 Riotgun
        case 49: return 13;// RF-07 Skorpion
        case 50: return 14;// Sparq Beam
        case 51: return 15;// DH-07 Stungun
    }
    return -1;
}

INLINE bool CurrentWeaponUsesEnergy(void) { int i = World.invP1.weaponIndex; return i==37 || i==40 || i==46 || i==50 || i==51; }
INLINE bool WeaponsHaveAnyHeat(void) { if (Cheats.redbull) {return false;} for (int i=0;i<7;i++) {if (World.invP1.currentEnergyWeaponHeat[i] > 0.0f) {return true;}} return false; }
void HeatBleedOff(void) {
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

void Recoil(int wep16) {
    float strength = recoilForWeapon[wep16];
    if (strength <= 0.0f) return;
    if (World.instances[PLAYER1].health > 0.0f && World.invP1.fatigue > 80.0f) strength *= 2.0f;
    strength *= 0.25f;

    V3 jolt = { wfx.reloadContainerPos.x - strength*0.5f*random_range(-1.0f,1.0f),
                wfx.reloadContainerPos.y,
                wfx.reloadContainerHome.z - strength };
    if (jolt.x > 999.0f) jolt.x = 0.0f;
    if (jolt.y > 999.0f) jolt.y = 0.0f;
    if (jolt.z > 999.0f) jolt.z = 0.0f;
    wfx.reloadContainerPos = jolt;
    wfx.recoiling = true;
}

void Recoiling(void) {
    if (!wfx.recoiling) return;
    float x = wfx.reloadContainerPos.x, z = wfx.reloadContainerPos.z;
    float dt = (float)World.deltaTime;
    z += (wfx.reloadContainerHome.z - z) * dt;
    x += (wfx.reloadContainerHome.x - x) * dt;
    wfx.reloadContainerPos = (V3){x, wfx.reloadContainerPos.y, z};
}

// ---- Weapon dip (reload/swap "animation") ----------------------------------
static float reloadLerpValue = 0.0f;
static u8 lerpUp = 0; // 0 idle, 1 lerping up, 2 lerping down
static double lerpStartTime = 0.0;
void WeaponLerpGetTargetUp(void) { reloadLerpValue = (0.5f - (1.0f - reloadLerpValue)) / 0.5f; wfx.targetY = -1.0f * 0.66f * (1.0f - reloadLerpValue); if (wfx.targetY > wfx.reloadContainerHome.y){wfx.targetY = wfx.reloadContainerHome.y;} }
void WeaponLerpGetTargetDown(void) { reloadLerpValue = reloadLerpValue / 0.5f; wfx.targetY = wfx.reloadContainerHome.y - 0.66f; wfx.targetY *= reloadLerpValue; }
void CompleteWeaponChange(void) {
    if (World.invP1.weaponIndexPending == -1) return;
    World.invP1.weaponCurrent = (u8)World.invP1.weaponCurrentPending;
    if (CurrentWeaponUsesEnergy()) { /*HudHeatBleed(World.invP1.currentEnergyWeaponHeat[World.invP1.weaponCurrent]);*/ }
    World.invP1.weaponIndex = (u16)World.invP1.weaponIndexPending;
    World.invP1.weaponCurrentPending = World.invP1.weaponIndexPending = -1;
    int ind = World.invP1.weaponIndex;
    bool alt = (ind >= 0 && ind < 16) ? World.invP1.wepLoadedWithAlternate[ind] : false;
    (void)alt; /*SetAmmoIcons(ind, alt);*/ /*SetWepInfo(World.invP1.weaponIndex);*/
}

void StartWeaponDip(float delay) { if (delay < 0.0f) {delay = 0.0f;} World.invP1.reloadFinished = World.pauseRelativeTime + delay; lerpStartTime = World.pauseRelativeTime; }
void UpdateWeaponReloadDip(void) {
    int i = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex);
    if (i < 0 || i > 15) {i = 0;}
    if (World.invP1.reloadFinished > World.pauseRelativeTime) {
        double elapsed = World.pauseRelativeTime - lerpStartTime;
        reloadLerpValue = (float)(elapsed / (World.invP1.reloadFinished - lerpStartTime));
        if (reloadLerpValue >= 0.5f) { lerpUp = 1; WeaponLerpGetTargetUp(); CompleteWeaponChange(); }
        else { lerpUp = 2; WeaponLerpGetTargetDown(); }
        wfx.targetY = vclamp(wfx.targetY, -100.0f, 100.0f);
        wfx.reloadContainerPos = (V3){wfx.reloadContainerPos.x, wfx.targetY, wfx.reloadContainerPos.z};
    } else { lerpUp = 0; wfx.reloadContainerPos = (V3){wfx.reloadContainerPos.x, wfx.reloadContainerHome.y, wfx.reloadContainerPos.z}; }
}

void RotateViewWeapon(void) {
    if (World.inventoryMode) {
        float screenHalf = (float)Sys_Settings.ScreenWidth * 0.5f;
        float cursorX = wfx.tempVec.x; // engine-supplied cursor x, set by mouse-cursor system
        float distFromCenter = cursorX - screenHalf;
        float percentRotated = distFromCenter / screenHalf;
        wfx.wepYRot = percentRotated * 48.0f; // inventoryModeViewRotateMax
        wfx.reloadContainerRot = QuatEulerY(wfx.wepYRot);
    } else { wfx.reloadContainerRot = QUAT_IDENTITY; }
}

bool DidRayHit(int wep16) {
    wfx.tempHitEnt = 0xFFFF;
    V3 origin = World.position[PLAYER1];
    V3 dir = ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right);
    dir.x += random_range(-driftForWeapon[wep16],driftForWeapon[wep16]);
    dir.y += random_range(-driftForWeapon[wep16],driftForWeapon[wep16]);
    RaycastHit hit = Raycast(origin,dir,wfx.fireDistance,LMASK_PLAYER_ATTACK);
    if (hit.hit) { wfx.tempHitEnt = hit.hitInstanceIndex; return true; }
    return false;
}

void CreateStandardImpactMarks(int wep16) {
    if (!wfx.tempHit.hit) return;
    Entity* e = &World.instances[wfx.tempHit.hitInstanceIndex];
    if ((e->kinematic == false && (e->entflags & EF_RIGIDBODY)) || IdxIsDoor(e->index)) return; // Don't create bullet holes on objects that move, die, animate, or are doors.
    V3 pos = V3_AplusB(wfx.tempHit.point, V3_ScaleByF(wfx.tempHit.normal, 0.16f));
    u16 markInst = SpawnDynamicObject(wepBulletHolePrefab[wep16], -1); if (markInst == 0xFFFF) return;
    World.position[markInst] = pos;
    World.rotation[markInst] = QuatFromToRotation((V3){0,1,0}, V3_ScaleByF(wfx.tempHit.normal,-1.0f));
    World.rotation[markInst] = quat_multiply(World.rotation[markInst], QuatEulerZ((float)(int)random_range(0.0f,3.99f) * 90.0f));
}

void CreateStandardImpactEffects(void) { // TODO
//     u16 impactPrefab = (wfx.tempHitEnt != 0xFFFF) ? GetImpactPrefabForEntity(wfx.tempHitEnt) : 731; // generic small sparks
//     V3 pos = V3_AplusB(wfx.tempHit.point, V3_ScaleByF(wfx.tempHit.normal, wfx.hitOffset));
//     u16 fx = SpawnDynamicObject(impactPrefab,-1);
//     World.position[fx] = pos; World.rotation[fx] = QuatFromToRotation((V3){0,1,0}, wfx.tempHit.normal);
}

void CreateBeamImpactEffects(int wep16) {
    int impactConstdex = 731; // Cyan sparq
    if (wep16 == 1) impactConstdex = 739;      // Red laser (blaster)
    else if (wep16 == 4) impactConstdex = 740; // Yellow laser (ion)
    u16 fx = SpawnDynamicObject((u16)impactConstdex, -1);
    if (fx == 0xFFFF) return;
    World.position[fx] = wfx.tempHit.point;
    World.rotation[fx] = QuatFromToRotation((V3){0,1,0}, wfx.tempHit.normal);
}

void CreateBeamEffects(int wep16) {
    u16 laserPrefab = 405; // sparq
    if (wep16 == 1) laserPrefab = 406;      // blaster
    else if (wep16 == 4) laserPrefab = 407; // ion
    u16 beam = SpawnDynamicObject(laserPrefab, -1);
    if (beam == 0xFFFF) return;
    World.position[beam] = wfx.reloadContainerPos; // muzzle-relative start point, LaserDrawing.startPoint/endPoint equivalent handled by the beam entity's own update, using its position and a linked end-point field (not modeled here).
}

float DamageForPower(int wep16) {
    if (World.invP1.overloadEnabled) return damageOverloadForWeapon[wep16];
    float dmgMin = damagePerHitForWeapon[wep16], dmgMax = damagePerHitForWeapon2[wep16];
    float enerMin = energyDrainLowForWeapon[wep16], enerMax = energyDrainHiForWeapon[wep16];
    float setting = World.invP1.weaponEnergySetting[World.invP1.weaponCurrent];
    // Slope-of-slopes curve: interpolates damage/energy ratio across the energy setting, then scales by the interpolated energy drain itself. See design spreadsheet.
    return ((setting/100.0f)*((dmgMax/enerMax)-(dmgMin/enerMin)) + 3.0f) * ((setting/100.0f)*(enerMax-enerMin) + enerMin);
}

float TakeDamage(u16 self,DamageData dd);
void HitScanFire(int wep16) {
    DamageData dd = {0};
    dd.hitIdx = wfx.tempHitEnt;
    bool isBeam = (wep16==1 || wep16==4 || wep16==14);
    if (isBeam) CreateBeamImpactEffects(wep16);
    else { CreateStandardImpactEffects(); if(wep16==2 && World.invP1.wepLoadedWithAlternate[World.invP1.weaponCurrent]){dd.attackType=Att_Trnq;} } // Dartgun's alternate ammo tranquilizes instead of wounding.
    float tranq = -1.0f;
    bool hitIsNPC = (wfx.tempHitEnt != 0xFFFF) && IdxIsNPC(World.instances[wfx.tempHitEnt].index);
    dd.isOtherNPC = hitIsNPC;
    if (hitIsNPC && dd.attackType == Att_Trnq) { tranq = Tranquilize(wfx.tempHitEnt, 3.0f + random_range(0.0f,4.0f), false); }
    if (!hitIsNPC && wfx.tempHitEnt != 0xFFFF && IdxIsGeometry(World.instances[wfx.tempHitEnt].index)) { CreateStandardImpactMarks(wep16); }
    dd.hitpoint = wfx.tempHit.point;
    dd.attacknormal = ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right);
    if (World.invP1.wepLoadedWithAlternate[World.invP1.weaponCurrent]) {
        dd.damage = damagePerHitForWeapon2[wep16];
        dd.offense = offenseForWeapon2[wep16];
        dd.penetration = penetrationForWeapon2[wep16];
    } else {
        dd.damage = CurrentWeaponUsesEnergy() ? DamageForPower(wep16) : damagePerHitForWeapon[wep16];
        dd.offense = offenseForWeapon[wep16];
        dd.penetration = penetrationForWeapon[wep16];
    }
    if (dd.attackType != Att_Trnq) dd.attackType = attackTypeForWeapon[wep16];
    dd.owner = PLAYER1;
    dd.impactVelocity = (wep16==12) ? 120.0f : 80.0f; // riotgun kicks harder
    float dmgFinal = 0.0f;
    if (wfx.tempHitEnt != 0xFFFF && World.instances[wfx.tempHitEnt].health > 0.0f) {
        dd.damage *= 0.8f; // rebalancing factor
        dmgFinal = TakeDamage(wfx.tempHitEnt,dd);
        dd.impactVelocity += dd.damage;
//         if (!dd.isOtherNPC || wep16==12) { ApplyImpactForce(wfx.tempHitEnt, dd.impactVelocity, dd.attacknormal, dd.hitpoint); } TODO
        // if (hitIsNPC && !NPCAsleep(wfx.tempHitEnt)) Music_SetCombat(true); TODO
    }
    if (dmgFinal < 0.0f) dmgFinal = 0.0f;
    (void)dmgFinal; (void)tranq;// CreateTargetIDInstance(dmgFinal, wfx.tempHitEnt, tranq); TODO
    if (isBeam) CreateBeamEffects(wep16);
}

static void BiomonitorEnergyPulse(float take);
void MeleeHitUpdate(void) {
    if (wfx.pendingMeleeFinished <= 0.0 || World.pauseRelativeTime < wfx.pendingMeleeFinished) return;
    wfx.pendingMeleeFinished = 0.0;
    int wep16 = wfx.pendingMeleeWep16;
    u16 targ = wfx.pendingMeleeTarget;
    bool isRapier = wfx.pendingMeleeIsRapier;
    bool silent = wfx.pendingMeleeSilent;
    if (targ == PLAYER1) return; // can't hit self
    DamageData dd = {0};
    dd.hitIdx = targ;
    dd.isOtherNPC = IdxIsNPC(World.instances[targ].index);
    dd.attacknormal = ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right);
    dd.damage = damagePerHitForWeapon[wep16];
    dd.offense = offenseForWeapon[wep16];
    dd.penetration = penetrationForWeapon[wep16];
    dd.owner = PLAYER1;
    dd.attackType = Att_Melee;
    if (isRapier) { dd.attackType = Att_MlEg; if (World.invP1.energy < 4.0f) dd.damage = damagePerHitForWeapon[6] / 2.0f; /*half power on low energy*/}
    // uou.HitForce(dd); // knock physics objects around - hook up to physics engine TODO
    wfx.tempHitEnt = targ;
    CreateStandardImpactEffects();
    if (IdxIsGeometry(World.instances[targ].index)) CreateStandardImpactMarks(wep16);
    if (World.instances[targ].health <= 0.0f && !dd.isOtherNPC) {
        // Non-health target (crate/scenery): footstep+swing sound, no damage path.
        if (!silent) {
            u16 fstepSnd = GetFootstepTypeForPrefab(targ);
            (void)fstepSnd;//if (fstepSnd) { PlayWav(World.position[targ], fstepSnd, 1.0f); PlayWav(World.position[targ], wfx.pendingMeleeHitSnd, 0.65f); } TODO
            //else PlayWav(World.position[targ], wfx.pendingMeleeHitSnd, 1.0f);
            World.instances[PLAYER1].noiseFinished = World.pauseRelativeTime + 0.5;
        }
        return;
    }
    dd.impactVelocity = 80.0f + dd.damage;
//     if ((!dd.isOtherNPC || wep16==12) && (!isRapier || World.invP1.energy >= 4.0f)) { ApplyImpactForce(targ,dd.impactVelocity,dd.attacknormal,dd.hitpoint); } TODO
    float dmgFinal = TakeDamage(targ,dd);
    if (dmgFinal < 0.0f) {dmgFinal = 0.0f;}
    (void)dmgFinal; // CreateTargetIDInstance(dmgFinal, targ, -1.0f); TODO
    if (!silent) {
        World.instances[PLAYER1].noiseFinished = World.pauseRelativeTime + 0.5;
        BloodType bt = World.instances[targ].bloodType; (void)bt;//TODO
//         if (bt==BloodType_Red || bt==BloodType_Yellow || bt==BloodType_Green) PlayUIOneShotSavable(wfx.pendingMeleeFleshSnd); TODO
//         else if (isRapier && World.invP1.energy < 4.0f) PlayUIOneShotSavable(67);
//         else PlayUIOneShotSavable(wfx.pendingMeleeHitSnd);
    }
    if (isRapier) { TakeEnergy(3.666f); BiomonitorEnergyPulse(3.666f); } // 3 hits per energy tick
}

void FireMelee(int wep16, bool isRapier, bool silent, u16 hitSnd, u16 missSnd, u16 fleshSnd) {
    wfx.fireDistance = wfx.meleescanDistance;
    if (DidRayHit(wep16)) {
        wfx.fireDistance = wfx.hitscanDistance;
//         PlayAnim(PLAYER1,ANIM_ATTACK2); TODO
        wfx.pendingMeleeWep16 = wep16;
        wfx.pendingMeleeTarget = wfx.tempHitEnt;
        wfx.pendingMeleeIsRapier = isRapier;
        wfx.pendingMeleeSilent = silent;
        wfx.pendingMeleeHitSnd = hitSnd; wfx.pendingMeleeMissSnd = missSnd; wfx.pendingMeleeFleshSnd = fleshSnd;
        wfx.pendingMeleeFinished = World.pauseRelativeTime + (isRapier ? 0.28 : 0.15);
        return;
    }
    wfx.fireDistance = wfx.hitscanDistance;
    V3 lookDir = V3_Normalize(ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right));
    for (u16 i=INSTS_1ST_IDX; i<World.instCount; i++) {
        if (!(World.instances[i].entflags & EF_ACTIVE)) continue;
        if (World.instances[i].health <= 0.0f) continue;
        if (V3_Dist(World.position[i], World.position[PLAYER1]) >= wfx.meleescanDistance) continue;

        V3 toTarget = V3_Normalize(V3_AsubB(World.position[i], World.position[PLAYER1]));
        if (V3_dot(lookDir, toTarget) <= 0.666f) continue; // outside ~+-48deg cone

//         PlayAnim(PLAYER1, ANIM_ATTACK2); TODO
        wfx.pendingMeleeWep16 = wep16;
        wfx.pendingMeleeTarget = i;
        wfx.pendingMeleeIsRapier = isRapier;
        wfx.pendingMeleeSilent = silent;
        wfx.pendingMeleeHitSnd = hitSnd; wfx.pendingMeleeMissSnd = missSnd; wfx.pendingMeleeFleshSnd = fleshSnd;
        wfx.pendingMeleeFinished = World.pauseRelativeTime + (isRapier ? 0.28 : 0.15);
        return;
    }

    // Swing and a miss.
//     if (!silent) PlayUIOneShotSavable(missSnd); TODO
//     PlayAnim(PLAYER1, isRapier ? ANIM_ATTACK2 : ANIM_ATTACK1); TODO
}

void FireRapier(int wep16) { FireMelee(wep16, true,  false, 246, 247, 246); } // wlaserrapier_hit/swing
void FirePipe(int wep16)   { FireMelee(wep16, false, false, 253, 254, 252); } // wpipe_hit/swing/dmg
void FireBeachball(int wep16, float shoveForce, u16 prefabID) {
    // Acts like a beachball for NPC collisions, but a baseball for walls/floor (prevents corner-catching); handled by the projectile's own collider setup.
    u16 ball = SpawnDynamicObject(prefabID, 1);
    if (ball == 0xFFFF) return;
    DamageData dd = {0};
    dd.damage = CurrentWeaponUsesEnergy() ? DamageForPower(wep16) : damagePerHitForWeapon[wep16];
    dd.owner = PLAYER1;
    dd.attackType = attackTypeForWeapon[wep16];
    dd.offense = offenseForWeapon[wep16];
    dd.penetration = penetrationForWeapon[wep16];
    // ProjectileEffectImpact.dd = dd; // attach damage payload to the projectile instance TODO
    World.position[ball] = World.position[PLAYER1];
    V3 fwd = V3_Normalize(ScreenPointToRay(World.instances[PLAYER1].forward,World.instances[PLAYER1].right));
    World.instances[ball].forward = fwd;
    World.velocity[ball] = (V3){0,0,0}; // clear any stale velocity before the impulse
    World.velocity[ball] = V3_AplusB(World.velocity[ball], V3_ScaleByF(fwd, shoveForce / vmax(World.mass[ball],0.0001f)));
}

void FirePlasma(int wep16)   { FireBeachball(wep16, plasmaShotForce,   485); }
void FireRailgun(int wep16)  { FireBeachball(wep16, railgunShotForce,  484); }
void FireMagpulse(int wep16) { FireBeachball(wep16, magpulseShotForce, 482); }
void FireStungun(int wep16)  { FireBeachball(wep16, stungunShotForce,  483); }
typedef void (*FireFn)(int);
FireFn wepSpecialFire[16] = {0,0,0,0,0,FireRapier,FirePipe,0,FireMagpulse,0,FirePlasma,FireRailgun,0,0,0,FireStungun};
void FireWeapon(int wep16, bool isSilent) {
    if (wep16 < 0 || wep16 > 15) return;
    World.instances[PLAYER1].noiseFinished = World.pauseRelativeTime + 0.5;
    (void)isSilent;//if (!isSilent) PlayUIOneShotSavable(wepFireSound[wep16]); TODO
    bool didHit = false;
    if (wepSpecialFire[wep16]) wepSpecialFire[wep16](wep16);
    else didHit = DidRayHit(wep16);
    // ActivateInst(wepMuzzleFlashInst[wep16]); // muzzle-flash instance table, per-weapon
    if (wepSmokePrefab[wep16] && didHit) { u16 smk = SpawnDynamicObject(wepSmokePrefab[wep16], -1); if (smk != 0xFFFF) { World.position[smk] = wfx.reloadContainerPos; } }
    World.fogFac += wepFogInc[wep16];
    u16 wc = World.invP1.weaponCurrent;
    if (wepClass[wep16] == WC_ENERGY) {
        float setting = World.invP1.weaponEnergySetting[wc];
        if (World.invP1.overloadEnabled) World.invP1.currentEnergyWeaponHeat[wc] = 100.0f;
        else { World.invP1.currentEnergyWeaponHeat[wc] += setting; if (World.invP1.currentEnergyWeaponHeat[wc] > 100.0f) {World.invP1.currentEnergyWeaponHeat[wc] = 100.0f;} }
    }
    // Take ammo/energy. Melee weapons consume neither and don't count towards shotsFired.
    if (wepClass[wep16] != WC_MELEE) {
        if (wepClass[wep16] == WC_ENERGY) {
            if (World.invP1.overloadEnabled) {
                OverloadFired();
                if (!Cheats.bottomless && !Cheats.redbull) { TakeEnergy(energyDrainOverloadForWeapon[wep16]); BiomonitorEnergyPulse(energyDrainOverloadForWeapon[wep16]); }
            } else {
                float takeEnerg = (World.invP1.weaponEnergySetting[wc]/100.0f) * (energyDrainHiForWeapon[wep16]-energyDrainLowForWeapon[wep16]);
                if (!Cheats.bottomless && !Cheats.redbull) { TakeEnergy(takeEnerg); BiomonitorEnergyPulse(takeEnerg); }
            }
        } else {
            if (World.invP1.wepLoadedWithAlternate[wc]) { if (!Cheats.bottomless) World.invP1.currentMagazineAmount2[wc]--; }
            else { if (!Cheats.bottomless) World.invP1.currentMagazineAmount[wc]--; }
        }
        World.shotsFired++;
    }
    Recoil(wep16);
    if (World.invP1.wepLoadedWithAlternate[wc] || World.invP1.overloadEnabled) { World.invP1.overloadEnabled = false; World.invP1.waitTilNextFire = World.pauseRelativeTime + delayBetweenShotsForWeapon2[wep16]; }
    else { World.invP1.waitTilNextFire = World.pauseRelativeTime + delayBetweenShotsForWeapon[wep16]; }
}

static bool pendingAttackWep16Valid = false; static int  pendingAttackWep16 = -1;
void StartNormalAttack(int wep16) { if ((wep16 < 0 || wep16 > 15) || (World.invP1.waitTilNextFire >= World.pauseRelativeTime) || (World.invP1.reloadFinished >= World.pauseRelativeTime)) return; pendingAttackWep16=wep16; pendingAttackWep16Valid=true; }
void CheckAttackInput(void) {
    if (World.invP1.holdingObject && !World.mouseClickHeldOverGUI) { if (!World.uiIsBlocking) { /* DropHeldItem(); */ return; } /*AddItemToInventory(World.invP1.heldObjectIndex,World.invP1.heldObjectCustomIndex); ResetHeldItem(); TODO*/ return; }
    int wepdex = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex);
    if (wepdex == -1 || World.invP1.holdingObject || World.mouseClickHeldOverGUI) return; // No weapon.
    StartNormalAttack(wepdex);
}

void CheckUIStateAndAttack(void) {
    if (!pendingAttackWep16Valid) return;
    int wepdex = pendingAttackWep16; pendingAttackWep16Valid = false;
    if (World.uiIsBlocking || World.invP1.holdingObject || World.mouseClickHeldOverGUI || World.invP1.reloadFinished >= World.pauseRelativeTime || World.invP1.waitTilNextFire >= World.pauseRelativeTime || wepdex < 0 || wepdex > 15) return;
    World.invP1.justFired = World.pauseRelativeTime; if (wepClass[wepdex] == WC_MELEE) { FireWeapon(wepdex,false); return; }
    if (wepClass[wepdex] == WC_ENERGY) {
        u16 wc = World.invP1.weaponCurrent;
        if (World.invP1.energy > 0.0f || Cheats.bottomless || Cheats.redbull) {
            if (World.invP1.currentEnergyWeaponHeat[wc] > wfx.overheatedPercent && !Cheats.bottomless && !Cheats.redbull) {/*TODO PlayUIOneShotSavable(238);*//*noammo*/ World.invP1.waitTilNextFire = World.pauseRelativeTime + 0.8f; CenterStatusPrint("%s", Sys_Text.stringTable[11]); } 
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
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex);
    if (wep16 < 0 || wepClass[wep16] == WC_MELEE) return;

    u16 wc = World.invP1.weaponCurrent;
    if (World.invP1.wepLoadedWithAlternate[wc]) {
        World.invP1.wepAmmoSecondary[wep16] += World.invP1.currentMagazineAmount2[wc];
        World.invP1.currentMagazineAmount2[wc] = 0;
    } else {
        World.invP1.wepAmmo[wep16] += World.invP1.currentMagazineAmount[wc];
        World.invP1.currentMagazineAmount[wc] = 0;
    }
    (void)isSilent;//if (!isSilent) PlayUIOneShotSavable(260); TODO// wreload
}

void LoadPrimaryAmmoType(bool isSilent) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); u16 wc = World.invP1.weaponCurrent;
    if (!World.invP1.wepLoadedWithAlternate[wc]) {
        if (World.invP1.currentMagazineAmount[wc] == magazinePitchCountForWeapon[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[191]); return; }
        if (World.invP1.currentMagazineAmount[wc] == World.invP1.wepAmmo[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[535]); return; }
    }

    Unload(true);
    World.invP1.wepLoadedWithAlternate[wc] = false;
    World.invP1.currentMagazineAmount[wc] = (World.invP1.wepAmmo[wep16] >= magazinePitchCountForWeapon[wep16]) ? magazinePitchCountForWeapon[wep16] : (u8)World.invP1.wepAmmo[wep16];
    World.invP1.wepAmmo[wep16] -= World.invP1.currentMagazineAmount[wc];
    (void)isSilent;//if (!isSilent) PlayUIOneShotSavable((wep16==0 || wep16==3) ? 248 : 260); TODO // wlocknload / wreload
    StartWeaponDip(reloadTime[wep16]);
    wfx.reloadContainerPos = wfx.reloadContainerHome;
}

void LoadSecondaryAmmoType(bool isSilent) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); u16 wc = World.invP1.weaponCurrent;
    if (World.invP1.wepLoadedWithAlternate[wc]) {
        if (World.invP1.currentMagazineAmount2[wc] == magazinePitchCountForWeapon2[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[191]); return; }
        if (World.invP1.currentMagazineAmount2[wc] == World.invP1.wepAmmoSecondary[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[535]); return; }
    }

    Unload(true);
    World.invP1.wepLoadedWithAlternate[wc] = true;
    World.invP1.currentMagazineAmount2[wc] = (World.invP1.wepAmmoSecondary[wep16] >= magazinePitchCountForWeapon2[wep16]) ? magazinePitchCountForWeapon2[wep16] : (u8)World.invP1.wepAmmoSecondary[wep16];
    World.invP1.wepAmmoSecondary[wep16] -= World.invP1.currentMagazineAmount2[wc];
    (void)isSilent;//if (!isSilent) PlayUIOneShotSavable((wep16==0 || wep16==3) ? 248 : 260); TODO
    StartWeaponDip(reloadTime[wep16]);
    wfx.reloadContainerPos = wfx.reloadContainerHome;
}

void ReloadSecret(bool isSilent) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); if (wep16 < 0) {return;}
    if (wepClass[wep16] == WC_MELEE) { CenterStatusPrint("%s", Sys_Text.stringTable[315]); return; }
    if (wepClass[wep16] == WC_ENERGY) { CenterStatusPrint("%s", Sys_Text.stringTable[538]); return; }
    u16 wc = World.invP1.weaponCurrent;
    if (World.invP1.wepLoadedWithAlternate[wc]) {
        if (World.invP1.currentMagazineAmount2[wc] == magazinePitchCountForWeapon2[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[191]); return; }
        if (World.invP1.wepAmmoSecondary[wep16] <= 0) {
            if (World.invP1.wepAmmo[wep16] <= 0) { CenterStatusPrint("%s", Sys_Text.stringTable[305]); return; }
            CenterStatusPrint("%s", Sys_Text.stringTable[192]);
            LoadPrimaryAmmoType(isSilent);
            return;
        }
        LoadSecondaryAmmoType(isSilent);
    } else {
        if (World.invP1.currentMagazineAmount[wc] == magazinePitchCountForWeapon[wep16]) { CenterStatusPrint("%s", Sys_Text.stringTable[191]); return; }
        if (World.invP1.wepAmmo[wep16] <= 0) {
            if (World.invP1.wepAmmoSecondary[wep16] <= 0) { CenterStatusPrint("%s", Sys_Text.stringTable[305]); return; }
            CenterStatusPrint("%s", Sys_Text.stringTable[192]);
            LoadSecondaryAmmoType(isSilent);
            return;
        }
        LoadPrimaryAmmoType(isSilent);
    }
}

void CheckReloadInput(void) {
    if (World.invP1.reloadFinished >= World.pauseRelativeTime) return;
    // if (!GetInput_Reload()) return; TODO
    if (World.invP1.weaponCurrent < 0) return; // note: weaponCurrent is unsigned; guard kept for parity with original intent
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex);
    if (wep16 < 0) return;
    u16 wc = World.invP1.weaponCurrent;
    if (World.invP1.wepLoadedWithAlternate[wc]) {
        if (World.invP1.currentMagazineAmount2[wc] <= 0 || World.invP1.wepAmmoSecondary[wep16] <= 0) { ReloadSecret(false); } else { Unload(false); }
    } else {
        if (World.invP1.currentMagazineAmount[wc] <= 0 || World.invP1.wepAmmo[wep16] <= 0) { ReloadSecret(false); } else { Unload(false); }
    }
}

void ActualChangeAmmoType(void) {
    int wep16 = Get16WeaponIndexFromConstIndex(World.invP1.weaponIndex); if (wep16 < 0) {return;}
    if (wepClass[wep16] == WC_MELEE) { CenterStatusPrint("%s", Sys_Text.stringTable[315]); return; }
    if (wepClass[wep16] == WC_ENERGY) { /*OverloadButtonAction();*/ /*toggles overload on whichever hand's button is active*/ return; }
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

void CheckAmmoChangeInput(void) { if (World.invP1.reloadFinished >= World.pauseRelativeTime) return; if (ChangeAmmoType()){ActualChangeAmmoType();} }
void WeaponsUpdate(void) { HeatBleedOff(); if (World.fogFac > 255) {World.fogFac = 255;} UpdateWeaponReloadDip(); RotateViewWeapon(); Recoiling(); CheckAttackInput(); CheckUIStateAndAttack(); CheckReloadInput(); CheckAmmoChangeInput(); MeleeHitUpdate(); }
