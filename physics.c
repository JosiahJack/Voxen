// physics.c - The Jack Physics Engine, By W. Josiah Jack MIT-0 -- full rigidbody 3D with torque for sphere, box, capsule, convex mesh dynamic objects and same set plus arbitrary trisoup mesh colliders for statics.
#include "common.h"
u16 cellLists[WORLDX*WORLDX][128],cellCounts[WORLDX*WORLDX];
static const float PHY_EPSILON=0.0001f,PHY_NEARNUFF=0.001f,MAX_SPEED=17.0f/*m/s fastest is railgun given 5.0 impulse w/ 0.3 mass=5.0/0.3 */,MAX_STEP_SIZE=(0.12f / MAX_SPEED),MAX_ANGULAR_SPEED=8.0f/*arbitrary*/,MANIFOLD_TIE_MARGIN=0.008f,MANIFOLD_ALIGN_THRESHOLD=0.8f;
static const float WALK_SPEED=5.7f,SPRINT_SPEED=17.0f,PLAYER_MAX_CYBER_SPEED=10.0f,SPRINT_SPEED_FATIGUED=10.0f,CROUCH_SPEED=2.5f,PLAYER_MAX_PRONE_SPEED=1.6f,PLAYER_BOOSTER_SPEED_BOOST=1.2f,PLAYER_CROUCH_RATIO=0.63f,PLAYER_PRONE_RATIO=0.32f;
enum { MANIFOLD_MAX=4, CVXMSH_HULL_CACHE=1024, EPA_MAX_FACES=64, EPA_MAX_VERTS=128, EPA_MAX_EDGES=EPA_MAX_FACES*3, GJK_ITER=32, EPA_ITER=16, SOLVER_ITER_GLOBAL=32, MAX_GLOBAL_CONTACTS=8192 };
typedef struct { V3 v[4];/*Minkowski difference verts (wA - wB)*/   V3 wA[4],wB[4];/*Cached support points from Shape A,B*/ i32 n;/*Vertex count*/ } Simplex3D;
typedef struct { V3 point; float pen; } ManifoldPt; typedef struct { V3 normal; ManifoldPt p[MANIFOLD_MAX]; i32 n; float maxPen; } Manifold;
typedef struct { u16 a,b; Manifold m; V3 rA[MANIFOLD_MAX],rB[MANIFOLD_MAX]; float targetVn[MANIFOLD_MAX],accumN[MANIFOLD_MAX],accumT[MANIFOLD_MAX],invSumN[MANIFOLD_MAX]; float Ra[3][3],Rb[3][3],Ka[3][3],Kb[3][3]; float invMassA,invMassB; bool bStatic,canRotateA,canRotateB; } SolverContact;
SolverContact gContacts[MAX_GLOBAL_CONTACTS]; u32 gContactCount;
float posBudget[INSTANCE_COUNT]; // Remaining |delta pos| entity may receive this substep; resets every substep in Physics().
u16 dynamicEntities[512],dynamicEntityCount;
bool PhysIsAsleep(u16 i) { return World.physSleep[i] != 0; } // exposed for showPhys debug coloring
INLINE bool AnimWaking(u16 j) {
    if (j == PLAYER1) return false;
    u16 an = World.instances[j].animationNum;
    if (!((an == 0 || an == 1 || (an >= 4 && an <= 20) || (an >= 43 && an <= 45) || an == 47 || an == 48) && an < MAX_ANIMS && World.instances[j].clip < MAX_ANIMCLIPS)) return false;
    u8 fr = modelAnimationClips[an][World.instances[j].clip].framerate;
    return fr > 0 && (World.current_time - World.instances[j].animFinished) * (double)fr < 1.0;
}
// Trigger System
void AddForce(u16 i, V3 f, bool imp); void AddAccessCardToInventory(int index); void UseTargets(u16 activator, const char* targetname); void DeleteInstance(u16 i); void TakeEnergy(float take);
void trigger_cyberpush_touch(u16 self, u16 other) { if (World.diffCyb < 1) {return;} AddForce(other,V3_ScaleByF(World.instances[self].direction,World.instances[self].force * (float)World.deltaTime),false); World.Sys_Music.cyberTube = true; }
void prop_cyber_exit(u16 other) { if (other != PLAYER1) {return;} UIExitCyberspace(); }
void CyberDataFragmentOnTriggerEnter(u16 self, u16 other) { Entity* e = &World.instances[self]; if (other != PLAYER1) {return;} CenterStatusPrint("%s",Sys_Text.stringTable[(u16)e->textIndex]); }
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
        case 448/*item_cyber_data*/: World.invP1.hasNewData = true; if (vers >= 0 && vers < LOGCNT) {World.invP1.hasLog[vers] = true;} play_wav(sounds[87],sfxVol,(V3){0.0f,0.0f,0.0f},false); CenterStatusPrint("%s",Sys_Text.stringTable[457]); return true; 
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
void CyberSwitchOnTriggerEnter(u16 self, u16 other) { Entity* e = &World.instances[self]; if (e->active || other != PLAYER1) {return;} CenterStatusPrint("%s",Sys_Text.stringTable[(u16)e->textIndex]); e->active = true; UseTargets(other,e->target); }
// TeleportTouch
static bool TeleportTouch_initialized;
void TeleportTouchInitAfterLoad(u16 self){Entity* e=&World.instances[self]; if(!TeleportTouch_initialized){for(u8 i=0;i<8;++i){World.TeleportTouch_allTeleportTouches[i]=U16_MAX;} TeleportTouch_initialized=true;} if(e->teleportID >= 8){DeleteInstance(self); return;} World.TeleportTouch_allTeleportTouches[e->teleportID]=self;}
void TeleportTouchOnTriggerEnter(u16 self, u16 other) {
    Entity* e = &World.instances[self];
    Entity* player = &World.instances[PLAYER1];
    if (!e->touchEnabled || other != PLAYER1) return;
    if (player->health <= 0.0f || e->justUsed >= World.pauseRelativeTime) return;
    u16 dest = e->targetDestinationID < 8 ? World.TeleportTouch_allTeleportTouches[e->targetDestinationID] : U16_MAX;
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
// Physics System
INLINE void SetPosition(u16 i, V3 newpos) { float d=V3_Dist(World.position[i],newpos); if(d < PHY_NEARNUFF){return;} float allowed=vmin(d,posBudget[i]); if(allowed < PHY_NEARNUFF){return;} V3 dir=V3_Normalize(V3_AsubB(newpos,World.position[i])); World.position[i]=V3_AplusB(World.position[i],V3_ScaleByF(dir,allowed)); flag_set(&World.instances[i].entflags,EF_MOVING,true); posBudget[i] -= allowed; }
INLINE Manifold OverlapToManifold(Overlap r) { Manifold m={0}; if (r.hit && r.pen > PHY_EPSILON) { m.normal = r.normal; m.n = 1; m.p[0] = (ManifoldPt){r.point, r.pen}; m.maxPen = r.pen; } return m; }
INLINE Overlap SphSph(V3 a, float ar, V3 b, float br) { V3 dt=V3_AsubB(a,b); float d2=V3_dot(dt,dt),rs=ar+br; float h=(d2<rs*rs); float d=vsqrtf(vmax(d2,0.0f)); float m=(d<PHY_EPSILON); V3 n=V3_AplusB(V3_ScaleByF(dt,(1.0f/vmax(d,PHY_EPSILON))*(1.0f-m)),V3_ScaleByF((V3){0,1,0},m)); V3 point=V3_AplusB(b,V3_ScaleByF(n,br)); return (Overlap){(bool)h,point,n,(rs-d)*h}; }
INLINE Overlap SphCap(ShapeSphere s, ShapeCapsule c) { V3 seg=V3_AsubB(c.tip,c.base); float l=V3_dot(seg,seg); float m=(l < PHY_EPSILON); V3 b=V3_AplusB(c.base, V3_ScaleByF(seg,vclamp(V3_dot(V3_AsubB(s.ctr, c.base),seg) / vmax(l, PHY_EPSILON), 0.0f, 1.0f) * (1.0f - m))); b = V3_AplusB(V3_ScaleByF(b,1.0f - m),V3_ScaleByF(c.base,m)); return SphSph(s.ctr,s.rad,b,c.rad); }
INLINE Overlap CapCap(ShapeCapsule a, ShapeCapsule b) {
    Overlap r={0}; float sc,tc,distSq, radSum=a.rad + b.rad; V3 d1 = V3_AsubB(a.tip,a.base), d2=V3_AsubB(b.tip,b.base), vr=V3_AsubB(a.base,b.base);
    float qa=V3_dot(d1,d1), e=V3_dot(d2,d2), f=V3_dot(d2,vr);
    if(qa < PHY_EPSILON && e < PHY_EPSILON){sc=tc=0.0f;}
    else if(qa < PHY_EPSILON){sc=0.0f; tc=vclamp(f/e,0.0f,1.0f);}
    else {
        float c=V3_dot(d1,vr);
        if(e < PHY_EPSILON){tc=0.0f; sc=vclamp(-c/qa,0.0f,1.0f);}
        else {
            float qb=V3_dot(d1,d2),denom=qa*e - qb*qb; sc=(denom > PHY_EPSILON) ? vclamp((qb*f - c*e)/denom,0.0f,1.0f) : 0.0f; tc=(qb*sc + f)/e;
            if(tc < 0.0f){tc=0.0f; sc=vclamp(-c/qa,0.0f,1.0f);}else if(tc > 1.0f){tc=1.0f; sc=vclamp((qb-c)/qa,0.0f,1.0f);}
        }
    }
    V3 ptA=V3_AplusB(a.base,V3_ScaleByF(d1,sc)), ptB=V3_AplusB(b.base,V3_ScaleByF(d2,tc)), diff=V3_AsubB(ptA,ptB); distSq=V3_dot(diff,diff); if(distSq >= radSum * radSum) return r;
    float dist=vsqrtf(vmax(distSq,0.0f)); r.pen=radSum - dist; r.hit=true; r.normal=(dist < PHY_EPSILON) ? (V3){0,1,0} : V3_ScaleByF(diff,1.0f/dist); r.point = V3_AplusB(ptB,V3_ScaleByF(r.normal,b.rad)); return r;
}

INLINE Overlap SphBoxAxes(V3 ctr, float rad, V3 boxCtr, V3 ax, V3 ay, V3 az, V3 hExt) {
    Overlap r={0}; V3 d = V3_AsubB(ctr,boxCtr); float lx = V3_dot(d,ax), ly = V3_dot(d,ay), lz = V3_dot(d,az);
    V3 localClosest = V3_AplusB(V3_AplusB(V3_ScaleByF(ax,vclamp(lx,-hExt.x,hExt.x)), V3_ScaleByF(ay,vclamp(ly,-hExt.y,hExt.y))), V3_ScaleByF(az,vclamp(lz,-hExt.z,hExt.z))); V3 delta = V3_AsubB(d,localClosest);
    float distSq = V3_dot(delta,delta);
    if (distSq >= rad * rad) return r;
    r.hit = true; float dist = vsqrtf(vmax(distSq, 0.0f));
    if (dist > PHY_EPSILON) { r.normal = V3_ScaleByF(delta, 1.0f/dist); r.pen=rad - dist; }
    else {
        float mind=hExt.x - vabs(lx), dy=hExt.y - vabs(ly), dz=hExt.z - vabs(lz);
        V3 nAx = V3_ScaleByF(ax,lx > 0.0f ? 1.0f : -1.0f);
        if(dy < mind){mind=dy; nAx = V3_ScaleByF(ay, ly > 0.0f ? 1.0f : -1.0f);}
        if(dz < mind){mind=dz; nAx = V3_ScaleByF(az, lz > 0.0f ? 1.0f : -1.0f);}
        r.normal = nAx; r.pen = rad + mind;
    }
    r.point = V3_AsubB(ctr, V3_ScaleByF(r.normal, rad - r.pen));
    return r;
}

INLINE Overlap SphBox(V3 ctr, float rad, ShapeBox box) { V3 ax=quat_rot_v3(box.rot,(V3){1,0,0}), ay=quat_rot_v3(box.rot,(V3){0,1,0}), az=quat_rot_v3(box.rot,(V3){0,0,1}); return SphBoxAxes(ctr, rad, box.ctr, ax, ay, az, box.hExt); }
INLINE Overlap CapBox(ShapeCapsule c,ShapeBox b){ 
    V3 ax=quat_rot_v3(b.rot,(V3){1,0,0}), ay=quat_rot_v3(b.rot,(V3){0,1,0}), az=quat_rot_v3(b.rot,(V3){0,0,1}); V3 d=V3_AsubB(c.tip,c.base); 
    Overlap best=SphBoxAxes(c.base,c.rad,b.ctr,ax,ay,az,b.hExt), r=SphBoxAxes(c.tip,c.rad,b.ctr,ax,ay,az,b.hExt); if(r.pen>best.pen){best=r;}
    if(V3_dot(d,d)>PHY_EPSILON*PHY_EPSILON) { for(int k=1;k<8;k++){ r=SphBoxAxes(V3_AplusB(c.base,V3_ScaleByF(d,k*.125f)),c.rad,b.ctr,ax,ay,az,b.hExt); if(r.pen>best.pen){best=r;} } }
    return best; 
}

static const u32 CollisionMaskTable[32] = {
    [0]  = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip|L_CorpseSearchable, // L_Default
    [1]  = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip,                             // L_TransparentFX
    [9]  = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_Clip,                                         // L_Geometry
    [10] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_NPCTrigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_NPCClip|L_Clip,      // L_NPC
    [11] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip|L_CorpseSearchable,           // L_PlayerBullets
    [12] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PhysObjects|L_PlayerTriggerOnly|L_Trigger|L_Door|L_Player2|L_NPCBullet|L_Clip,                                                // L_Player
    [13] = L_Default|L_Geometry|L_PlayerBullets|L_PhysObjects|L_Door|L_NPCBullet|L_Clip,                                                                                              // L_Corpse
    [14] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Door|L_InterDebris|L_NPCBullet|L_Clip,                                        // L_PhysObjects
    [16] = L_Player|L_Player2,                                                                                                                                                        // L_PlayerTriggerOnly
    [17] = L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Door|L_InterDebris|L_Clip,                                                                             // L_Trigger
    [18] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip,                    // L_Door
    [19] = L_Default|L_Geometry|L_NPC|L_PlayerBullets|L_PhysObjects|L_Trigger|L_Door|L_NPCBullet|L_Clip,                                                                              // L_InterDebris
    [20] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_PlayerTriggerOnly|L_Trigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_Clip,         // L_Player2
    [23] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_NPCTrigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_NPCClip|L_Clip,      // L_NPCTrigger (Copy of L_NPC)
    [24] = L_Default|L_TransparentFX|L_Geometry|L_PlayerBullets|L_Player|L_Corpse|L_PhysObjects|L_Door|L_InterDebris|L_Player2|L_Clip|L_CorpseSearchable,                             // L_NPCBullet
    [26] = L_Player|L_Player2|L_NPC,                                                                                                                                                  // L_Clip
    [25] = L_Default|L_TransparentFX|L_Geometry|L_NPC|L_PlayerBullets|L_Player|L_PhysObjects|L_Trigger|L_NPCTrigger|L_Door|L_InterDebris|L_Player2|L_NPCBullet|L_NPCClip|L_Clip,      // L_NPCClip (Copy of L_NPC)
    [29] = L_Default|L_PlayerBullets,                                                                                                                                                 // L_CorpseSearchable
};

u32 GetCollisionMask(u32 layer) { u32 ctz = __builtin_ctz(layer | 1); u32 valid = (ctz < 32); return ((layer == L_NPCTrigger) | (layer == L_NPCClip)) ? L_NPC : (CollisionMaskTable[ctz * valid] * valid); }
ShapeCapsule Entity_GetCap(u16 i) {
    float scaleMax = vmax(World.scale[i].x,vmax(World.scale[i].y,World.scale[i].z));
    float r = World.colliderSize[i].x * scaleMax; float hi = vmax(0.0f, (World.colliderSize[i].y * 0.5f * scaleMax) - r); V3 wc,axis;
    if (i == PLAYER1 || World.layer[i] == L_NPC) { wc = V3_AplusB(World.position[i], World.colliderCenter[i]); axis = (V3){0.0f,1.0f,0.0f};/*Player+NPC remain strictly upright*/ }
    else { wc = V3_AplusB(World.position[i], quat_rot_v3(World.rotation[i], World.colliderCenter[i])); axis = (World.colliderSize[i].z < 0.5f) ? quat_rot_v3(World.rotation[i], (V3){1,0,0}) : (World.colliderSize[i].z < 1.5f) ? quat_rot_v3(World.rotation[i], (V3){0,1,0}) : quat_rot_v3(World.rotation[i], (V3){0,0,1}); }
    return (ShapeCapsule){.tip=V3_AplusB(wc,V3_ScaleByF(axis,hi)),.base=V3_AsubB(wc,V3_ScaleByF(axis,hi)),.rad=r};
}

ShapeBox Entity_GetBox(u16 i) { return (ShapeBox){.ctr=V3_AplusB(World.position[i],quat_rot_v3(World.rotation[i],World.colliderCenter[i])),.hExt=(V3){World.colliderSize[i].x*0.5f * World.scale[i].x,World.colliderSize[i].y*0.5f * World.scale[i].y,World.colliderSize[i].z*0.5f * World.scale[i].z},.rot=World.rotation[i]}; }
ShapeSphere Entity_GetSph(u16 i) { return (ShapeSphere){.ctr=V3_AplusB(World.position[i],quat_rot_v3(World.rotation[i],World.colliderCenter[i])),.rad = World.colliderSize[i].x * vmax(World.scale[i].x,vmax(World.scale[i].y,World.scale[i].z))}; }
Quaternion quat_from_axis_angle(V3 axis, float angle) { float half = angle * 0.5f; float s = vsinf(half); return (Quaternion){axis.x*s,axis.y*s,axis.z*s,vcosf(half)}; }
Quaternion quat_normalize(Quaternion q) { float l = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w; float m = (l < PHY_EPSILON); float inv=vinvsqtf(vmax(l,PHY_EPSILON)); q.x*=inv; q.y*=inv; q.z*=inv; q.w*=inv; q.x=q.x*(1.0f - m); q.y=q.y*(1.0f - m); q.z=q.z*(1.0f - m); q.w=q.w*(1.0f - m) + 1.0f*m; return q; }
INLINE V3 MeshVert(u16 m, u32 i) { const float* p = physPos[m] + i * 3; return (V3){p[0],p[1],p[2]}; }
void ComputeConvexMeshInertiaTensor(u16 i) {
    World.mass[i] *= 10.0f; // Intentional global mass scaling.  Tunes all masses vs Unity engine version of Citadel, gives better responses and robustness to small items.
    u16 mi = World.instances[i].colMeshIndex; World.invTnsrValid[i]=false; if (mi >= MAX_MDLS || !modelTriangleCounts[mi] || !modelVertexCounts[mi]) {return;}
    float acc[6]={0}; float cm[3]={0}; float volAcc=0.0f; u32 triCount = modelTriangleCounts[mi];
    for (u32 ti=0;ti<triCount;++ti) { // Accumulates without dividing by 6 for each saving that for the end for performance.
        u32 i0 = modelTriangles[mi][ti*3+0], i1 = modelTriangles[mi][ti*3+1], i2 = modelTriangles[mi][ti*3+2];
        V3 v0=MeshVert(mi,i0), v1=MeshVert(mi,i1), v2=MeshVert(mi,i2);
        float det = V3_dot(v0,V3_Cross(v1,v2)); volAcc += det; // Signed 6x tetrahedron volume via scalar triple product
        cm[0] += det*(v0.x + v1.x + v2.x); cm[1] += det*(v0.y + v1.y + v2.y); cm[2] += det*(v0.z + v1.z + v2.z);
        acc[0] += det*(v0.x*v0.x + v0.x*v1.x + v1.x*v1.x + v0.x*v2.x + v1.x*v2.x + v2.x*v2.x); // Analytic polynomial integration over volume
        acc[1] += det*(v0.y*v0.y + v0.y*v1.y + v1.y*v1.y + v0.y*v2.y + v1.y*v2.y + v2.y*v2.y); // (Mirtich / Eberly polyhedron mass properties technique)
        acc[2] += det*(v0.z*v0.z + v0.z*v1.z + v1.z*v1.z + v0.z*v2.z + v1.z*v2.z + v2.z*v2.z);
        acc[3] += det*(2.0f*(v0.x*v0.y + v1.x*v1.y + v2.x*v2.y) + v0.x*v1.y + v1.x*v0.y + v0.x*v2.y + v2.x*v0.y + v1.x*v2.y + v2.x*v1.y); // Multiplier 2.0f handles
        acc[4] += det*(2.0f*(v0.x*v0.z + v1.x*v1.z + v2.x*v2.z) + v0.x*v1.z + v1.x*v0.z + v0.x*v2.z + v2.x*v0.z + v1.x*v2.z + v2.x*v1.z); // the algebraic cross-term
        acc[5] += det*(2.0f*(v0.y*v0.z + v1.y*v1.z + v2.y*v2.z) + v0.y*v1.z + v1.y*v0.z + v0.y*v2.z + v2.y*v0.z + v1.y*v2.z + v2.y*v1.z); // polynomial expansion
    }
    if (vabs(volAcc) < PHY_EPSILON) return;
    float sx = World.scale[i].x, sy = World.scale[i].y, sz = World.scale[i].z; float sd = World.mass[i] / (volAcc * 10.0f/*6/60*/), so = World.mass[i] / (volAcc * 20.0f/*6/120*/); // Diagonals=6/60, off-diagonals=6/120, since those have the 2.0f multiplied above from the x^2 expansion resulting in 2xy terms.
    float cx = cm[0] / (4.0f * volAcc), cy = cm[1] / (4.0f * volAcc), cz = cm[2] / (4.0f * volAcc); float scx = cx * sx, scy = cy * sy, scz = cz * sz, m=World.mass[i]; // Center of mass denominator 24 reduced by 6/24 = 1/4
    float Ixx = sd*(acc[1]*sy*sy + acc[2]*sz*sz) - m*(scy*scy + scz*scz); float Iyy = sd*(acc[0]*sx*sx + acc[2]*sz*sz) - m*(scx*scx + scz*scz); float Izz = sd*(acc[0]*sx*sx + acc[1]*sy*sy) - m*(scx*scx + scy*scy);
    float Ixy = -(so*acc[3]*sx*sy - m*scx*scy); float Ixz = -(so*acc[4]*sx*sz - m*scx*scz); float Iyz = -(so*acc[5]*sy*sz - m*scy*scz); // Parallel Axis Theorem: shifts rotation center from origin (0,0,0) to center of mass
    float r = modelBounds[mi] * vmax(vmax(sx,sy),sz); float mn = 0.04f * m * r * r;
    Ixx = vmax(Ixx,mn); Iyy = vmax(Iyy,mn); Izz = vmax(Izz,mn); // Clamp diagonal inertia to a minimum floor of 10% of the I=2/5ths*mr^2=0.4mr^2 hence 0.04 of spherical inertia to avoid NaN's.
    float *IT=World.inertiaTensor[i]; IT[0]=Ixx; IT[1]=Iyy; IT[2]=Izz; IT[3]=Ixy; IT[4]=Ixz; IT[5]=Iyz;
    float det = Ixx*(Iyy*Izz - Iyz*Iyz) - Ixy*(Ixy*Izz - Ixz*Iyz) + Ixz*(Ixy*Iyz - Iyy*Ixz); if (vabs(det) < PHY_EPSILON) return;
    float invDet = 1.0f / det, *iI=World.invInertiaTensor[i]; // Applies Cramer's Rule to invert for actual use.  [ 0  3  4 ]  (0=Ixx, 1=Iyy, 2=Izz) (3=Ixy, 4=Ixz, 5=Iyz)
    iI[0]=(Iyy*Izz - Iyz*Iyz)*invDet; iI[1]=(Ixx*Izz - Ixz*Ixz)*invDet; iI[2]=(Ixx*Iyy - Ixy*Ixy)*invDet;      // [ 3  1  5 ]  Tensor is symmetric only 6 unique elements needed
    iI[3]=(Ixz*Iyz - Ixy*Izz)*invDet; iI[4]=(Ixy*Iyz - Iyy*Ixz)*invDet; iI[5]=(Ixy*Ixz - Ixx*Iyz)*invDet;      // [ 4  5  2 ]
    World.invTnsrValid[i]=true;
}



INLINE Manifold BoxBox(ShapeBox a, ShapeBox b) {
    Manifold m={0}; V3 aAxes[3], bAxes[3]; aAxes[0]=quat_rot_v3(a.rot,(V3){1,0,0}); aAxes[1]=quat_rot_v3(a.rot,(V3){0,1,0}); aAxes[2]=quat_rot_v3(a.rot,(V3){0,0,1});
    bAxes[0]=quat_rot_v3(b.rot,(V3){1,0,0}); bAxes[1]=quat_rot_v3(b.rot,(V3){0,1,0}); bAxes[2]=quat_rot_v3(b.rot,(V3){0,0,1});
    float aExt[3] = { a.hExt.x, a.hExt.y, a.hExt.z }; float bExt[3] = { b.hExt.x, b.hExt.y, b.hExt.z }; V3 T = V3_AsubB(b.ctr,a.ctr); float R[3][3],AbsR[3][3];
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) { R[i][j]=V3_dot(aAxes[i],bAxes[j]); AbsR[i][j]=vabs(R[i][j])+1e-6f; }
    float minOverlap=1e9f; int bestAxis=-1; bool flipNormal=false;
    for (int i=0;i<3;i++) {
        float ra=aExt[i], rb=bExt[0]*AbsR[i][0]+bExt[1]*AbsR[i][1]+bExt[2]*AbsR[i][2];
        float t=vabs(V3_dot(T,aAxes[i]));
        if(t>ra+rb) return m;
        float ov=(ra+rb)-t;
        if(ov<minOverlap-MANIFOLD_TIE_MARGIN){minOverlap=ov; bestAxis=i; flipNormal=(V3_dot(T,aAxes[i])<0.f);}
    }
    for (int i=0;i<3;i++) {
        float ra=aExt[0]*AbsR[0][i]+aExt[1]*AbsR[1][i]+aExt[2]*AbsR[2][i], rb=bExt[i];
        float t=vabs(V3_dot(T,bAxes[i]));
        if(t>ra+rb) return m;
        float ov=(ra+rb)-t;
        if(ov<minOverlap-MANIFOLD_TIE_MARGIN){minOverlap=ov; bestAxis=3+i; flipNormal=(V3_dot(T,bAxes[i])<0.f);}
    }
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) {
        int i1=(i+1)%3, i2=(i+2)%3, j1=(j+1)%3, j2=(j+2)%3;
        float t=vabs(V3_dot(T,aAxes[i2])*R[i1][j] - V3_dot(T,aAxes[i1])*R[i2][j]);
        float ra=aExt[i1]*AbsR[i2][j]+aExt[i2]*AbsR[i1][j];
        float rb=bExt[j1]*AbsR[i][j2]+bExt[j2]*AbsR[i][j1];
        if(t>ra+rb) return m;
        float axLenSq=1.f-(R[i][j]*R[i][j]);
        if (axLenSq>1e-4f) { float ov=((ra+rb)-t)/vsqrtf(axLenSq); if (ov < minOverlap-MANIFOLD_TIE_MARGIN) { V3 ea=V3_Cross(aAxes[i],bAxes[j]); minOverlap=ov; bestAxis=6+i*3+j; flipNormal=(V3_dot(T,ea)<0.f); } }
    }
    if (bestAxis < 0) return m;
    m.maxPen=minOverlap; V3 normal;
    if (bestAxis >= 6) { // Fallback for edge-edge collisions
        int i=(bestAxis - 6) / 3, j=(bestAxis - 6) % 3; V3 ea=V3_Cross(aAxes[i],bAxes[j]);
        normal = flipNormal ? V3_Normalize(ea) : V3_ScaleByF(V3_Normalize(ea),-1.f); m.normal = normal; m.n = 1; V3 sA=a.ctr;
        sA=V3_AplusB(sA,V3_ScaleByF(aAxes[0],(V3_dot(aAxes[0],normal)<0.f?1.f:-1.f)*a.hExt.x));
        sA=V3_AplusB(sA,V3_ScaleByF(aAxes[1],(V3_dot(aAxes[1],normal)<0.f?1.f:-1.f)*a.hExt.y));
        sA=V3_AplusB(sA,V3_ScaleByF(aAxes[2],(V3_dot(aAxes[2],normal)<0.f?1.f:-1.f)*a.hExt.z));
        m.p[0].point = V3_AplusB(sA,V3_ScaleByF(normal,minOverlap*0.5f)); m.p[0].pen = minOverlap; m.maxPen = minOverlap; return m;
    }
    int refAxis; float *refExt,*incExt; V3 *refAxes,*incAxes,refCenter,refNormal;
    if (bestAxis < 3) { refAxis = bestAxis; normal = flipNormal ? aAxes[refAxis] : V3_ScaleByF(aAxes[refAxis], -1.f); refAxes = aAxes; refExt = aExt; incAxes = bAxes; incExt = bExt; refCenter = a.ctr; refNormal = V3_ScaleByF(normal, -1.0f); } // Points outward from A towards B
    else { refAxis = bestAxis - 3; normal = flipNormal ? bAxes[refAxis] : V3_ScaleByF(bAxes[refAxis], -1.f); refAxes = bAxes; refExt = bExt; incAxes = aAxes; incExt = aExt; refCenter = b.ctr; refNormal = normal; } // Points outward from B towards A
    m.normal = normal; refCenter = V3_AplusB(refCenter, V3_ScaleByF(refNormal, refExt[refAxis])); float minDot=1e9f, incSign=1.0f; int incAxis = 0; // Push refCenter exactly to the touching face
    for (int i=0; i<3; ++i) { float dot = V3_dot(incAxes[i], refNormal); if (dot < minDot) { minDot = dot; incAxis = i; incSign = 1.0f; } if (-dot < minDot) { minDot = -dot; incAxis = i; incSign = -1.0f; } } // Find incident face (most anti-parallel to reference normal)
    V3 incBoxCtr = (bestAxis < 3) ? b.ctr : a.ctr; V3 incCenter = V3_AplusB(incBoxCtr, V3_ScaleByF(incAxes[incAxis], incSign * incExt[incAxis]));
    int i1 = (incAxis+1)%3, i2 = (incAxis+2)%3; V3 ext1 = V3_ScaleByF(incAxes[i1], incExt[i1]); V3 ext2 = V3_ScaleByF(incAxes[i2], incExt[i2]);
    V3 clipped[8]; int clipCount = 4;
    clipped[0] = V3_AplusB(incCenter, V3_AplusB(ext1, ext2)); clipped[1] = V3_AsubB(incCenter, V3_AsubB(ext1, ext2)); clipped[2] = V3_AsubB(incCenter, V3_AplusB(ext1, ext2)); clipped[3] = V3_AplusB(incCenter, V3_AsubB(ext1, ext2));
    int r1 = (refAxis+1)%3, r2 = (refAxis+2)%3;
    #define CLIP_PLANE(n_axis, sign) { V3 n = V3_ScaleByF(refAxes[n_axis], sign); float dist = V3_dot(n, refCenter) + refExt[n_axis]; V3 out[8]; int outCount = 0; \
        for (int i=0; i<clipCount; ++i) { \
            V3 v1 = clipped[i]; V3 v2 = clipped[(i+1)%clipCount]; float d1 = V3_dot(n, v1) - dist; float d2 = V3_dot(n, v2) - dist; if (d1 <= 0.0f) out[outCount++] = v1; \
            if ((d1 > 0.0f && d2 <= 0.0f) || (d1 <= 0.0f && d2 > 0.0f)) { float t = d1 / (d1 - d2); out[outCount++] = V3_AplusB(v1, V3_ScaleByF(V3_AsubB(v2, v1), t)); } \
        } \
        clipCount = outCount; for(int i=0; i<clipCount; ++i) clipped[i] = out[i]; \
    } // Sutherland-Hodgman clipping against the 4 side planes of the reference face
    CLIP_PLANE(r1, 1.0f) CLIP_PLANE(r1, -1.0f) CLIP_PLANE(r2, 1.0f) CLIP_PLANE(r2, -1.0f)
    float refPlaneDist = V3_dot(refNormal, refCenter); // Keep points that are behind the reference face
    for (int i=0; i<clipCount; ++i) {
        float dist = V3_dot(refNormal, clipped[i]) - refPlaneDist;
        if (dist <= 0.001f) { // Small skin tolerance
            bool isDup = false;
            for(int k=0; k<m.n; ++k) { V3 diff = V3_AsubB(clipped[i], m.p[k].point); if (V3_dot(diff, diff) < 0.00001f) { isDup = true; break; } }
            if (!isDup && m.n < MANIFOLD_MAX) { m.p[m.n].point=clipped[i]; float pen=-dist; if(pen < 0){pen=0;} m.p[m.n].pen=pen; if (pen > m.maxPen){m.maxPen=pen;} m.n++; }
        }
    }
    if (m.n == 0) { m.n = 1; m.p[0].point = V3_AplusB(refCenter, V3_ScaleByF(refNormal, 0.01f)); m.p[0].pen = minOverlap; m.maxPen = minOverlap; }
    return m;
}

V3 MvVert(const float* M, V3 v) { return (V3){ M[0]*v.x + M[4]*v.y + M[8]*v.z  + M[12], M[1]*v.x + M[5]*v.y + M[9]*v.z  + M[13], M[2]*v.x + M[6]*v.y + M[10]*v.z + M[14] }; }
INLINE void MeshTri(u16 m, u32 ti, const float* mx, V3* a, V3* b, V3* c) { u32 i0=modelTriangles[m][ti*3+0],i1=modelTriangles[m][ti*3+1],i2=modelTriangles[m][ti*3+2]; *a=MvVert(mx,MeshVert(m,i0)); *b=MvVert(mx,MeshVert(m,i1)); *c=MvVert(mx,MeshVert(m,i2)); }
INLINE V3 SphSupport(ShapeSphere b, V3 d) { float L=V3_dot(d,d); float safeL=vmax(L,PHY_EPSILON); float scale=b.rad / vsqrtf(safeL); V3 dir=V3_ScaleByF(d,scale); float mask=(L > PHY_EPSILON); return V3_AplusB(V3_ScaleByF(dir,mask),V3_ScaleByF((V3){0,b.rad,0},1.0f - mask)); }
float copysignf(float magnitude, float sign) { union { float f; u32 i; } m, s; m.f = magnitude; s.f = sign; m.i = (m.i & 0x7FFFFFFFu) | (s.i & 0x80000000u); return m.f; }
INLINE V3 BoxSupport(ShapeBox b, V3 d) { V3 x=quat_rot_v3(b.rot,(V3){1,0,0}), y=quat_rot_v3(b.rot,(V3){0,1,0}), z=quat_rot_v3(b.rot,(V3){0,0,1}); float kx = copysignf(1.0f, V3_dot(d, x)); float ky = copysignf(1.0f, V3_dot(d, y)); float kz = copysignf(1.0f, V3_dot(d, z)); return V3_AplusB(V3_AplusB(V3_AplusB(b.ctr, V3_ScaleByF(x, kx * b.hExt.x)), V3_ScaleByF(y, ky * b.hExt.y)), V3_ScaleByF(z, kz * b.hExt.z)); }
INLINE V3 CapsuleSupport(ShapeCapsule cap, V3 d) { float db=V3_dot(cap.base,d), dt=V3_dot(cap.tip,d); float mask=(dt > db); V3 best=V3_AplusB(V3_ScaleByF(cap.tip,mask),V3_ScaleByF(cap.base,1.0f - mask)); float L = V3_dot(d,d); float safeL=vmax(L,PHY_EPSILON); V3 dir=V3_ScaleByF(d,cap.rad / vsqrtf(safeL)); float lmask=(L >= PHY_EPSILON); return V3_AplusB(best,V3_ScaleByF(dir,lmask)); }
V3 HullSupport(u16 m, const float* M, u16 adjIdx, V3 dWorld) {
    V3 dLocal = (V3){M[0]*dWorld.x + M[1]*dWorld.y + M[2]*dWorld.z,M[4]*dWorld.x + M[5]*dWorld.y + M[6]*dWorld.z,M[8]*dWorld.x + M[9]*dWorld.y + M[10]*dWorld.z};
    bool haveAdj = adjIdx < uniqueCvxMeshCount && uniqueCvxMeshIndices[adjIdx] == m && cvxAdjOffsets[adjIdx] && physPos[m] && physVertCounts[m];
    u32 n = modelVertexCounts[m];
    const float* p = physPos[m];
    if (unlikely(!haveAdj || V3_dot(dLocal, dLocal) < 0.000001f || n <= 64)) {
        float bestDot = -3.402823466e38F;
        u32 bestIdx = 0;
        for (u32 i = 0; i < n; ++i) {
            float dot = p[i*3]*dLocal.x + p[i*3+1]*dLocal.y + p[i*3+2]*dLocal.z;
            if (dot > bestDot) { bestDot=dot; bestIdx=i; }
        }
        return MvVert(M, (V3){p[bestIdx*3], p[bestIdx*3+1], p[bestIdx*3+2]});
    }
    u16 curr = cvxAdjStart[adjIdx]; 
    if (curr >= n) curr = 0;
    float currDot = p[curr*3]*dLocal.x + p[curr*3+1]*dLocal.y + p[curr*3+2]*dLocal.z;
    for (u32 steps = 0; steps < n; ++steps) { // Hill-climbing search using adjacency information
        u16 next = curr; float nextDot = currDot; u32 s = cvxAdjOffsets[adjIdx][curr]; u32 e = cvxAdjOffsets[adjIdx][curr+1]; 
        for (u32 i = s; i < e; ++i) { u16 nb = cvxAdjLists[adjIdx][i]; float d = p[nb*3]*dLocal.x + p[nb*3+1]*dLocal.y + p[nb*3+2]*dLocal.z; if (d > nextDot) { nextDot = d; next = nb; } } 
        if (next == curr) break; 
        curr = next; currDot = nextDot; 
    }
    return MvVert(M, (V3){p[curr*3], p[curr*3+1], p[curr*3+2]});
}

INLINE void GJKSet(Simplex3D *s, int i, V3 v, V3 wA, V3 wB) { s->v[i] = v; s->wA[i] = wA; s->wB[i] = wB; }
INLINE void GJKCopy(Simplex3D *s, int dst, int src) { s->v[dst] = s->v[src]; s->wA[dst] = s->wA[src]; s->wB[dst] = s->wB[src]; }
INLINE void GJKSwap(Simplex3D *s, int i, int j) { V3 t = s->v[i]; s->v[i] = s->v[j]; s->v[j] = t; t = s->wA[i]; s->wA[i] = s->wA[j]; s->wA[j] = t; t = s->wB[i]; s->wB[i] = s->wB[j]; s->wB[j] = t; }
bool GJKNextSimplex(Simplex3D *s, V3 *dir) {
    V3 A = s->v[s->n - 1], AO = {-A.x,-A.y,-A.z}; V3 wAA = s->wA[s->n - 1], wBA = s->wB[s->n - 1];
    if (s->n == 2) {
        V3 AB = V3_AsubB(s->v[0], A);
        if (V3_dot(AB,AB) < PHY_EPSILON) AB=V3_AplusB(AB,V3_ScaleByF(*dir,0.001f));
        if (V3_dot(AB,AO) > 0.f){*dir = V3_Cross(V3_Cross(AB,AO),AB);} else { s->n = 1; GJKSet(s,0,A,wAA,wBA); *dir = AO; }
        if (V3_dot(*dir,*dir) < PHY_EPSILON) { V3 px = (vabs(AB.x) > 0.9f) ? (V3){0,1,0} : (V3){1,0,0}; *dir = V3_Cross(AB,px); }
        return true;
    }
    if (s->n == 3) {
        V3 B=s->v[1], C=s->v[0], AB=V3_AsubB(B,A), AC=V3_AsubB(C,A), ABC=V3_Cross(AB,AC);
        if (V3_dot(V3_Cross(ABC,AC),AO) > 0.f) { if (V3_dot(AC,AO) > 0.f) { GJKSet(s,1,A,wAA,wBA); s->n = 2; *dir = V3_Cross(V3_Cross(AC,AO),AC); } else { goto line_AB3; } }
        else if (V3_dot(V3_Cross(AB,ABC),AO) > 0.f) { line_AB3: if (V3_dot(AB,AO) > 0.f) { GJKCopy(s,0,1); GJKSet(s,1,A,wAA,wBA); s->n = 2; *dir = V3_Cross(V3_Cross(AB,AO),AB); } else { GJKSet(s,0,A,wAA,wBA); s->n = 1; *dir = AO; } }
        else { if (V3_dot(ABC,AO) > 0.f) {*dir = ABC;} else { GJKSwap(s,0,1); *dir = (V3){-ABC.x,-ABC.y,-ABC.z}; } }
        return true;
    }
    V3 B=s->v[2], C=s->v[1], D=s->v[0], AB=V3_AsubB(B,A), AC=V3_AsubB(C,A), AD=V3_AsubB(D,A);
    V3 nABC=V3_Cross(AB,AC), nACD=V3_Cross(AC,AD), nADB=V3_Cross(AD,AB);
    nABC = V3_dot(nABC,AD) > 0.f ? (V3){-nABC.x,-nABC.y,-nABC.z} : nABC;
    nACD = V3_dot(nACD,AB) > 0.f ? (V3){-nACD.x,-nACD.y,-nACD.z} : nACD;
    nADB = V3_dot(nADB,AC) > 0.f ? (V3){-nADB.x,-nADB.y,-nADB.z} : nADB;
    if (V3_dot(nABC,AO) > 0.f) { GJKCopy(s,0,1); GJKCopy(s,1,2); GJKSet(s,2,A,wAA,wBA); s->n=3; *dir=nABC; return true; }
    if (V3_dot(nACD,AO) > 0.f) { GJKSet(s,2,A,wAA,wBA); s->n = 3; *dir=nACD; return true; }
    if (V3_dot(nADB,AO) > 0.f) { GJKCopy(s,1,0); GJKCopy(s,0,2); GJKSet(s,2,A,wAA,wBA); s->n=3; *dir=nADB; return true; }
    return false;
}

typedef struct { int a,b,c; V3 n; float d; } EPAFace; typedef struct { V3 v,wA,wB; } EPAVert;
INLINE EPAFace MakeEPAFace(const EPAVert* vb, int a, int b, int c) { V3 n = V3_Cross(V3_AsubB(vb[b].v,vb[a].v),V3_AsubB(vb[c].v,vb[a].v)); float L = V3_Mag(n); if(L < PHY_EPSILON){return (EPAFace){a,b,c,{0},-1.f};} n = V3_ScaleByF(n,1.f/L); float d = V3_dot(n,vb[a].v); if(d < 0.f){n=(V3){-n.x,-n.y,-n.z}; d=-d; int t=b;b=c;c=t;} return (EPAFace){a,b,c,n,d}; }
V3 EPAContactPoint(const EPAVert* ev, int a, int b, int c) { V3 pa=ev[a].v, pb=ev[b].v, pc=ev[c].v; V3 v0=V3_AsubB(pb,pa), v1=V3_AsubB(pc,pa), v2=V3_AsubB((V3){0,0,0},pa); float d00 = V3_dot(v0,v0), d01 = V3_dot(v0,v1), d11 = V3_dot(v1,v1), d20 = V3_dot(v2,v0), d21 = V3_dot(v2,v1); float denom = d00*d11 - d01*d01 + PHY_EPSILON; float v = vmax((d11*d20 - d01*d21)*(1.0f/denom),0.0f), w = vmax((d00*d21 - d01*d20)*(1.0f/denom),0.0f), u = vmax(1.0f - v - w,0.0f); float sum = u + v + w; if (sum > PHY_EPSILON) {u /= sum; v /= sum; w /= sum;} return (V3){u*ev[a].wA.x + v*ev[b].wA.x + w*ev[c].wA.x,u*ev[a].wA.y + v*ev[b].wA.y + w*ev[c].wA.y,u*ev[a].wA.z + v*ev[b].wA.z + w*ev[c].wA.z}; }
INLINE Manifold MakeEPAManifold(const EPAVert* ev, int a, int b, int c, V3 n, float d) { Manifold m={0}; m.normal=n; m.maxPen=d; m.n=1; m.p[0]=(ManifoldPt){EPAContactPoint(ev,a,b,c),d}; return m; }
void FeatureOverlap(V3 sc, float sr, V3 pt, Overlap* r) {
    V3 delta=V3_AsubB(sc,pt); float dist2=V3_dot(delta,delta); int hit=(dist2 < sr * sr); float dist=vsqrtf(vmax(dist2,0.0f)); float nMask=(dist > PHY_EPSILON) ? 1.0f : 0.0f; float invD = 1.0f / vmax(dist, PHY_EPSILON); V3 n=V3_AplusB(V3_ScaleByF(delta,invD * nMask), V3_ScaleByF((V3){0.0f,1.0f,0.0f},1.0f - nMask)); 
    float pen = (sr - dist) * (float)hit; int better = (pen > r->pen); r->hit = r->hit | (hit & better);  r->point = better ? pt : r->point; r->normal = better ? n : r->normal; r->pen = better ? pen : r->pen; 
}

void SphTriTest(V3 sc, float sr, u16 mesh, u32 ti, const float* mx, Overlap* r) {
    V3 a,b,c; MeshTri(mesh,ti,mx,&a,&b,&c); V3 ab=V3_AsubB(b,a), ac=V3_AsubB(c,a), ap=V3_AsubB(sc,a); float d1=V3_dot(ab,ap), d2=V3_dot(ac,ap); if(d1 <= 0.0f && d2 <= 0.0f){FeatureOverlap(sc,sr,a,r); return;}
    V3 bp=V3_AsubB(sc,b); float d3=V3_dot(ab,bp), d4=V3_dot(ac,bp); if(d3 >= 0.0f && d4 <= d3){FeatureOverlap(sc,sr,b,r); return;}
    V3 cp=V3_AsubB(sc,c); float d5=V3_dot(ab,cp), d6=V3_dot(ac,cp); if(d6>=0.f && d5<=d6){FeatureOverlap(sc,sr,c,r); return;}
    float vc=d1*d4-d3*d2; if (vc<=0.f && d1>=0.f && d3<=0.f) { float v=d1/(d1-d3); V3 pt=V3_AplusB(a,V3_ScaleByF(ab,v)); FeatureOverlap(sc,sr,pt,r); return; }
    float vb=d5*d2-d1*d6; if (vb<=0.f && d2>=0.f && d6<=0.f) { float w=d2/(d2-d6); V3 pt=V3_AplusB(a,V3_ScaleByF(ac,w)); FeatureOverlap(sc,sr,pt,r); return; }
    float va=d3*d6-d5*d4; if (va<=0.f && (d4-d3)>=0.f && (d5-d6)>=0.f) { float w=(d4-d3)/((d4-d3)+(d5-d6)); V3 bc=V3_AsubB(c,b); V3 pt=V3_AplusB(b,V3_ScaleByF(bc,w)); FeatureOverlap(sc,sr,pt,r); return; }
    V3 n = V3_Cross(ab,ac); float nLen=V3_Mag(n); if(nLen<PHY_EPSILON) return; n=V3_ScaleByF(n,1.f/nLen); float dist=V3_dot(n,ap), absDist=vabs(dist);
    if (absDist < sr) { V3 fn = /*(dist >= 0.0f) ? n : Ah nope, want it to be one-sided so that if ever small objects just barely penetrate their center past the tri it doesn't pop it through the wall/floor*/ (V3){-n.x,-n.y,-n.z}; // For some reason it always needs negated to work properly.
    Overlap t={true,V3_AsubB(sc,V3_ScaleByF(fn,absDist)),fn,sr-absDist}; if(t.pen>r->pen) *r=t; }
}

INLINE V3 TriSupport(V3 ta, V3 tb, V3 tc, V3 d) { float d1=V3_dot(ta,d),d2=V3_dot(tb,d),d3=V3_dot(tc,d); return d1>d2 ? (d1>d3 ? ta : tc) : (d2>d3 ? tb : tc); }
typedef struct SupportCtx { V3 (*supA)(const struct SupportCtx *ctx, V3 dir); V3 (*supB)(const struct SupportCtx *ctx, V3 negDir); u16 prim,meshA,meshB; const float *matA,*matB; V3 ta,tb,tc; u16 adjA,adjB; ShapeBox boxShape; } SupportCtx;
INLINE V3 _supA_hull(const SupportCtx *ctx, V3 d) { return HullSupport(ctx->meshA, ctx->matA, ctx->adjA, d); }
INLINE V3 _supA_sph(const SupportCtx *ctx, V3 d)  { return SphSupport(Entity_GetSph(ctx->prim), d); }
INLINE V3 _supA_box(const SupportCtx *ctx, V3 d)  { return BoxSupport(Entity_GetBox(ctx->prim), d); }
INLINE V3 _supA_boxShape(const SupportCtx *ctx, V3 d) { return BoxSupport(ctx->boxShape, d); }
INLINE V3 _supA_cap(const SupportCtx *ctx, V3 d)  { return CapsuleSupport(Entity_GetCap(ctx->prim), d); }
INLINE V3 _supB_hull(const SupportCtx *ctx, V3 nd)  { return HullSupport(ctx->meshB, ctx->matB, ctx->adjB, nd); }
INLINE V3 _supB_hullA(const SupportCtx *ctx, V3 nd) { return HullSupport(ctx->meshA, ctx->matA, ctx->adjA, nd); }
INLINE V3 _supB_tri(const SupportCtx *ctx, V3 nd)   { return TriSupport(ctx->ta, ctx->tb, ctx->tc, nd); }
INLINE void GetSupportPair(const SupportCtx *ctx, V3 dir, V3 *wA, V3 *wB) { V3 nd = {-dir.x, -dir.y, -dir.z}; *wA = ctx->supA(ctx, dir); *wB = ctx->supB(ctx, nd); }
typedef struct { Simplex3D s; V3 dir; bool hit; } GJKResult;
GJKResult RunGJK(const SupportCtx *ctx, int maxIter) {
    GJKResult res = {0}; res.dir = (V3){0, 1, 0}; V3 wA, wB;
    GetSupportPair(ctx,res.dir,&wA,&wB);
    res.s.wA[res.s.n] = wA; res.s.wB[res.s.n] = wB; res.s.v[res.s.n++] = V3_AsubB(wA, wB);
    res.dir = (V3){-res.s.v[0].x, -res.s.v[0].y, -res.s.v[0].z};
    if (V3_dot(res.dir, res.dir) < PHY_EPSILON) res.dir = (V3){0, 1, 0};
    for (int it = 0; it < maxIter; ++it) { GetSupportPair(ctx, res.dir, &wA, &wB); V3 sup = V3_AsubB(wA, wB); if (V3_dot(sup, res.dir) < 0) {break;} res.s.wA[res.s.n] = wA; res.s.wB[res.s.n] = wB; res.s.v[res.s.n++] = sup; if (!GJKNextSimplex(&res.s, &res.dir)) { res.hit = true; break; } }
    return res;
}

INLINE void RunGJKFallback(const SupportCtx *ctx, Simplex3D *s) { static const V3 kAx[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}}; for(int d=0;s->n<4 && d<6;++d){V3 wA,wB; GetSupportPair(ctx,kAx[d],&wA,&wB); V3 sup=V3_AsubB(wA,wB); bool dup=false; for (int k=0;k<s->n;++k){V3 dv=V3_AsubB(sup,s->v[k]); dup |= (V3_dot(dv,dv) < PHY_EPSILON * PHY_EPSILON); } if(!dup){s->wA[s->n]=wA; s->wB[s->n]=wB; s->v[s->n++]=sup;}} }
typedef struct { EPAVert ev[EPA_MAX_VERTS]; EPAFace ef[EPA_MAX_FACES]; int nv, nf; } EPAState;
void SeedEPA(EPAState *epa, const Simplex3D *s) {
    static const int kTetFaces[4][3] = {{0,1,2},{0,3,1},{0,2,3},{1,3,2}}; epa->nv = 0; epa->nf = 0;
    for (int i = 0; i < 4; i++) { epa->ev[epa->nv].wA = s->wA[i]; epa->ev[epa->nv].wB = s->wB[i]; epa->ev[epa->nv].v = s->v[i]; epa->nv++; }
    for (int f = 0; f < 4; f++) { EPAFace face = MakeEPAFace(epa->ev,kTetFaces[f][0],kTetFaces[f][1],kTetFaces[f][2]); if(face.d >= 0.f && epa->nf < EPA_MAX_FACES){epa->ef[epa->nf++]=face;} }
}

bool ExpandEPA(EPAState *epa, V3 sup, V3 wA, V3 wB) {
    if(epa->nv >= EPA_MAX_VERTS){return false;}
    epa->ev[epa->nv].v=sup; epa->ev[epa->nv].wA=wA; epa->ev[epa->nv].wB=wB;
    int edges[EPA_MAX_EDGES][2], ne = 0, keep[EPA_MAX_FACES], nk = 0;
    for (int f = 0; f < epa->nf; f++) {
        if (V3_dot(epa->ef[f].n, V3_AsubB(sup, epa->ev[epa->ef[f].a].v)) > 0.f) {
            int fv[3] = {epa->ef[f].a, epa->ef[f].b, epa->ef[f].c};
            for (int e = 0; e < 3; e++) {
                int ea = fv[e], eb = fv[(e + 1) % 3]; bool found = false;
                for (int k = 0; k < ne; k++) if (edges[k][0] == eb && edges[k][1] == ea) { edges[k][0] = edges[--ne][0]; edges[k][1] = edges[ne][1]; found = true; break; }
                if (!found && ne < EPA_MAX_EDGES) { edges[ne][0] = ea; edges[ne++][1] = eb; }
            }
        } else keep[nk++] = f;
    }
    epa->nf = 0; for (int k = 0; k < nk; k++) epa->ef[epa->nf++] = epa->ef[keep[k]];
    for (int k = 0; k < ne && epa->nf < EPA_MAX_FACES; k++) { EPAFace face = MakeEPAFace(epa->ev, edges[k][0], edges[k][1], epa->nv); if (face.d >= 0.f) epa->ef[epa->nf++] = face; }
    epa->nv++; return true;
}

INLINE bool BvhSphereAABBOverlap(V3 sc, float sr, V3 mn, V3 mx) { V3 cl = {vclamp(sc.x, mn.x, mx.x), vclamp(sc.y, mn.y, mx.y), vclamp(sc.z, mn.z, mx.z)}; V3 d = V3_AsubB(sc, cl); return V3_dot(d, d) <= sr * sr; }
INLINE void BvhNodeWorldAABB(const BvhNode* node, const float* mx, V3* wMn, V3* wMx) {
    __m128 col0=_mm_loadu_ps(mx + 0); __m128 col1=_mm_loadu_ps(mx + 4); __m128 col2=_mm_loadu_ps(mx + 8); __m128 tr=_mm_loadu_ps(mx + 12);  __m128 mn_v=_mm_setr_ps(node->mn.x,node->mn.y,node->mn.z,0.0f); __m128 mx_v=_mm_setr_ps(node->mx.x,node->mx.y,node->mx.z,0.0f);
    __m128 lc = _mm_mul_ps(_mm_add_ps(mn_v,mx_v),_mm_set1_ps(0.5f)); // lc = (mn + mx) * 0.5f   
    __m128 lh = _mm_mul_ps(((__m128)((__v4sf)(mx_v) - (__v4sf)(mn_v))),_mm_set1_ps(0.5f)); // lh = (mx - mn) * 0.5f
    __m128 lc_x = __builtin_shufflevector(lc,lc,0,0,0,0); __m128 lc_y = __builtin_shufflevector(lc,lc,1,1,1,1); __m128 lc_z = __builtin_shufflevector(lc,lc,2,2,2,2); // Replicate lc.x, lc.y, lc.z across vectors
    __m128 wc = _mm_add_ps(_mm_add_ps(_mm_mul_ps(col0,lc_x), _mm_mul_ps(col1,lc_y)), _mm_add_ps(_mm_mul_ps(col2,lc_z),tr)); // wc = col0 * lc_x + col1 * lc_y + col2 * lc_z + tr
    __v4si sign_mask = (__v4si)_mm_set1_ps(-0.0f); __v4si inv_mask = ~sign_mask;
    __m128 abs_col0 = (__m128)((__v4si)col0 & inv_mask); __m128 abs_col1 = (__m128)((__v4si)col1 & inv_mask); __m128 abs_col2 = (__m128)((__v4si)col2 & inv_mask); // Take absolute value of matrix columns using bitwise AND
    __m128 lh_x = __builtin_shufflevector(lh,lh,0,0,0,0); __m128 lh_y = __builtin_shufflevector(lh,lh,1,1,1,1); __m128 lh_z = __builtin_shufflevector(lh,lh,2,2,2,2); // Replicate lh components
    __m128 wh = _mm_add_ps(_mm_add_ps(_mm_mul_ps(abs_col0,lh_x),_mm_mul_ps(abs_col1,lh_y)),_mm_mul_ps(abs_col2,lh_z)); // wh = abs_col0 * lh_x + abs_col1 * lh_y + abs_col2 * lh_z
    __m128 wMn_v=((__m128)((__v4sf)(wc) - (__v4sf)(wh))); __m128 wMx_v=_mm_add_ps(wc,wh); // wMn = wc - wh, wMx = wc + wh
    wMn->x = wMn_v[0]; wMn->y = wMn_v[1]; wMn->z = wMn_v[2]; wMx->x = wMx_v[0]; wMx->y = wMx_v[1]; wMx->z = wMx_v[2]; // Store back to V3 (avoids overwriting adjacent struct memory)
}

void BvhWalkSphMsh(V3 sc, float sr, u16 m, const float* mx, Overlap* r) {
    const BvhNode* nodes=modelBVHNodes[m]; const u16* triOrder = modelBVHTriOrder[m]; const BvhNode* stack[64]; int sp = 0; stack[sp++] = &nodes[0];
    while (sp > 0) {
        const BvhNode* node = stack[--sp]; V3 wMn,wMx; BvhNodeWorldAABB(node,mx,&wMn,&wMx); if (!BvhSphereAABBOverlap(sc,sr,wMn,wMx)) continue;
        if (node->triCount > 0) { for (u32 i = 0; i < node->triCount; i++) SphTriTest(sc, sr, m, triOrder[node->triStart + i], mx, r); } else { for (int o=0;o<8;++o) if (node->children[o] >= 0) stack[sp++] = &nodes[node->children[o]]; }
    }
}

Overlap SphMsh(V3 sc, float sr, u16 m, const float* mx) { Overlap r={0}; if(m>=MAX_MDLS)return r; u32 tc=modelTriangleCounts[m]; if(!tc){return r;} if (BvhHasBVH(m)) { BvhWalkSphMsh(sc,sr,m,mx,&r); return r; } for(u32 ti=0;ti<tc;++ti){SphTriTest(sc,sr,m,ti,mx,&r);} return r; }
Overlap CapMsh(ShapeCapsule c, u16 m, const float* mx) { Overlap best=SphMsh(c.base,c.rad,m,mx), rt=SphMsh(c.tip,c.rad,m,mx); if(rt.pen>best.pen)best=rt; V3 d=V3_AsubB(c.tip,c.base); if(V3_Mag(d)>PHY_EPSILON){/*Hey I was doing a snowman of just the end spheres, don't hate the simplicity, I only use capsules for npcs and player*/for(int k=1;k<6;++k){float t=(float)k/5.0f; Overlap rm=SphMsh(V3_AplusB(c.base,V3_ScaleByF(d,t)),c.rad,m,mx); if(rm.pen>best.pen)best=rm;}} return best; }
Manifold PrimitiveCvx(u16 prim, u16 mesh, const float* mx, u16 adjIdx) {
    Manifold m={0}; if(mesh>=MAX_MDLS||adjIdx>=MAX_MDLS||!modelVertexCounts[mesh])return m;
    u8 col = World.col[prim]; V3 (*supA)(const SupportCtx*, V3) = (col == COLTYPE_SPH) ? _supA_sph : (col == COLTYPE_BOX) ? _supA_box : _supA_cap;
    SupportCtx ctx = (SupportCtx){supA, _supB_hullA, .prim=prim, .meshA=mesh, .matA=mx, .adjA=adjIdx, .adjB=adjIdx};
    GJKResult gjk = RunGJK(&ctx,GJK_ITER); if(!gjk.hit)return m;
    if(gjk.s.n<4) RunGJKFallback(&ctx,&gjk.s); if(gjk.s.n<4)return m;
    EPAState epa; SeedEPA(&epa,&gjk.s);
    for(int it=0;it<EPA_ITER;++it){
        int bf=-1; float bd=1e9f; for(int f=0;f<epa.nf;f++)if(epa.ef[f].d<bd){bd=epa.ef[f].d;bf=f;} if(bf<0)break;
        V3 bn=epa.ef[bf].n; V3 wA, wB; GetSupportPair(&ctx,bn,&wA,&wB); V3 sup=V3_AsubB(wA,wB);
        if(V3_dot(bn,sup)-bd<PHY_EPSILON){return MakeEPAManifold(epa.ev,epa.ef[bf].a,epa.ef[bf].b,epa.ef[bf].c,bn,bd);}
        if (!ExpandEPA(&epa,sup,wA,wB)) break;
    }
    return m;
}

typedef struct {V3 mn,mx;} AABB3;
typedef struct {u16 hullMesh; const float* hullMx; const V3* boxV; u32 boxN; AABB3 hb; V3 hullCenter; float hullRadius,spreadEps,thicknessTolerance; Manifold best; u16 adjHull; ShapeBox boxShape; V3 bestTa,bestTb,bestTc,bestTriN,bestDeepPoint; float bestTriD; bool haveBestTri;} CvxMshCtx;
void CvxTriTest(CvxMshCtx* ctx, V3 ta, V3 tb, V3 tc) {
    u16 hullMesh=ctx->hullMesh; Manifold* best=&ctx->best;
    if (vmin(ta.x,vmin(tb.x,tc.x))>ctx->hb.mx.x || vmax(ta.x,vmax(tb.x,tc.x))<ctx->hb.mn.x || vmin(ta.y,vmin(tb.y,tc.y))>ctx->hb.mx.y || vmax(ta.y,vmax(tb.y,tc.y))<ctx->hb.mn.y || vmin(ta.z,vmin(tb.z,tc.z))>ctx->hb.mx.z || vmax(ta.z,vmax(tb.z,tc.z))<ctx->hb.mn.z) return;
    V3 triEdge1=V3_AsubB(tb,ta), triEdge2=V3_AsubB(tc,ta); V3 triN=V3_Cross(triEdge1,triEdge2); float triLenSq=V3_dot(triN,triN); if (triLenSq < PHY_EPSILON) return;
    triN = V3_ScaleByF(triN, 1.0f / vsqrtf(triLenSq));
    if ((ctx->hullRadius - vabs(V3_dot(triN,V3_AsubB(ctx->hullCenter, ta)))) <= (best->n ? best->maxPen + MANIFOLD_TIE_MARGIN : 0.0f)) return;
    SupportCtx supCtx = (SupportCtx){ctx->boxV ? _supA_boxShape : _supA_hull, _supB_tri, .meshA=hullMesh, .matA=ctx->hullMx, .adjA=ctx->adjHull, .adjB=ctx->adjHull, .ta=ta, .tb=tb, .tc=tc, .boxShape=ctx->boxShape};
    GJKResult gjk = RunGJK(&supCtx,GJK_ITER); if(!gjk.hit){return;}
    Simplex3D *s = &gjk.s;
    while (s->n<4) {
        V3 fallbackDir={0.0f,1.0f,0.0f};
        if(s->n==1) fallbackDir=(vabs(s->v[0].x)>0.5f)?(V3){0.0f,1.0f,0.0f}:(V3){1.0f,0.0f,0.0f};
        else if(s->n==2){V3 edge=V3_AsubB(s->v[1],s->v[0]); fallbackDir=V3_Cross(edge,(vabs(edge.x)>0.5f)?(V3){0.0f,1.0f,0.0f}:(V3){1.0f,0.0f,0.0f});}
        else if(s->n==3){V3 e1=V3_AsubB(s->v[1],s->v[0]), e2=V3_AsubB(s->v[2],s->v[0]); fallbackDir=V3_Cross(e1,e2);}
        float fLen=V3_Mag(fallbackDir); fallbackDir=(fLen>PHY_EPSILON)?V3_ScaleByF(fallbackDir,1.0f/fLen):(V3){0.0f,1.0f,0.0f};
        V3 wA, wB; GetSupportPair(&supCtx, fallbackDir, &wA, &wB); V3 sup=V3_AsubB(wA,wB); bool dup=false;
        for (int k=0;k<s->n;k++){V3 dv=V3_AsubB(sup,s->v[k]); dup|=(V3_dot(dv,dv)<PHY_EPSILON*PHY_EPSILON);}
        if (!dup){s->wA[s->n]=wA; s->wB[s->n]=wB; s->v[s->n++]=sup;} else {fallbackDir=(V3){-fallbackDir.x,-fallbackDir.y,-fallbackDir.z}; GetSupportPair(&supCtx, fallbackDir, &wA, &wB); s->wA[s->n]=wA; s->wB[s->n]=wB; s->v[s->n++]=V3_AsubB(wA,wB);}
    }
    EPAState epa; SeedEPA(&epa, s); if (epa.nf<4){ return; } bool tHit=false; V3 tN={0}; float tD=0; V3 tP={0};
    for (int it=0;it<EPA_ITER;++it){
        int bf=-1; float bd=1e9f; for (int f=0;f<epa.nf;f++)if(epa.ef[f].d<bd){bd=epa.ef[f].d;bf=f;} if(bf<0)break;
        V3 bn=epa.ef[bf].n; V3 wA, wB; GetSupportPair(&supCtx, bn, &wA, &wB); V3 sup=V3_AsubB(wA,wB);
        if(V3_dot(bn,sup)-bd<PHY_EPSILON){ if(V3_dot(bn,triN) < 0.0f){bn=triN;} tHit=true; tN=bn; tD=bd; tP=EPAContactPoint(epa.ev,epa.ef[bf].a,epa.ef[bf].b,epa.ef[bf].c); break; }
        if (!ExpandEPA(&epa,sup,wA,wB)) break;
    }
    if (!tHit){return;} V3 deepPoint=tP;
    if (!best->n) {
        best->normal=tN; best->maxPen=tD; best->p[best->n++]=(ManifoldPt){deepPoint,tD};
        ctx->bestTa=ta; ctx->bestTb=tb; ctx->bestTc=tc; ctx->bestTriN=tN; ctx->bestTriD=tD; ctx->bestDeepPoint=deepPoint; ctx->haveBestTri=true;
    } else {
        float align=V3_dot(tN,best->normal);
        if (align>MANIFOLD_ALIGN_THRESHOLD) {
            bool better=(tD>best->maxPen+MANIFOLD_TIE_MARGIN) || (vabs(tD-best->maxPen)<=MANIFOLD_TIE_MARGIN && V3_dot(tN,(V3){0,1,0})>V3_dot(best->normal,(V3){0,1,0}));
            if (better){ best->normal=tN; best->maxPen=tD; ctx->bestTa=ta; ctx->bestTb=tb; ctx->bestTc=tc; ctx->bestTriN=tN; ctx->bestTriD=tD; ctx->bestDeepPoint=deepPoint; ctx->haveBestTri=true; }
            bool spread=true;
            for (int k=0;k<best->n;++k){V3 dv=V3_AsubB(deepPoint,best->p[k].point); if(V3_dot(dv,dv)<ctx->spreadEps*ctx->spreadEps){spread=false; if(tD>best->p[k].pen)best->p[k].pen=tD; break;}}
            if (spread&&best->n<MANIFOLD_MAX)best->p[best->n++]=(ManifoldPt){deepPoint,tD};
        } else if (tD>best->maxPen+MANIFOLD_TIE_MARGIN){
            best->n=0; best->normal=tN; best->maxPen=tD; best->p[best->n++]=(ManifoldPt){deepPoint,tD};
            ctx->bestTa=ta; ctx->bestTb=tb; ctx->bestTc=tc; ctx->bestTriN=tN; ctx->bestTriD=tD; ctx->bestDeepPoint=deepPoint; ctx->haveBestTri=true;
        }
    }
}

void CvxMshFillExtraPoints(CvxMshCtx* ctx) {
    Manifold* best = &ctx->best;
    if (!ctx->haveBestTri || best->n == 0 || best->n >= MANIFOLD_MAX) return;
    u32 hn = ctx->boxV ? ctx->boxN : modelVertexCounts[ctx->hullMesh];
    if (!hn) return;
    V3 ta=ctx->bestTa, tN=ctx->bestTriN;
    V3 triEdge1=V3_AsubB(ctx->bestTb,ta), triEdge2=V3_AsubB(ctx->bestTc,ta);
    float planeDist=V3_dot(tN,ctx->bestDeepPoint), tD=ctx->bestTriD;
    float d00=V3_dot(triEdge1,triEdge1), d01=V3_dot(triEdge1,triEdge2), d11=V3_dot(triEdge2,triEdge2), denom=d00*d11-d01*d01;
    bool validTri=vabs(denom)>PHY_EPSILON;
    for (u32 i=0;i<hn && best->n<MANIFOLD_MAX;++i) {
        V3 pt=ctx->boxV ? ctx->boxV[i] : MvVert(ctx->hullMx,MeshVert(ctx->hullMesh,i));
        float distToPlane=V3_dot(tN,pt)-planeDist;
        if (vabs(distToPlane)<ctx->thicknessTolerance) {
            bool insideTri=false;
            if (validTri){V3 projPt=V3_AsubB(pt,V3_ScaleByF(tN,distToPlane)), v2=V3_AsubB(projPt,ta); float d20=V3_dot(v2,triEdge1), d21=V3_dot(v2,triEdge2), v=(d11*d20-d01*d21)/denom, w=(d00*d21-d01*d20)/denom, u=1.0f-v-w; if(u>=-0.02f&&v>=-0.02f&&w>=-0.02f)insideTri=true;}
            if (insideTri){
                float ptPen=tD-distToPlane;
                if(ptPen>0.0f){
                    bool isDup=false;
                    for(int k=0;k<best->n;++k){V3 diff=V3_AsubB(pt,best->p[k].point); if(V3_dot(diff,diff)<ctx->spreadEps*ctx->spreadEps){isDup=true;break;}}
                    if(!isDup&&best->n<MANIFOLD_MAX)best->p[best->n++]=(ManifoldPt){pt,ptPen};
                }
            }
        }
    }
}

void BvhWalkAABB_CvxTri(u16 triMesh, const float* triMx, AABB3 hb, CvxMshCtx* ctx) {
    const BvhNode* nodes = modelBVHNodes[triMesh]; const u16* triOrder = modelBVHTriOrder[triMesh]; const BvhNode* stack[64]; int sp = 0; stack[sp++] = &nodes[0];
    while (sp > 0) {
        const BvhNode* node = stack[--sp]; V3 wMn, wMx; BvhNodeWorldAABB(node, triMx, &wMn, &wMx);
        bool bvhAABBOverlap = (wMx.x >= hb.mn.x && wMn.x <= hb.mx.x && wMx.y >= hb.mn.y && wMn.y <= hb.mx.y && wMx.z >= hb.mn.z && wMn.z <= hb.mx.z);
        if (!bvhAABBOverlap) continue;
        if (node->triCount > 0) { for (u32 i = 0; i < node->triCount; i++) { V3 ta, tb, tc; MeshTri(triMesh, triOrder[node->triStart + i], triMx, &ta, &tb, &tc); CvxTriTest(ctx, ta, tb, tc); } }
        else { for (int o = 0; o < 8; o++) if (node->children[o] >= 0) stack[sp++] = &nodes[node->children[o]]; }
    }
}

// --- hull local-AABB cache (computed once per hull mesh, reused across queries) ---
static AABB3 g_hullLocalAABB[MAX_MDLS];
static u8 g_hullLocalAABBInit[MAX_MDLS] = {0};
Manifold CvxMsh(u16 hullMesh, const float* hullMx, u16 triMesh, const float* triMx, u16 adjHull) {
    Manifold z={0}; if(hullMesh>=MAX_MDLS||adjHull>=MAX_MDLS||triMesh>=MAX_MDLS)return z;
    u32 hn=modelVertexCounts[hullMesh]; if(!hn)return z;
    CvxMshCtx ctx={0}; ctx.hullMesh=hullMesh; ctx.hullMx=hullMx; ctx.adjHull=adjHull; AABB3 hb;
    if (!g_hullLocalAABBInit[hullMesh]) {
        AABB3 la={{1e9f,1e9f,1e9f},{-1e9f,-1e9f,-1e9f}};
        for (u32 i=0;i<hn;++i){ V3 v=MeshVert(hullMesh,i); la.mn.x=vmin(la.mn.x,v.x); la.mn.y=vmin(la.mn.y,v.y); la.mn.z=vmin(la.mn.z,v.z); la.mx.x=vmax(la.mx.x,v.x); la.mx.y=vmax(la.mx.y,v.y); la.mx.z=vmax(la.mx.z,v.z); }
        g_hullLocalAABB[hullMesh]=la; g_hullLocalAABBInit[hullMesh]=1;
    }
    { const AABB3* la=&g_hullLocalAABB[hullMesh]; V3 c[8]={{la->mn.x,la->mn.y,la->mn.z},{la->mn.x,la->mn.y,la->mx.z},{la->mn.x,la->mx.y,la->mn.z},{la->mn.x,la->mx.y,la->mx.z},{la->mx.x,la->mn.y,la->mn.z},{la->mx.x,la->mn.y,la->mx.z},{la->mx.x,la->mx.y,la->mn.z},{la->mx.x,la->mx.y,la->mx.z}};
      hb.mn.x=1e9f;hb.mn.y=1e9f;hb.mn.z=1e9f;hb.mx.x=-1e9f;hb.mx.y=-1e9f;hb.mx.z=-1e9f;
      for (int i=0;i<8;++i){ V3 w=MvVert(hullMx,c[i]); hb.mn.x=vmin(hb.mn.x,w.x); hb.mn.y=vmin(hb.mn.y,w.y); hb.mn.z=vmin(hb.mn.z,w.z); hb.mx.x=vmax(hb.mx.x,w.x); hb.mx.y=vmax(hb.mx.y,w.y); hb.mx.z=vmax(hb.mx.z,w.z); } }
    ctx.hb=hb; ctx.hullCenter = V3_ScaleByF(V3_AplusB(hb.mn, hb.mx), 0.5f); ctx.hullRadius = V3_Mag(V3_AsubB(hb.mx, hb.mn)) * 0.5f;
    V3 hext=V3_AsubB(hb.mx,hb.mn); ctx.spreadEps=vmax(0.02f,vmax(hext.x,vmax(hext.y,hext.z))*0.15f);
    float wscaleH=V3_Mag((V3){hullMx[0],hullMx[1],hullMx[2]}); ctx.thicknessTolerance=vclamp(modelBounds[hullMesh]*wscaleH*0.06f,0.003f,0.02f);
    u32 triCount=modelTriangleCounts[triMesh]; if(!triCount)return ctx.best;
    if (BvhHasBVH(triMesh)) { BvhWalkAABB_CvxTri(triMesh, triMx, hb, &ctx); }
    else { for (u32 ti=0;ti<triCount;++ti) { V3 ta,tb,tc; MeshTri(triMesh,ti,triMx,&ta,&tb,&tc); CvxTriTest(&ctx,ta,tb,tc); } }
    CvxMshFillExtraPoints(&ctx);
    return ctx.best;
}

void obb_axes(Quaternion q, V3 *ax, V3 *ay, V3 *az) { *ax=quat_rot_v3(q,(V3){1,0,0}); *ay=quat_rot_v3(q,(V3){0,1,0}); *az=quat_rot_v3(q,(V3){0,0,1}); }
AABB3 BoxWorldAABB(ShapeBox b) { V3 x,y,z; obb_axes(b.rot,&x,&y,&z); V3 hx=V3_ScaleByF(x,b.hExt.x), hy=V3_ScaleByF(y,b.hExt.y), hz=V3_ScaleByF(z,b.hExt.z); V3 e ={vabs(hx.x)+vabs(hy.x)+vabs(hz.x),vabs(hx.y)+vabs(hy.y)+vabs(hz.y),vabs(hx.z)+vabs(hy.z)+vabs(hz.z)}; return (AABB3){V3_AsubB(b.ctr,e),V3_AplusB(b.ctr,e)}; }
static Manifold BoxMsh(ShapeBox box, u16 triMesh, const float* triMx) {
    Manifold z={0}; if(triMesh>=MAX_MDLS||!modelTriangleCounts[triMesh]) return z;
    CvxMshCtx ctx={0}; ctx.boxShape=box; ctx.adjHull=U16_MAX;
    AABB3 hb=BoxWorldAABB(box); float skin=0.02f; hb.mn.x-=skin; hb.mn.y-=skin; hb.mn.z-=skin; hb.mx.x+=skin; hb.mx.y+=skin; hb.mx.z+=skin; ctx.hb=hb;
    ctx.hullCenter=box.ctr; ctx.hullRadius=V3_Mag(box.hExt);
    V3 ext=V3_AsubB(hb.mx,hb.mn); ctx.spreadEps=vmax(0.02f,vmax(ext.x,vmax(ext.y,ext.z))*0.15f);
    ctx.thicknessTolerance=vclamp(V3_Mag(box.hExt)*0.06f,0.003f,0.02f);
    V3 ax,ay,az; obb_axes(box.rot,&ax,&ay,&az);
    V3 hx=V3_ScaleByF(ax,box.hExt.x),hy=V3_ScaleByF(ay,box.hExt.y),hz=V3_ScaleByF(az,box.hExt.z);
    V3 bv[8]={V3_AplusB(V3_AplusB(V3_AplusB(box.ctr,hx),hy),hz),V3_AplusB(V3_AsubB(V3_AplusB(box.ctr,hx),hy),hz),V3_AplusB(V3_AplusB(V3_AsubB(box.ctr,hx),hy),hz), V3_AplusB(V3_AsubB(V3_AsubB(box.ctr,hx),hy),hz),
              V3_AsubB(V3_AplusB(V3_AplusB(box.ctr,hx),hy),hz), V3_AsubB(V3_AsubB(V3_AplusB(box.ctr,hx),hy),hz), V3_AsubB(V3_AplusB(V3_AsubB(box.ctr,hx),hy),hz),  V3_AsubB(V3_AsubB(V3_AsubB(box.ctr,hx),hy),hz)};
    ctx.boxV=bv; ctx.boxN=8;
    if (BvhHasBVH(triMesh)) { BvhWalkAABB_CvxTri(triMesh,triMx,ctx.hb,&ctx); }
    else { u32 triCount=modelTriangleCounts[triMesh]; for(u32 ti=0;ti<triCount;++ti){V3 ta,tb,tc; MeshTri(triMesh,ti,triMx,&ta,&tb,&tc); CvxTriTest(&ctx,ta,tb,tc);} }
    CvxMshFillExtraPoints(&ctx);
    if(ctx.best.n) ctx.best.normal=V3_ScaleByF(ctx.best.normal,-1.f);
    return ctx.best;
}

Manifold CvxCvx(u16 meshA, u16 meshB, const float* matA, const float* matB, u16 adjA, u16 adjB) {
    Manifold m={0}; if(meshA>=MAX_MDLS||adjA>=MAX_MDLS||meshB>=MAX_MDLS||adjB>=MAX_MDLS)return m;
    SupportCtx ctx = (SupportCtx){_supA_hull, _supB_hull, .meshA=meshA, .meshB=meshB, .matA=matA, .matB=matB, .adjA=adjA, .adjB=adjB};
    GJKResult gjk = RunGJK(&ctx,GJK_ITER); if(!gjk.hit)return m;
    if(gjk.s.n<4) RunGJKFallback(&ctx,&gjk.s); if(gjk.s.n<4)return m;
    EPAState epa; SeedEPA(&epa, &gjk.s);
    for(int it=0;it<EPA_ITER;++it) {
        int bf=-1; float bd=1e9f; for(int f=0;f<epa.nf;f++)if(epa.ef[f].d<bd){bd=epa.ef[f].d;bf=f;} if(bf<0)break;
        V3 bn=epa.ef[bf].n; V3 wA, wB; GetSupportPair(&ctx, bn, &wA, &wB); V3 sup=V3_AsubB(wA,wB);
        if (V3_dot(bn,sup)-bd<PHY_EPSILON) {
            m.normal=bn; m.maxPen=bd; m.n=1; V3 deepPoint=EPAContactPoint(epa.ev,epa.ef[bf].a,epa.ef[bf].b,epa.ef[bf].c); m.p[0]=(ManifoldPt){deepPoint,bd};
            u32 nVertsB = modelVertexCounts[meshB];
            if (nVertsB > 0) {
                float planeDist=V3_dot(bn,deepPoint),wscaleB=V3_Mag((V3){matB[0],matB[1],matB[2]}),thicknessTolerance=vclamp(modelBounds[meshB]*wscaleB*0.06f,0.003f,0.02f);
                const u8* vb = (u8*)physPos[meshB];
                for(u32 i=0;i<nVertsB;++i) {
                    const u8* p = vb + i * 12; V3 ptLocal = *(V3*)p; V3 pt = MvVert(matB,ptLocal); float distToPlane=V3_dot(bn,pt)-planeDist;
                    if (vabs(distToPlane)<thicknessTolerance) { 
                        float ptPen=bd-distToPlane; 
                        if(ptPen>0.0f) { bool isDup=false; for(int k=0;k<m.n;++k){V3 diff=V3_AsubB(pt,m.p[k].point); if(V3_dot(diff,diff)<0.00001f){isDup=true; break;}} if(!isDup&&m.n<MANIFOLD_MAX){m.p[m.n++]=(ManifoldPt){pt,ptPen};} if(m.n>=MANIFOLD_MAX){break;} }
                    }
                }
            }
            return m;
        }
        if (!ExpandEPA(&epa,sup,wA,wB)) break;
    }
    return m;
}

INLINE void quat_to_mat3(Quaternion q, float R[3][3]) { float x=q.x,y=q.y,z=q.z,w=q.w, xx=x*x,yy=y*y,zz=z*z, xy=x*y,xz=x*z,yz=y*z, wx=w*x,wy=w*y,wz=w*z; R[0][0]=1.0f-2.0f*(yy+zz); R[0][1]=2.0f*(xy-wz); R[0][2]=2.0f*(xz+wy); R[1][0]=2.0f*(xy+wz); R[1][1]=1.0f-2.0f*(xx+zz); R[1][2]=2.0f*(yz-wx); R[2][0]=2.0f*(xz-wy); R[2][1]=2.0f*(yz+wx); R[2][2]=1.0f-2.0f*(xx+yy); }
INLINE void BuildInvInertiaMatrix(u16 i, const float R[3][3], float K[3][3]) { /* K = R * I_inv * R^T, precomputed per body for solver hotpath */ float I0,I1,I2,I3,I4,I5; if (World.col[i]==COLTYPE_BOX) { ShapeBox b=Entity_GetBox(i); float m=World.mass[i],hx=b.hExt.x,hy=b.hExt.y,hz=b.hExt.z; I0=1.0f/vmax((1.0f/3.0f)*m*(hy*hy+hz*hz),1e-6f); I1=1.0f/vmax((1.0f/3.0f)*m*(hx*hx+hz*hz),1e-6f); I2=1.0f/vmax((1.0f/3.0f)*m*(hx*hx+hy*hy),1e-6f); I3=I4=I5=0.0f; } else if (World.col[i]==COLTYPE_CVX && World.invTnsrValid[i]) { float *II=World.invInertiaTensor[i]; I0=II[0]; I1=II[1]; I2=II[2]; I3=II[3]; I4=II[4]; I5=II[5]; } else { float s=1.0f/vmax((2.0f/5.0f)*World.mass[i]*World.radius[i]*World.radius[i],1e-6f); I0=I1=I2=s; I3=I4=I5=0.0f; } for (int r=0;r<3;++r) for (int c=0;c<3;++c) K[r][c]=R[r][0]*(I0*R[c][0]+I3*R[c][1]+I4*R[c][2])+R[r][1]*(I3*R[c][0]+I1*R[c][1]+I5*R[c][2])+R[r][2]*(I4*R[c][0]+I5*R[c][1]+I2*R[c][2]); }
INLINE V3 M33_v(const float M[3][3], V3 v) { return (V3){M[0][0]*v.x+M[0][1]*v.y+M[0][2]*v.z, M[1][0]*v.x+M[1][1]*v.y+M[1][2]*v.z, M[2][0]*v.x+M[2][1]*v.y+M[2][2]*v.z}; }
V3 ApplyInvTensor(u16 i, V3 v, const float R[3][3]) {
    if (World.col[i] == COLTYPE_BOX) {
        ShapeBox b = Entity_GetBox(i); float m = World.mass[i], hx = b.hExt.x, hy = b.hExt.y, hz = b.hExt.z;
        float Ixx=(1.0f/3.0f)*m*(hy*hy+hz*hz), Iyy=(1.0f/3.0f)*m*(hx*hx+hz*hz), Izz=(1.0f/3.0f)*m*(hx*hx+hy*hy), invIxx=1.0f/vmax(Ixx,1e-6f), invIyy=1.0f/vmax(Iyy,1e-6f), invIzz=1.0f/vmax(Izz,1e-6f);
        float bx=R[0][0]*v.x+R[1][0]*v.y+R[2][0]*v.z, by=R[0][1]*v.x+R[1][1]*v.y+R[2][1]*v.z, bz=R[0][2]*v.x+R[1][2]*v.y+R[2][2]*v.z; float wx=invIxx*bx, wy=invIyy*by, wz=invIzz*bz; 
        return (V3){R[0][0]*wx+R[0][1]*wy+R[0][2]*wz, R[1][0]*wx+R[1][1]*wy+R[1][2]*wz, R[2][0]*wx+R[2][1]*wy+R[2][2]*wz};
    }
    if (World.col[i] != COLTYPE_CVX || !World.invTnsrValid[i]) { float r=World.radius[i]; return V3_ScaleByF(v,1.0f/vmax((2.0f/5.0f)*World.mass[i]*r*r,0.0f)); }
    float *I=World.invInertiaTensor[i];
    float bx=R[0][0]*v.x+R[1][0]*v.y+R[2][0]*v.z, by=R[0][1]*v.x+R[1][1]*v.y+R[2][1]*v.z, bz=R[0][2]*v.x+R[1][2]*v.y+R[2][2]*v.z;
    float wx=I[0]*bx+I[3]*by+I[4]*bz, wy=I[3]*bx+I[1]*by+I[5]*bz, wz=I[4]*bx+I[5]*by+I[2]*bz; 
    return (V3){R[0][0]*wx+R[0][1]*wy+R[0][2]*wz, R[1][0]*wx+R[1][1]*wy+R[1][2]*wz, R[2][0]*wx+R[2][1]*wy+R[2][2]*wz};
}

void SolveGlobalContacts(void) { // PGS over the FULL contact set queued this substep.
    for (int it=0;it<SOLVER_ITER_GLOBAL;++it) {
        float maxDelta = 0.0f;
        for (u32 c=0;c<gContactCount;++c) {
            SolverContact *sc=&gContacts[c];
            for (int p=0;p<sc->m.n;++p) {
                u16 a = sc->a, b = sc->b;
                V3 n = sc->m.normal;
                V3 rAarm = sc->rA[p], rBarm = sc->rB[p];
                float targetVn = sc->targetVn[p];
                float *accumN = &sc->accumN[p], *accumT = &sc->accumT[p];
                bool bStatic = sc->bStatic;
                float invMassA = sc->invMassA, invMassB = sc->invMassB;
                float invSumN = sc->invSumN[p];
                bool canRotateA = sc->canRotateA, canRotateB = sc->canRotateB;
                const float (*Ka)[3] = sc->Ka;
                const float (*Kb)[3] = sc->Kb;
                if (invSumN < PHY_EPSILON) continue;
                V3 vAtA = V3_AplusB(World.velocity[a],V3_Cross(World.angularVelocity[a],rAarm)), vAtB = bStatic ? (V3){0,0,0} : V3_AplusB(World.velocity[b],V3_Cross(World.angularVelocity[b],rBarm));
                float vn = V3_dot(V3_AsubB(vAtA,vAtB),n), j = (targetVn - vn) / invSumN, newAccumN = vmax(*accumN + j, 0.0f); 
                j = newAccumN - *accumN; 
                *accumN = newAccumN;
                float deltaVn = j * invSumN;
                if (deltaVn < 0.0f) deltaVn = -deltaVn;
                if (deltaVn > maxDelta) maxDelta = deltaVn;
                V3 impulse = V3_ScaleByF(n,j); World.velocity[a] = V3_AplusB(World.velocity[a],V3_ScaleByF(impulse,invMassA));
                if (!bStatic) World.velocity[b] = V3_AsubB(World.velocity[b],V3_ScaleByF(impulse,invMassB));
                if (canRotateA) World.angularVelocity[a] = V3_AplusB(World.angularVelocity[a],M33_v(Ka,V3_Cross(rAarm,impulse)));
                if (canRotateB) World.angularVelocity[b] = V3_AsubB(World.angularVelocity[b],M33_v(Kb,V3_Cross(rBarm,impulse)));
                V3 vAtA2 = V3_AplusB(World.velocity[a],V3_Cross(World.angularVelocity[a],rAarm)), vAtB2 = bStatic ? (V3){0,0,0} : V3_AplusB(World.velocity[b],V3_Cross(World.angularVelocity[b],rBarm));
                V3 relVel2 = V3_AsubB(vAtA2,vAtB2), tangent = V3_AsubB(relVel2,V3_ScaleByF(n,V3_dot(relVel2,n))); float tLen = V3_Mag(tangent);
                if (tLen > 0.0001f) {
                    tangent = V3_ScaleByF(tangent,1.0f/tLen); V3 rAxT = V3_Cross(rAarm,tangent), rBxT = V3_Cross(rBarm,tangent);
                    float angTermAT = canRotateA ? V3_dot(rAxT,M33_v(Ka,rAxT)) : 0.0f, angTermBT = canRotateB ? V3_dot(rBxT,M33_v(Kb,rBxT)) : 0.0f, invSumT = invMassA + invMassB + angTermAT + angTermBT;
                    if (invSumT > PHY_EPSILON) {
                        float jt = -V3_dot(relVel2,tangent) / invSumT, friction; bool aIsSpecial = (World.col[a] == COLTYPE_CAP && (a == PLAYER1 || IdxIsNPC(World.instances[a].index)));
                        if (bStatic && aIsSpecial) { friction = 0.001f; } else { float mix = vclamp((tLen - 0.005f) / 0.10f, 0.0f, 1.0f); friction = 0.8f + mix * (0.6f - 0.8f); }
                        float maxT = friction * (*accumN), newAccumT = vclamp(*accumT + jt, -maxT, maxT); 
                        jt = newAccumT - *accumT; 
                        *accumT = newAccumT;
                        float deltaVt = jt * invSumT;
                        if (deltaVt < 0.0f) deltaVt = -deltaVt;
                        if (deltaVt > maxDelta) maxDelta = deltaVt;
                        V3 fImpulse = V3_ScaleByF(tangent,jt); World.velocity[a] = V3_AplusB(World.velocity[a],V3_ScaleByF(fImpulse,invMassA));
                        if (!bStatic) World.velocity[b] = V3_AsubB(World.velocity[b],V3_ScaleByF(fImpulse,invMassB));
                        if (canRotateA) World.angularVelocity[a] = V3_AplusB(World.angularVelocity[a],M33_v(Ka,V3_Cross(rAarm,fImpulse)));
                        if (canRotateB) World.angularVelocity[b] = V3_AsubB(World.angularVelocity[b],M33_v(Kb,V3_Cross(rBarm,fImpulse)));
                    }
                }
            }
        }
        if (maxDelta < 0.005f) break; // Don't use all iters if not needed.
    }
}

void DrawSphereContact(V3 pos, float rad);
void PrepareSolverContact(u16 a, u16 b, const Manifold *m, float dt) {
    if (!m->n || (World.col[b] == COLTYPE_MSH && World.col[a] == COLTYPE_MSH)) return;
    if (gContactCount >= MAX_GLOBAL_CONTACTS) { DualLogWarn("Ran out of global contact slots!\n"); return; }
    SolverContact *sc = &gContacts[gContactCount++]; sc->a=a; sc->b=b; sc->m=*m;
    sc->bStatic = (!(World.instances[b].entflags & EF_RIGIDBODY) || World.mass[b] < 0.001f || World.col[b] == COLTYPE_NONE || World.col[b] == COLTYPE_MSH || World.physSleep[b]);
    for (int i=0;i<m->n;++i) { if(m->p[i].pen > 0.0f){DrawSphereContact(m->p[i].point,0.02f);} }
    quat_to_mat3(World.rotation[a],sc->Ra); BuildInvInertiaMatrix(a,sc->Ra,sc->Ka);
    if (!sc->bStatic) { quat_to_mat3(World.rotation[b],sc->Rb); BuildInvInertiaMatrix(b,sc->Rb,sc->Kb); }
    sc->invMassA = World.mass[a] < 0.001f ? 1.0f : 1.0f / World.mass[a]; sc->invMassB = (sc->bStatic || World.mass[b] < 0.001f) ? 0.0f : 1.0f / World.mass[b];
    sc->canRotateA = (World.col[a] != COLTYPE_CAP && !IdxIsNPC(World.instances[a].index)); sc->canRotateB = (!sc->bStatic && World.col[b] != COLTYPE_CAP && !IdxIsNPC(World.instances[b].index));
    float bouncinessA = (World.instances[a].index == 485/*proj_plasmarifle_shot*/ ? 0.9f : 0.3f);
    float bouncinessB = (World.instances[b].index == 485/*proj_plasmarifle_shot*/ ? 0.9f : 0.3f); if (sc->bStatic) bouncinessB = 0.0f;
    if (a == PLAYER1 || b == PLAYER1 || IdxIsNPC(a) || IdxIsNPC(b)) bouncinessA = bouncinessB = 0.0f;
    for (int i=0;i<m->n;++i) {
        sc->rA[i] = V3_AsubB(m->p[i].point,World.position[a]); sc->rB[i] = sc->bStatic ? (V3){0,0,0} : V3_AsubB(m->p[i].point,World.position[b]);
        V3 vAtA = V3_AplusB(World.velocity[a],V3_Cross(World.angularVelocity[a],sc->rA[i])), vAtB = sc->bStatic ? (V3){0,0,0} : V3_AplusB(World.velocity[b],V3_Cross(World.angularVelocity[b],sc->rB[i]));
        float vn0 = V3_dot(V3_AsubB(vAtA,vAtB),m->normal), e_r = (vn0 < -0.5f) ? vmax(bouncinessA,bouncinessB) : 0.0f;
        sc->targetVn[i] = (vn0 < -0.5f) ? -e_r * vn0 : 0.0f;
        sc->targetVn[i] += 0.22f * vmax(m->p[i].pen - 0.06f, 0.0f) / dt; // per-point Baumgarte bias, frozen at gather time
        V3 rAxN = V3_Cross(sc->rA[i],m->normal), rBxN = V3_Cross(sc->rB[i],m->normal);
        sc->invSumN[i] = sc->invMassA + sc->invMassB + (sc->canRotateA ? V3_dot(rAxN,M33_v(sc->Ka,rAxN)) : 0.0f) + (sc->canRotateB ? V3_dot(rBxN,M33_v(sc->Kb,rBxN)) : 0.0f);
        sc->accumN[i] = 0.0f; sc->accumT[i] = 0.0f; // no warm-starting across substeps
    }
}

void EntityColliderMatrixNow(u16 i, float M[16]) { // Convex meshes need to keep their matrix4x4 up to date.
    Quaternion q = World.rotation[i]; V3 sx = V3_ScaleByF(quat_rot_v3(q,(V3){1,0,0}),World.scale[i].x); V3 sy = V3_ScaleByF(quat_rot_v3(q,(V3){0,1,0}),World.scale[i].y); V3 sz = V3_ScaleByF(quat_rot_v3(q,(V3){0,0,1}),World.scale[i].z); V3 p = World.position[i];
    M[0]=sx.x; M[1]=sx.y; M[2]=sx.z; M[3]=0.0f; M[4]=sy.x; M[5]=sy.y; M[6]=sy.z; M[7]=0.0f; M[8]=sz.x; M[9]=sz.y; M[10]=sz.z; M[11]=0.0f; M[12]=p.x; M[13]=p.y; M[14]=p.z; M[15]=1.0f;
}

bool PointInOBB(V3 pt, ShapeBox box) {
    V3 d=V3_AsubB(pt,box.ctr); V3 ax=quat_rot_v3(box.rot,(V3){1,0,0}), ay=quat_rot_v3(box.rot,(V3){0,1,0}), az=quat_rot_v3(box.rot,(V3){0,0,1}); float lx = V3_dot(d,ax), ly = V3_dot(d,ay), lz = V3_dot(d,az);
    return (vabs(lx) <= box.hExt.x + 0.001f) && (vabs(ly) <= box.hExt.y + 0.001f) && (vabs(lz) <= box.hExt.z + 0.001f);
}

INLINE int V3_IsSane(V3 v) { union { float f; u32 i; } ux,uy,uz; ux.f = v.x; uy.f = v.y; uz.f = v.z; return !(((ux.i & 0x7FFFFFFF) >= 0x7F800000) | ((uy.i & 0x7FFFFFFF) >= 0x7F800000) | ((uz.i & 0x7FFFFFFF) >= 0x7F800000)); }
u16 triggerVolumes[128]; u16 numTriggers; extern double game_actual_start_time;
void DrawBoxColliderColored(u16 i, Color col);
void Physics(float dt) {
    for (u16 i=0;i<World.instCount;++i) flag_set(&World.instances[i].entflags,EF_MOVING,false);
    World.substeps = (u8)vclamp((u32)(dt / MAX_STEP_SIZE + 0.5f),1u,(u32)40); float dtsub = dt / (float)World.substeps; dynamicEntityCount = 0;
    for (u16 i=0;i<World.instCount && dynamicEntityCount < 512;++i) {
        if (World.col[i] == COLTYPE_MSH || World.col[i] == COLTYPE_CVX) { World.radius[i] = modelBounds[World.col[i] == COLTYPE_CVX ? World.instances[i].colMeshIndex : World.instances[i].modelIndex] * vmax(vmax(World.scale[i].x,World.scale[i].y),World.scale[i].z); }
        else if (likely(World.col[i] == COLTYPE_BOX)) { float hx = World.colliderSize[i].x * 0.5f * World.scale[i].x, hy = World.colliderSize[i].y * 0.5f * World.scale[i].y, hz = World.colliderSize[i].z * 0.5f * World.scale[i].z; World.radius[i] = vsqrtf(hx * hx + hy * hy + hz * hz); }
        else if (World.col[i] == COLTYPE_SPH || World.col[i] == COLTYPE_CAP) { World.radius[i] = vmax(World.colliderSize[i].x,World.colliderSize[i].y) * vmax(World.scale[i].x,vmax(World.scale[i].y,World.scale[i].z)); }
        else World.radius[i] = World.colliderSize[i].x * vmax(World.scale[i].x,vmax(World.scale[i].y,World.scale[i].z));
        if ((World.instances[i].entflags & EF_RIGIDBODY) && (World.instances[i].entflags & EF_ACTIVE) && !(World.physSleep[i]) && World.col[i] != COLTYPE_NONE && vabs(World.scale[i].x) > 0.01f && vabs(World.scale[i].y) > 0.01f && vabs(World.scale[i].z) > 0.01f) {dynamicEntities[dynamicEntityCount++]=i;}
    }
    for (u8 s=0;s<World.substeps;++s) {
        mset(cellCounts,0,sizeof(cellCounts)); numTriggers=0;
        for (u16 t=0;t<128;++t) triggerVolumes[t]=0xFFFF;
        for (u16 i=0;i<World.instCount;++i) { // 0. Broadphase cell lists
            posBudget[i] = 0.64f; World.instances[i].cellX=(i16)PosGetCellCoordX(World.position[i].x); World.instances[i].cellZ=(i16)PosGetCellCoordZ(World.position[i].z);
            World.instances[i].cellIndex=PosGetCellCoordsP(World.instances[i].cellX,World.instances[i].cellZ);
            u32 cell=(u32)World.instances[i].cellIndex; if(cell < WORLDX*WORLDX && cellCounts[cell] < 128){cellLists[cell][cellCounts[cell]++]=i;}
            u16 idx=World.instances[i].index;
            if (unlikely(((idx >= 595 && idx <= 601) || idx == 746) && (World.instances[i].entflags & EF_ACTIVE) && numTriggers < 128)) triggerVolumes[numTriggers++] = i;
        }
        if (numTriggers >= 127) DualLogWarn("Ran out of triggers!\n");
        gContactCount = 0;
        for (u16 i=0;i<dynamicEntityCount;++i) { // 1. Integrate velocity
            u16 a=dynamicEntities[i]; V3 acc = {0.0f,-9.81f * World.gravity[a],0.0f}; if ((a == PLAYER1) && (Cheats.noclip || World.invP1.ladderState > 0)) acc.y = 0.0f;
            acc = V3_AplusB(acc,V3_ScaleByF(World.instances[a].accumulatedForce,1.0f / World.mass[a])); World.velocity[a] = V3_AplusB(World.velocity[a],V3_ScaleByF(acc,dtsub));
            if (!V3_IsSane(World.velocity[a])) { World.velocity[a]=(V3){0.0f,0.0f,0.0f}; }
            else { float speed=V3_Mag(World.velocity[a]); if (speed > MAX_SPEED) World.velocity[a]=V3_ScaleByF(World.velocity[a],MAX_SPEED / speed); }
            float linDrag = vexp(-0.1f * dtsub); World.velocity[a].x*=linDrag; /*Y axis left unaffected, so gravity accumulates*/ World.velocity[a].z*=linDrag; if (Cheats.noclip) World.velocity[a].y*=linDrag*linDrag;
            float angDrag = vexp(-2.0f * dtsub); World.angularVelocity[a]=V3_ScaleByF(World.angularVelocity[a],angDrag);
            SetPosition(a,V3_AplusB(World.position[a],V3_ScaleByF(World.velocity[a],dtsub)));
            if (World.col[a] != COLTYPE_CAP) {
                if (unlikely(!V3_IsSane(World.angularVelocity[a]))) { World.angularVelocity[a] = (V3){0.0f,0.0f,0.0f}; }
                else {
                    float avel = V3_Mag(World.angularVelocity[a]); if (avel > MAX_ANGULAR_SPEED) { World.angularVelocity[a] = V3_ScaleByF(World.angularVelocity[a],MAX_ANGULAR_SPEED / avel); avel = MAX_ANGULAR_SPEED; }
                    if (avel > PHY_EPSILON) { Quaternion dq = quat_from_axis_angle(V3_ScaleByF(World.angularVelocity[a],1.f / avel),avel * dtsub); World.rotation[a] = quat_normalize(quat_multiply(dq,World.rotation[a])); }
                }
            } else World.angularVelocity[a] = (V3){0.0f,0.0f,0.0f};
        }
        for (u16 i=0;i<dynamicEntityCount;++i) {
            u16 a = dynamicEntities[i]; if (unlikely(World.col[a] == COLTYPE_MSH || (Cheats.noclip && a == PLAYER1))) continue;
            u8 colA = World.col[a]; ShapeBox boxA = colA==COLTYPE_BOX ? Entity_GetBox(a) : (ShapeBox){0}; ShapeCapsule capA = colA==COLTYPE_CAP ? Entity_GetCap(a) : (ShapeCapsule){0}; ShapeSphere sphA = colA==COLTYPE_SPH ? Entity_GetSph(a) : (ShapeSphere){0};
            i32 cx = PosGetCellCoordX(World.position[a].x), cz = PosGetCellCoordZ(World.position[a].z); u32 mask = GetCollisionMask(World.layer[a]);
            float searchRad = World.radius[a] + V3_Mag(World.velocity[a]) * dtsub; i32 radCells = vmax((i32)(searchRad / CELLSZ),1);
            float matA[16]; const float *mxA = &world_from_mdl[a*16];
            if (World.col[a] == COLTYPE_CVX) { EntityColliderMatrixNow(a,matA); mxA = matA; } // Not MSH as only CVX is dynamically moving during physics substeps.
            Manifold contactsMani[32]; u16 contactsOther[32]; int contactCount = 0;
            for (i32 dx = -radCells; dx <= radCells; ++dx) {
                for (i32 dz = -radCells; dz <= radCells; ++dz) { // 2. Collisions
                    u32 cell = PosGetCellCoordsP(cx + dx,cz + dz);
                    for (u16 k = 0; k < cellCounts[cell]; ++k) {
                        u16 b = cellLists[cell][k]; if (b == a || b >= World.instCount) continue;
                        u8 colB = World.col[b]; ShapeBox boxB = colB==COLTYPE_BOX ? Entity_GetBox(b) : (ShapeBox){0}; ShapeCapsule capB = colB==COLTYPE_CAP ? Entity_GetCap(b) : (ShapeCapsule){0}; ShapeSphere sphB = colB==COLTYPE_SPH ? Entity_GetSph(b) : (ShapeSphere){0};
                        if (unlikely(Cheats.noclip && b == PLAYER1)) continue;
                        if (!(mask & World.layer[b]) || World.col[b] == COLTYPE_NONE) continue;
                        if (unlikely((World.instances[b].entflags & EF_RIGIDBODY) && !World.physSleep[b] && b > a)) continue; // Prevent doubled restitutions; asleep b handled as static collider
                        V3 deltaPos = V3_AsubB(World.position[a],World.position[b]); float rr = (World.radius[a] + World.radius[b]) + 1.28f/*One chunk extent*/; if (V3_dot(deltaPos,deltaPos) > rr * rr) continue;
                        Manifold mf = {0}; float matB[16]; const float *mxB = &world_from_mdl[b*16];
                        if (World.col[b] == COLTYPE_CVX) { EntityColliderMatrixNow(b,matB); mxB = matB; }
                        if      (World.col[a] == COLTYPE_CAP && World.col[b] == COLTYPE_CAP) { mf = OverlapToManifold(CapCap(capA,capB)); }
                        else if (World.col[a] == COLTYPE_CAP && World.col[b] == COLTYPE_BOX) { mf = OverlapToManifold(CapBox(capA,boxB)); }
                        else if (World.col[a] == COLTYPE_CAP && World.col[b] == COLTYPE_SPH) { Overlap r=SphCap(sphB,capA); if(r.hit) r.normal=V3_ScaleByF(r.normal,-1.f); mf=OverlapToManifold(r); }
                        else if (World.col[a] == COLTYPE_SPH && World.col[b] == COLTYPE_CAP) { mf=OverlapToManifold(SphCap(sphA,capB)); }
                        else if (World.col[a] == COLTYPE_BOX && World.col[b] == COLTYPE_CAP) { Overlap r = CapBox(capB,boxA); if(r.hit) r.normal=V3_ScaleByF(r.normal,-1.0f); mf=OverlapToManifold(r); }
                        else if (World.col[a] == COLTYPE_BOX && World.col[b] == COLTYPE_BOX) { mf = BoxBox(boxA,boxB); }
                        else if (World.col[a] == COLTYPE_SPH && World.col[b] == COLTYPE_BOX) { ShapeSphere sa = sphA; mf = OverlapToManifold(SphBox(sa.ctr,sa.rad,boxB)); }
                        else if (World.col[a] == COLTYPE_BOX && World.col[b] == COLTYPE_SPH) { ShapeSphere sa = sphB; Overlap r = SphBox(sa.ctr,sa.rad,boxA); if(r.hit) r.normal=V3_ScaleByF(r.normal,-1.0f); mf=OverlapToManifold(r); }
                        else if (World.col[a] == COLTYPE_SPH && World.col[b] == COLTYPE_SPH) { ShapeSphere sa = sphA, sb = sphB; mf = OverlapToManifold(SphSph(sa.ctr,sa.rad,sb.ctr,sb.rad)); }
                        else if (World.col[a] == COLTYPE_CAP && World.col[b] == COLTYPE_MSH) { mf = OverlapToManifold(CapMsh(capA,World.instances[b].modelIndex,mxB)); }
                        else if (World.col[a] == COLTYPE_SPH && World.col[b] == COLTYPE_MSH) { ShapeSphere sa = sphA; mf = OverlapToManifold(SphMsh(sa.ctr,sa.rad,World.instances[b].modelIndex,mxB)); }
                        else if (World.col[a] == COLTYPE_BOX && World.col[b] == COLTYPE_MSH) { mf = BoxMsh(boxA,World.instances[b].modelIndex,mxB); }
                        else if (World.col[a] == COLTYPE_CVX && World.col[b] == COLTYPE_MSH) { mf = CvxMsh(World.instances[a].colMeshIndex,mxA,World.instances[b].modelIndex,mxB,World.instances[a].adjacencyIdx); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.col[a] == COLTYPE_CAP && World.col[b] == COLTYPE_CVX) { mf = PrimitiveCvx(a,World.instances[b].colMeshIndex,mxB,World.instances[b].adjacencyIdx); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.col[a] == COLTYPE_CVX && World.col[b] == COLTYPE_CAP) { mf = PrimitiveCvx(b,World.instances[a].colMeshIndex,mxA,World.instances[a].adjacencyIdx); }
                        else if (World.col[a] == COLTYPE_SPH && World.col[b] == COLTYPE_CVX) { mf = PrimitiveCvx(a,World.instances[b].colMeshIndex,mxB,World.instances[b].adjacencyIdx); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.col[a] == COLTYPE_CVX && World.col[b] == COLTYPE_SPH) { mf = PrimitiveCvx(b,World.instances[a].colMeshIndex,mxA,World.instances[a].adjacencyIdx); }
                        else if (World.col[a] == COLTYPE_BOX && World.col[b] == COLTYPE_CVX) { mf = PrimitiveCvx(a,World.instances[b].colMeshIndex,mxB,World.instances[b].adjacencyIdx); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else if (World.col[a] == COLTYPE_CVX && World.col[b] == COLTYPE_BOX) { mf = PrimitiveCvx(b,World.instances[a].colMeshIndex,mxA,World.instances[a].adjacencyIdx); }
                        else if (World.col[a] == COLTYPE_CVX && World.col[b] == COLTYPE_CVX) { mf = CvxCvx(World.instances[a].colMeshIndex,World.instances[b].colMeshIndex,mxA,mxB,World.instances[a].adjacencyIdx,World.instances[b].adjacencyIdx); if(mf.n) mf.normal=V3_ScaleByF(mf.normal,-1.0f); }
                        else { mf=OverlapToManifold(SphSph(World.position[a],World.colliderSize[a].x,World.position[b],World.colliderSize[b].x)); }
                        if (likely(mf.n && contactCount < 32)) { contactsMani[contactCount] = mf; contactsOther[contactCount] = b; contactCount++; }
                    }
                }
            }
            World.colliding[a]=false; flag_set(&World.instances[a].entflags,EF_GROUNDED,false);
            for (int c = 0; c < contactCount; ++c) {
                Manifold *mfp=&contactsMani[c]; World.colliding[a]=World.colliding[contactsOther[c]]=true; if (V3_dot(mfp->normal,(V3){0.0f,1.0f,0.0f})>=0.574f) {World.instances[a].entflags |= EF_GROUNDED;} PrepareSolverContact(a,contactsOther[c],mfp,dt);
            }
            World.instances[a].accumulatedForce = (V3){0.0f,0.0f,0.0f};
        }
        SolveGlobalContacts(); // 3. Restitution
        for (u32 c=0; c<gContactCount; ++c) { // 3.5 Positional Correction (Projection)
            SolverContact *sc = &gContacts[c];
            float avgPen = 0.0f;
            for (int p=0; p<sc->m.n; ++p) avgPen += sc->m.p[p].pen;
            if (sc->m.n > 0) avgPen /= (float)sc->m.n;
            float correction = vmax(avgPen - 0.005f, 0.0f) * 0.4f;
            float massDiv = sc->invMassA + sc->invMassB + PHY_EPSILON;
            SetPosition(sc->a, V3_AplusB(World.position[sc->a], V3_ScaleByF(sc->m.normal, correction * sc->invMassA / massDiv)));
            if (!sc->bStatic) SetPosition(sc->b, V3_AsubB(World.position[sc->b], V3_ScaleByF(sc->m.normal, correction * sc->invMassB / massDiv)));
        }
        for (u16 i=0;i<numTriggers;++i) {
            u16 self = triggerVolumes[i];
            u16 trigdx=World.instances[self].index;
            if (Cheats.showPhys) DrawBoxColliderColored(self,(Color){1.0f,0.642f,0.0f,0.5f});
            for (u16 o=0;o<dynamicEntityCount;++o) { // 4. Triggers
                u16 other = dynamicEntities[o]; if (World.col[other] == COLTYPE_NONE || !(World.instances[other].entflags & EF_ACTIVE)) continue;
                if (!PointInOBB(World.position[other],Entity_GetBox(self))) continue;
                if (other != PLAYER1 && trigdx == 596) { trigger_gravitylift_touch(self,other); continue; }
                World.Sys_Music.cyberTube = false; World.gravity[PLAYER1] = 1.0f; World.invP1.ladderState=0; World.Sys_Music.inZone = World.Sys_Music.elevator = World.Sys_Music.distortion = false; // Reset trigger sustained flags
                switch(trigdx) {
                    case 595/*trigger_cyberpush*/:   trigger_cyberpush_touch(self,other); break;
                    case 596/*trigger_gravitylift*/: trigger_gravitylift_touch(self,other); break;
                    case 597/*trigger_ladder*/:      World.invP1.ladderState=1; break;
                    case 598/*trigger_multiple*/: case 600/*trigger_once*/: TriggerTriggerTripped(self,other); break;
                    case 599/*trigger_music*/: { TrackType tt=World.instances[self].trackType; World.Sys_Music.inZone=true; World.Sys_Music.elevator=(tt == TT_Elevator); World.Sys_Music.distortion=(tt == TT_Distortion); break; }
                    case 601/*trigger_radiation*/:            World.invP1.radiationArea=true;World.instances[PLAYER1].radiation=World.instances[self].radiation; break; // TODO bleedoff when !radiationArea, amelioration from envirosuit, detox patch negation for 30secs
                    case 746/*weapon_grenadeenergmine_live*/: TakeEnergy(256.0f); break;
                }
            }
        }
    }
    {
        const float SLEEP_LIN2 = 0.0025f;   // (0.05 m/s)^2
        const float SLEEP_ANG2 = 0.0025f;   // (0.05 rad/s)^2
        const i32 WAKE_CELLS = 2;
        for (u32 i=0;i<World.instCount;++i) {
            if (AnimWaking(i)) flag_set(&World.instances[i].entflags,EF_MOVING,true);
            u32 ef = World.instances[i].entflags;
            bool canSleep = (i!=PLAYER1) && (ef & EF_RIGIDBODY) && (ef & EF_ACTIVE) && (World.col[i]!=COLTYPE_NONE) && (World.mass[i] >= 0.001f);
            if (!canSleep) { World.physSleep[i]=0; continue; }
            i32 cx = PosGetCellCoordX(World.position[i].x), cz = PosGetCellCoordZ(World.position[i].z);
            bool nearAwake = false;
            for (i32 dx=-WAKE_CELLS; dx<=WAKE_CELLS && !nearAwake; ++dx)
              for (i32 dz=-WAKE_CELLS; dz<=WAKE_CELLS && !nearAwake; ++dz) {
                u32 cl = PosGetCellCoordsP(cx+dx,cz+dz);
                for (u16 k=0;k<cellCounts[cl];++k) {
                    u16 j = cellLists[cl][k]; if (j==i) continue;
                    u32 ej = World.instances[j].entflags;
                    if (World.physSleep[j] || !(ej & EF_ACTIVE)) continue; // asleep/inactive bodies don't wake others
                    float sj2 = V3_dot(World.velocity[j], World.velocity[j]);
                    bool jMoving = sj2 > SLEEP_LIN2;
                    bool jAnimWaking = AnimWaking(j);
                    if (!(jMoving || jAnimWaking)) continue; // not a waker -> lets i sleep
                    V3 d = V3_AsubB(World.position[i], World.position[j]);
                    float rr = World.radius[i] + World.radius[j] + (jMoving ? 2.0f * vsqrtf(sj2) : 0.0f); // mover reach only when actually translating
                    if (V3_dot(d,d) < rr*rr) { nearAwake=true; break; }
                }
            }
            if (World.physSleep[i]) { if (nearAwake) World.physSleep[i]=0; }
            else if (!nearAwake && (ef & EF_GROUNDED)) {
                float sp2 = V3_dot(World.velocity[i],World.velocity[i]), asp2 = V3_dot(World.angularVelocity[i],World.angularVelocity[i]);
                if (sp2 < SLEEP_LIN2 && asp2 < SLEEP_ANG2) { World.physSleep[i]=1; World.velocity[i]=(V3){0,0,0}; World.angularVelocity[i]=(V3){0,0,0}; }
            }
        }
    }
}

void AddForce(u16 i, V3 f, bool imp) { if (imp) { World.velocity[i] = V3_AplusB(World.velocity[i],V3_ScaleByF(f,1.0f / vmax(World.mass[i],0.001f))); } else { World.instances[i].accumulatedForce = V3_AplusB(World.instances[i].accumulatedForce,f); } }
INLINE float smooth_damp(float cur, float targ, float* vel, float tm, float dt) { float o=2.0f / vmax(tm,0.0001f); float x=o * dt; float exp=1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x); float d=cur - targ; float t=(*vel + o * d) * dt; *vel=(*vel - o * t) * exp; return targ + (d + t) * exp; }
bool CantStand(u16 playerIdx, float targetHeight) { // I can't stand it.
    float oldHeight = World.colliderSize[playerIdx].y; V3 oldPos = World.position[playerIdx];
    World.colliderSize[playerIdx].y = targetHeight; World.position[playerIdx].y += (targetHeight - oldHeight); // Temporarily morph player into the standing capsule
    bool blocked = false; i32 cx=PosGetCellCoordX(World.position[playerIdx].x), cz=PosGetCellCoordZ(World.position[playerIdx].z); u32 mask=GetCollisionMask(World.layer[playerIdx]);
    for (i32 dx = -1; dx <= 1 && !blocked; ++dx) {
        for (i32 dz = -1; dz <= 1 && !blocked; ++dz) {
            u32 cell = PosGetCellCoordsP(cx + dx, cz + dz);
            for (u16 k = 0; k < cellCounts[cell]; ++k) {
                u16 b = cellLists[cell][k]; if (b == playerIdx || !(mask & World.layer[b]) || World.col[b] == COLTYPE_NONE) continue;
                if (World.col[b] == COLTYPE_MSH) { Overlap r = CapMsh(Entity_GetCap(playerIdx),World.instances[b].modelIndex,&world_from_mdl[b*16]); if (r.hit && r.pen > 0.08f) { blocked = true; break; } }
            }
        }
    }
    World.colliderSize[playerIdx].y = oldHeight; World.position[playerIdx] = oldPos; return blocked;
}

KeyState* GetCodeMapping(int settingIndex);
void ApplyPlayerMovements(float dt) {
    Entity *p = &World.instances[PLAYER1]; Quaternion r = World.rotation[PLAYER1]; float leanSpeed = 70.0f, leanMaxAngle = 35.0f; float leanInput = (float)LeanLeft() - (float)LeanRight(); bool doubleTapLean = DoubleTapLeanLeft() || DoubleTapLeanRight();
    bool movingForward = Forward() > 0.1f, leanRight = leanInput < 0.0f, leanLeft = leanInput > 0.0f;
    if (doubleTapLean) { World.invP1.leanResetting = true; World.invP1.leanVelocity = 0.0f; KeyState *kL = GetCodeMapping(7), *kR = GetCodeMapping(8); kL->pressed = kR->pressed = false; } // Double-tap lean: initiate smooth reset to upright over 0.2 seconds
    if (World.invP1.leanResetting) { 
        World.invP1.leanTarget = smooth_damp(World.invP1.leanTarget,0.0f,&World.invP1.leanVelocity,0.2f,dt); 
        if(vabs(World.invP1.leanTarget) < 0.5f){World.invP1.leanTarget=World.invP1.leanVelocity=0.0f; World.invP1.leanResetting=false;} 
    } else {
        if (leanLeft || leanRight) { if(leanLeft){World.invP1.leanRightTapFinished =0;} if(leanRight){World.invP1.leanLeftTapFinished=0;} World.invP1.leanTarget=vclamp(World.invP1.leanTarget + (leanInput * leanSpeed * dt),-leanMaxAngle,leanMaxAngle); }
        else if (movingForward) { if (vabs(World.invP1.leanTarget) < 0.5f) { World.invP1.leanTarget = 0.0f; } else { World.invP1.leanTarget -= (World.invP1.leanTarget > 0.0f ? 1.0f : -1.0f) * leanSpeed * dt; } }
    }
    World.cam_roll = World.invP1.leanTarget;
    float targetRatio=1.0f, transitionSec=0.2f; float currentRatio=World.invP1.currentCrouchRatio;
    if (Crouch()) { // Crouch key always targets crouch ratio from any state
        if (p->bodyState == BodyState_Crouch) { if (!CantStand(PLAYER1,PLAYER_HEIGHT)){p->bodyState = BodyState_StandingUp;}} // Already at crouch → toggle up to standing
        else if (currentRatio > PLAYER_CROUCH_RATIO) { p->bodyState = BodyState_CrouchingDown;} // Above crouch → go down to crouch (handles "if standing up will go back to crouched")
        else {p->bodyState=BodyState_ProningUp;} // Below crouch → go up to crouch (handles "if proning down will go back to crouched")
    } else if (Prone()) {
        if (p->bodyState == BodyState_Standing) { p->bodyState = BodyState_ProningDown; } // Standing → go to prone
        else if (currentRatio > PLAYER_CROUCH_RATIO) { if (!CantStand(PLAYER1,PLAYER_HEIGHT)){p->bodyState=BodyState_StandingUp;}else{p->bodyState = BodyState_ProningDown;} } // Between crouch and standing → up to standing
        else if (p->bodyState == BodyState_Crouch) { p->bodyState = BodyState_ProningDown; } // Crouch → go to prone
        else { p->bodyState = BodyState_ProningUp; } // Between prone and crouch, or prone → up to crouch
    }
    float fatigueWane = 1.0f; if (Cheats.fatigueCheat || World.invP1.staminupActive) World.invP1.fatigue = 0.0f;
    switch (p->bodyState) {
        case BodyState_CrouchingDown:targetRatio=-0.01f; fatigueWane = 2.0f; break; case BodyState_StandingUp:targetRatio=1.01f; fatigueWane = 2.0f; break; case BodyState_ProningDown:targetRatio=-0.01f; fatigueWane = 3.5f; break;
        case BodyState_ProningUp:targetRatio=1.01f; transitionSec+=0.1f; fatigueWane = 3.5f; break; case BodyState_Crouch:targetRatio=PLAYER_CROUCH_RATIO; fatigueWane = 2.0f; break; case BodyState_Prone:targetRatio=PLAYER_PRONE_RATIO; fatigueWane = 3.5f; break;
    }
    bool inGravLift = ((p->entflags & EF_GRAVLIFT) > 0);
    bool grounded = !inGravLift && ((p->entflags & EF_GROUNDED) > 0);
    bool jumpjettin = ((World.invP1.hasHardware & HW_JET) > 0 && (World.invP1.hardwareIsActive & HW_JET) > 0);
    if (JumpDown() && grounded && !jumpjettin) {
        if (!Cheats.noclip) {World.velocity[PLAYER1].y += (World.invP1.fatigue > 80.0f ? 2.0f : 4.51f/*;)*/) + 0.2f; if(!World.boosterActive && !Cheats.noclip){World.invP1.fatigue += 6.5f;} }
        RaycastHit jhit = Raycast(World.position[PLAYER1],(V3){0.0f,-1.0f,0.0f},2.0f,LMASK_PLAYER_FEET); FootStepType jfstp = jhit.hit ? GetFootstepTypeForPrefab(World.instances[jhit.hitInstanceIndex].index) : FSTP_Concrete; play_wav(JumpSound(jfstp),SfxVol(),World.position[PLAYER1],true);
    }
    if (Jump() && jumpjettin && World.invP1.jumpJetFinished < World.pauseRelativeTime && World.invP1.energy > 0.0f) {
        if (!Cheats.noclip) {World.velocity[PLAYER1].y += 1.3f;} World.invP1.jumpJetFinished = World.pauseRelativeTime + 0.1f;
        if (World.invP1.jumpJetSuckFinished < World.pauseRelativeTime) { World.invP1.jumpJetSuckFinished=World.pauseRelativeTime+1.0f; float energysuck = 11.0f; switch (World.invP1.hardwareVersionSetting[10]) { case 0: energysuck=11.0f; break; case 1:energysuck=26.0f; break; case 2:energysuck=22.0f; break; } TakeEnergy(energysuck); }
    }
    float lastRatio = World.invP1.currentCrouchRatio;
    World.invP1.currentCrouchRatio = smooth_damp(lastRatio,targetRatio,&World.invP1.crouchingVelocity,transitionSec,dt);
    if (World.invP1.currentCrouchRatio >= 1.0f) { World.invP1.currentCrouchRatio = 1.0f; if(p->bodyState == BodyState_StandingUp){p->bodyState=BodyState_Standing;} }
    else if (p->bodyState == BodyState_CrouchingDown && World.invP1.currentCrouchRatio <= PLAYER_CROUCH_RATIO) { World.invP1.currentCrouchRatio = PLAYER_CROUCH_RATIO; p->bodyState = BodyState_Crouch; }
    else if (p->bodyState == BodyState_ProningUp && World.invP1.currentCrouchRatio >= PLAYER_CROUCH_RATIO) { World.invP1.currentCrouchRatio = PLAYER_CROUCH_RATIO; p->bodyState = BodyState_Crouch; }
    else if (p->bodyState == BodyState_ProningDown && World.invP1.currentCrouchRatio <= PLAYER_PRONE_RATIO) { World.invP1.currentCrouchRatio = PLAYER_PRONE_RATIO; p->bodyState = BodyState_Prone; }
    World.colliderSize[PLAYER1].y = PLAYER_HEIGHT * World.invP1.currentCrouchRatio; World.colliderCenter[PLAYER1].y =  -PLAYER_CAM_OFFSET_Y + (PLAYER_HEIGHT - World.colliderSize[PLAYER1].y) * 0.5f; // Split capsule shape in the middle, camera is thus 0.16 away from top of the capsule ((2 / 2 = 1) - 0.84 which is PLAYER_CAM_OFFSET_Y)
    float h=(float)Forward() - (float)Backpedal(), s=(float)StrafeRight() - (float)StrafeLeft(), vertInput = Cheats.noclip ? (float)((SwimUp()) || Jump()) - (float)SwimDn() : 0.0;
    bool isSprinting=Sprint() && (grounded || inGravLift || World.invP1.ladderState > 0 || Cheats.noclip);
    if (World.invP1.fatigueMoveFinished < World.pauseRelativeTime && (vabs(h) > 0.0f || vabs(s) > 0.0f) && grounded && (V3_dot(World.velocity[PLAYER1],World.velocity[PLAYER1]) > 0.1f && !Cheats.noclip) && !World.boosterActive){World.invP1.fatigue += isSprinting ? 2.85f : 1.0f; World.invP1.fatigueMoveFinished=World.pauseRelativeTime + 0.298f;/*Slightly different than bleedoff to keep out of sync*/}
    float stepVolMod = fatigueWane > 3.4f ? 0.2f : fatigueWane > 1.9f ? 0.4f : 1.0f;
    float rustleVolMod = fatigueWane > 3.4f ? 0.65f : fatigueWane > 1.9f ? 0.7f : 1.0f;
    if (World.invP1.footstepFinished < World.pauseRelativeTime && (vabs(h) > 0.0f || vabs(s) > 0.0f) && grounded && (V3_dot(World.velocity[PLAYER1],World.velocity[PLAYER1]) > 0.1f && !Cheats.noclip) && !World.boosterActive) {
        RaycastHit fstep = Raycast(World.position[PLAYER1],(V3){0.0f,-1.0f,0.0f},2.0f,LMASK_PLAYER_FEET);
        FootStepType fstp = fstep.hit ? GetFootstepTypeForPrefab(World.instances[fstep.hitInstanceIndex].index) : FSTP_Concrete;
        play_wav(FootStepSound(fstp),SfxVol() * random_range(0.4f,0.55f) * stepVolMod * 0.5f,World.position[PLAYER1],true);
        World.invP1.footstepFinished = World.pauseRelativeTime + (isSprinting ? random_range(0.2f,0.3f) : random_range(0.35f,0.65f));
    }
    if (World.invP1.rustleFinished < World.pauseRelativeTime && (vabs(h) > 0.0f || vabs(s) > 0.0f) && (V3_dot(World.velocity[PLAYER1],World.velocity[PLAYER1]) > 0.1f && !Cheats.noclip) && !World.boosterActive) {
        play_wav(RustleSound(),SfxVol() * random_range(0.3f,0.5f) * rustleVolMod * 0.75f,World.position[PLAYER1],true);
        World.invP1.rustleFinished = World.pauseRelativeTime + (isSprinting ? random_range(0.4f,0.6f) : random_range(0.8f,1.2f));
    }
    float y2=r.y*r.y, xz=r.x*r.z, wy=r.w*r.y;
    p->forward=V3_Normalize((V3){ 2.0f*(xz + wy),2.0f*(r.y*r.z - r.w*r.x),1.0f - 2.0f*(r.x*r.x + y2) }); p->right=V3_Normalize((V3){ 1.0f - 2.0f*(y2 + r.z*r.z),2.0f*(r.x*r.y + r.w*r.z),2.0f*(xz - wy) });
    V3 inputDir={ p->forward.x*h + p->right.x*s,vertInput,p->forward.z*h + p->right.z*s};
    float inputLenSq = V3_dot(inputDir,inputDir);
    V3 w = (inputLenSq > 0.0001f) ? V3_ScaleByF(inputDir, 1.0f / vsqrtf(inputLenSq)) : (V3){0, 0, 0}; 
    bool isRunning = (inputLenSq > 0.01f); float speedAdjust = 0.0f; bool setSpeedAdjusted = false;
    if (Cheats.noclip) { speedAdjust = PLAYER_MAX_CYBER_SPEED*(isSprinting ? 2.5f : 1.5f); setSpeedAdjusted = true; }
    if (World.curLev==LEVEL_CYBERSPACE) { speedAdjust = PLAYER_MAX_CYBER_SPEED; setSpeedAdjusted = true; }
    BodyState b=World.instances[PLAYER1].bodyState; float v=WALK_SPEED;
    switch(b){ case BodyState_CrouchingDown: case BodyState_Crouch:v=CROUCH_SPEED; break; case BodyState_Prone: case BodyState_ProningDown: case BodyState_ProningUp:v=PLAYER_MAX_PRONE_SPEED; break; default:break; }
    if ((isSprinting||World.boosterActive) && isRunning) {
        v = World.invP1.fatigue > 80.0f && !World.boosterActive ? SPRINT_SPEED_FATIGUED : SPRINT_SPEED;
        if (b==BodyState_Standing||b==BodyState_Crouch||b==BodyState_CrouchingDown) v -= (WALK_SPEED-CROUCH_SPEED)*1.5f; else if(b==BodyState_Prone||b==BodyState_ProningDown||b==BodyState_ProningUp) v -= (WALK_SPEED-PLAYER_MAX_PRONE_SPEED)*2.f;
    }
    float speed = (setSpeedAdjusted ? speedAdjust : v + (World.boosterActive ? PLAYER_BOOSTER_SPEED_BOOST : 0.0f)) + (World.invP1.staminupActive ? 1.0f : 0.0f), accel=World.boosterActive && World.curLev!=LEVEL_CYBERSPACE ? 1.0f : 3.0f; V3 targetVel = V3_ScaleByF(w,speed); 
    if (World.invP1.ladderState > 0) { float climbSpeed = (isSprinting && isRunning) ? 3.0f : 1.3f; targetVel = (V3){p->right.x * s * speed * 0.3f, h * climbSpeed, p->right.z * s * speed * 0.3f}; accel = 5.0f; }
    else { if (vabs(vertInput) < 0.001f) { targetVel.y = World.velocity[PLAYER1].y; } }
    V3 dv = V3_AsubB(targetVel, World.velocity[PLAYER1]); 
    dv = (V3){ vclamp(dv.x, -10.0f, 10.0f), vclamp(dv.y, -10.0f, 10.0f), vclamp(dv.z,-10.0f,10.0f) };
    World.velocity[PLAYER1] = V3_AplusB(World.velocity[PLAYER1], V3_ScaleByF(dv,accel * vclamp(dt,0.0005f,0.1f)));
    if (World.invP1.fatigueBleedoffFinished < World.pauseRelativeTime && World.curLev!=LEVEL_CYBERSPACE && !Cheats.noclip) { World.invP1.fatigue -= fatigueWane; World.invP1.fatigueBleedoffFinished = World.pauseRelativeTime + 0.3f; } // Fatigue bleed off
    World.invP1.fatigue = vclamp(World.invP1.fatigue,0.0f,100.0f); // TODO textwarnings Fatigue high when > 80.0f
    if (grounded && !World.invP1.wasGrounded) {
        float velChange = vabs(World.invP1.lastVelY - World.velocity[PLAYER1].y);
        if (velChange > 2.0f) {
            RaycastHit lhit = Raycast(World.position[PLAYER1],(V3){0.0f,-1.0f,0.0f},2.0f,LMASK_PLAYER_FEET);
            FootStepType lstp = lhit.hit ? GetFootstepTypeForPrefab(World.instances[lhit.hitInstanceIndex].index) : FSTP_Concrete;
            float vol = vclamp((velChange - 1.0f) / (11.72f - 1.0f),0.0f,1.0f) * (1.0f - 0.5f) * 0.8f * stepVolMod;
            play_wav(JumpLandSound(lstp),SfxVol() * vol,World.position[PLAYER1],true);
            World.invP1.noiseFinished = World.pauseRelativeTime + 0.1f;
        }
        if (velChange >= 11.72f) {
            DamageData dd = {0};
            float falltake = 75.0f - random_range(0.0f,68.0f);
            if (falltake > World.instances[PLAYER1].health && falltake - World.instances[PLAYER1].health < 5.0f) falltake = World.instances[PLAYER1].health - 1.0f; // some small saving grace
            dd.damage = falltake; // No need for GetDamageTakeAmount since this is strictly internal to Player
            TakeDamage(PLAYER1,dd);
            World.invP1.noiseFinished = World.pauseRelativeTime + 0.2f;
        }
    }
    World.invP1.wasGrounded = grounded;
    World.invP1.lastVelY = World.velocity[PLAYER1].y;
    // TODO booster friction mod, TODO booster double tap JumpDown() burst forward, TODO cyber drift forward based on difficultyCyber (like a plane woo), TODO maxCyberUltimateSpeed clamping
}
