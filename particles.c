// particles.c - Particle System, supports subemitters (up to 8 children), soft particles, billboard projectiles, texture index++ animation, all timed against World.pauseRelativeTime.
    // // 6. Trigger effects (e.g. in GetImpactType):
    // u16 impactType = GetImpactType(instanceIdx);
    // PSys_PlayOneshot(impactType==729?0 : impactType==724?3 : impactType==723?8 : impactType==722?7 : impactType==730?9 : impactType==756?12 : impactType==757?13 : 14, hitPoint);
    // 
    // // 7. For a billboard projectile:
    // u16 proj = PSys_PlayBurstAt(6/*Projectile*/, muzzlePos, aimDir, 40.f);
#define MAX_PSYS_DEFS 64
#define MAX_PSYS_PARTICLES 16384
#define MAX_PSYS_EMITTERS 256
#define PSYS_SUB_MAX 8
enum{PSYS_ALIVE=1u,PSYS_BURST=2u,PSYS_LOOPING=4u,PSYS_PREWARMED=8u,PSYS_STOPPING=16u};
enum{PSYS_EMIT_CONTINUOUS=0,PSYS_EMIT_BURST=1};
enum{PSYS_SHAPE_CONE=0,PSYS_SHAPE_SPHERE=1,PSYS_SHAPE_BOX=2,PSYS_SHAPE_CIRCLE=3,PSYS_SHAPE_HEMI=4,PSYS_SHAPE_EDGE=5};
enum{PSYS_SUB_BIRTH=0,PSYS_SUB_DEATH=1,PSYS_SUB_COLLISION=2};
enum{PSYS_BILLBOARD_SCREEN=0,PSYS_BILLBOARD_STRETCHED=1,PSYS_BILLBOARD_HORIZ=2,PSYS_BILLBOARD_VERT=3};
enum{PSYS_SPACE_WORLD=0,PSYS_SPACE_LOCAL=1};
typedef struct { V3 pos,vel; float age,maxAge,size,startSize,endSize,rot,rotSpeed; u32 startCol,endCol; u8 texAnim,emitIdx,flags,pad; } __attribute__((aligned(64))) PsParticle;
typedef struct { u8 defIdx,flags,pad8[2]; V3 pos,lastPos; Quaternion rot; float startTime,nextEmit,interval; u32 emitted,firstP,partCount; u16 parentEnt,pad16; u8 subDefs[PSYS_SUB_MAX],subEvents[PSYS_SUB_MAX]; u8 subCnt; u8 _pad7; } PsEmitter;
typedef struct {
    const char* name; u16 maxParts,burstCnt; float life,lifeRand,startSz,szRand,endSz,endRand;
    float startSpd,spdRand,gravMod,emitRate; u32 startCol,endCol;
    float shapeAngle,shapeRad,shapeArc,prewarm,rotMin,rotMax;
    u16 texStart;
    u8 texCnt,animFPS,emitMode,shape,space,billboard,softPart,inheritVel,sortMode,pad8b,subDefs[PSYS_SUB_MAX],subEvents[PSYS_SUB_MAX],subCnt,pad8c;
} PsDef;

static const PsDef psysDefs[MAX_PSYS_DEFS] = {
    [0]={.name="Spark",     .maxParts=64,  .burstCnt=8,  .life=0.35f,.lifeRand=0.15f,.startSz=0.04f,.szRand=0.02f,.endSz=0.005f,.endRand=0.003f,.startSpd=3.5f,.spdRand=1.5f,.gravMod=0.3f,.emitRate=0,.startCol=0xFFFFD080u,.endCol=0x00FF8000u,.shapeAngle=10.f,.shapeRad=0.02f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_BURST,.texStart=817,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=0,.space=PSYS_SPACE_WORLD,.rotMin=-360.f,.rotMax=360.f},
    [1]={.name="Smoke",     .maxParts=32,  .burstCnt=4,  .life=1.2f,.lifeRand=0.6f,.startSz=0.15f,.szRand=0.08f,.endSz=0.35f,.endRand=0.15f,.startSpd=1.2f,.spdRand=0.5f,.gravMod=-0.15f,.emitRate=8.f,.startCol=0x88666666u,.endCol=0x00333333u,.shapeAngle=30.f,.shapeRad=0.1f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_CONTINUOUS,.texStart=757,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=1,.space=PSYS_SPACE_WORLD,.rotMin=-20.f,.rotMax=20.f},
    [2]={.name="Explosion", .maxParts=96,  .burstCnt=48, .life=0.8f,.lifeRand=0.4f,.startSz=0.06f,.szRand=0.04f,.endSz=0.01f,.endRand=0.01f,.startSpd=8.f,.spdRand=4.f,.gravMod=0.2f,.emitRate=0,.startCol=0xFFFFAA44u,.endCol=0x00FF4400u,.shapeAngle=180.f,.shapeRad=0.01f,.shape=PSYS_SHAPE_SPHERE,.emitMode=PSYS_EMIT_BURST,.texStart=729,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=0,.space=PSYS_SPACE_WORLD,.rotMin=-720.f,.rotMax=720.f,.subDefs={1},.subEvents={PSYS_SUB_BIRTH},.subCnt=1},
    [3]={.name="BloodSpurt",.maxParts=24,  .burstCnt=12, .life=0.5f,.lifeRand=0.2f,.startSz=0.03f,.szRand=0.02f,.endSz=0.008f,.endRand=0.004f,.startSpd=4.f,.spdRand=2.f,.gravMod=0.8f,.emitRate=0,.startCol=0xFFCC1111u,.endCol=0x00990000u,.shapeAngle=20.f,.shapeRad=0.02f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_BURST,.texStart=724,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=0,.space=PSYS_SPACE_WORLD,.rotMin=-180.f,.rotMax=180.f},
    [4]={.name="MuzzleFlash",.maxParts=8,  .burstCnt=4,  .life=0.08f,.lifeRand=0.03f,.startSz=0.12f,.szRand=0.04f,.endSz=0.06f,.endRand=0.02f,.startSpd=0.5f,.spdRand=0.3f,.gravMod=0,.emitRate=0,.startCol=0xFFFFF8C0u,.endCol=0x00FFCC00u,.shapeAngle=5.f,.shapeRad=0.03f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_BURST,.texStart=729,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=0,.space=PSYS_SPACE_WORLD,.rotMin=-90.f,.rotMax=90.f},
    [5]={.name="Trail",      .maxParts=128, .burstCnt=0,  .life=0.6f,.lifeRand=0.2f,.startSz=0.06f,.szRand=0.02f,.endSz=0.01f,.endRand=0.005f,.startSpd=0.5f,.spdRand=0.3f,.gravMod=0,.emitRate=60.f,.startCol=0x88FFAA44u,.endCol=0x00FF4400u,.shapeAngle=0,.shapeRad=0.01f,.shape=PSYS_SHAPE_CIRCLE,.emitMode=PSYS_EMIT_CONTINUOUS,.texStart=729,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=1,.space=PSYS_SPACE_WORLD,.rotMin=-90.f,.rotMax=90.f},
    [6]={.name="Projectile", .maxParts=1,  .burstCnt=1,  .life=8.f,.lifeRand=0,.startSz=0.08f,.endSz=0.08f,.startSpd=0,.spdRand=0,.gravMod=0,.emitRate=0,.startCol=0xFFFFFFFFu,.endCol=0xFFFFFFFFu,.shape=PSYS_SHAPE_EDGE,.emitMode=PSYS_EMIT_BURST,.texStart=750,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_STRETCHED,.softPart=0,.space=PSYS_SPACE_WORLD,.inheritVel=1},
    [7]={.name="GreenSpurt", .maxParts=24, .burstCnt=12, .life=0.5f,.lifeRand=0.2f,.startSz=0.03f,.szRand=0.02f,.endSz=0.008f,.endRand=0.004f,.startSpd=4.f,.spdRand=2.f,.gravMod=0.8f,.emitRate=0,.startCol=0xFF11CC11u,.endCol=0x00009900u,.shapeAngle=20.f,.shapeRad=0.02f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_BURST,.texStart=722,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=0,.space=PSYS_SPACE_WORLD,.rotMin=-180.f,.rotMax=180.f},
    [8]={.name="YellowSpurt",.maxParts=24, .burstCnt=12, .life=0.5f,.lifeRand=0.2f,.startSz=0.03f,.szRand=0.02f,.endSz=0.008f,.endRand=0.004f,.startSpd=4.f,.spdRand=2.f,.gravMod=0.8f,.emitRate=0,.startCol=0xFFCCCC11u,.endCol=0x00999900u,.shapeAngle=20.f,.shapeRad=0.02f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_BURST,.texStart=723,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=0,.space=PSYS_SPACE_WORLD,.rotMin=-180.f,.rotMax=180.f},
    [9]={.name="BlueSparks", .maxParts=64, .burstCnt=8,  .life=0.35f,.lifeRand=0.15f,.startSz=0.04f,.szRand=0.02f,.endSz=0.005f,.endRand=0.003f,.startSpd=3.5f,.spdRand=1.5f,.gravMod=0.3f,.emitRate=0,.startCol=0xFF4488FFu,.endCol=0x000044CCu,.shapeAngle=10.f,.shapeRad=0.02f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_BURST,.texStart=730,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=0,.space=PSYS_SPACE_WORLD,.rotMin=-360.f,.rotMax=360.f},
    [10]={.name="DustRing",  .maxParts=32, .burstCnt=16, .life=0.7f,.lifeRand=0.3f,.startSz=0.08f,.szRand=0.04f,.endSz=0.02f,.endRand=0.01f,.startSpd=2.f,.spdRand=1.f,.gravMod=-0.05f,.emitRate=0,.startCol=0x88888877u,.endCol=0x00444433u,.shapeAngle=0,.shapeRad=0.01f,.shape=PSYS_SHAPE_CIRCLE,.emitMode=PSYS_EMIT_BURST,.texStart=741,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=1,.space=PSYS_SPACE_WORLD,.rotMin=-45.f,.rotMax=45.f},
    [11]={.name="Fire",      .maxParts=48, .burstCnt=0,  .life=0.6f,.lifeRand=0.3f,.startSz=0.12f,.szRand=0.06f,.endSz=0.02f,.endRand=0.01f,.startSpd=2.5f,.spdRand=1.f,.gravMod=-0.25f,.emitRate=24.f,.startCol=0xFFFF8800u,.endCol=0x00FF2200u,.shapeAngle=8.f,.shapeRad=0.08f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_CONTINUOUS,.texStart=742,.texCnt=4,.animFPS=12,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=1,.space=PSYS_SPACE_WORLD,.rotMin=-30.f,.rotMax=30.f},
    [12]={.name="LeafBurst", .maxParts=16, .burstCnt=8, .life=1.f,.lifeRand=0.5f,.startSz=0.06f,.szRand=0.03f,.endSz=0.02f,.endRand=0.01f,.startSpd=2.f,.spdRand=1.f,.gravMod=0.6f,.emitRate=0,.startCol=0xFF44AA22u,.endCol=0x00226600u,.shapeAngle=30.f,.shapeRad=0.02f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_BURST,.texStart=756,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=0,.space=PSYS_SPACE_WORLD,.rotMin=-90.f,.rotMax=90.f},
    [13]={.name="Mutation",  .maxParts=20, .burstCnt=10, .life=0.5f,.lifeRand=0.2f,.startSz=0.04f,.szRand=0.02f,.endSz=0.01f,.endRand=0.005f,.startSpd=3.f,.spdRand=2.f,.gravMod=0.4f,.emitRate=0,.startCol=0xFFAA44AAu,.endCol=0x00660066u,.shapeAngle=25.f,.shapeRad=0.02f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_BURST,.texStart=757,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=0,.space=PSYS_SPACE_WORLD,.rotMin=-180.f,.rotMax=180.f},
    [14]={.name="Graytation",.maxParts=20,.burstCnt=10, .life=0.5f,.lifeRand=0.2f,.startSz=0.04f,.szRand=0.02f,.endSz=0.01f,.endRand=0.005f,.startSpd=3.f,.spdRand=2.f,.gravMod=0.4f,.emitRate=0,.startCol=0xFF888888u,.endCol=0x00444444u,.shapeAngle=25.f,.shapeRad=0.02f,.shape=PSYS_SHAPE_CONE,.emitMode=PSYS_EMIT_BURST,.texStart=758,.texCnt=1,.animFPS=0,.billboard=PSYS_BILLBOARD_SCREEN,.softPart=0,.space=PSYS_SPACE_WORLD,.rotMin=-180.f,.rotMax=180.f},
};

static DepthSort* psysSortBuf = NULL;
#define PSys_PlayOneshot(defIdx,pos) PSys_Play(defIdx,pos,QUAT_IDENTITY,0)
PsParticle psysParts[MAX_PSYS_PARTICLES];
PsEmitter psysEmitters[MAX_PSYS_EMITTERS];
u32 psysPartFree=0,psysEmitterFree=0,psysAliveCount=0;
u32 psysVao=0,psysVbo=0,psysSp=0;
static double psysLastTime;
static float* psysVertBuf=NULL; // Flat-VBO vertex: 8 floats per corner (x,y,z, u,v, r,g,b,a packed?, texIdx float), Actually: xyz(3)+uv(2)+rgba(4)+texIdx(1) = 10 floats per vertex, 60 per quad
#define PSYS_VTX_STRIDE 10
#define PSYS_QUAD_FLOATS (6*PSYS_VTX_STRIDE) // 60 floats per particle quad
static inline u32 psysAllocPart(void) { u32 i=psysPartFree; if(i>=MAX_PSYS_PARTICLES)return MAX_PSYS_PARTICLES; psysPartFree=*(u32*)&psysParts[i].pos.x; return i; }
static inline void psysFreePart(u32 i) { *(u32*)&psysParts[i].pos.x=psysPartFree; psysPartFree=i; }
static inline u32 psysAllocEmitter(void) { u32 i=psysEmitterFree; if(i>=MAX_PSYS_EMITTERS)return MAX_PSYS_EMITTERS; psysEmitterFree=*(u32*)&psysEmitters[i].defIdx; return i; }
static inline void psysFreeEmitter(u32 i) { *(u32*)&psysEmitters[i].defIdx=psysEmitterFree; psysEmitterFree=i; }

INLINE float psysRand(float mn, float mx) { return mn + (mx - mn) * (float)random_range(0u, 100000u) * 0.00001f; }

static inline u32 lerpColor(u32 a, u32 b, float t) {
    float ti = 1.f - t;
    u32 ar = (a) & 0xFF, ag = (a >> 8) & 0xFF, ab = (a >> 16) & 0xFF, aa = (a >> 24) & 0xFF;
    u32 br = (b) & 0xFF, bg = (b >> 8) & 0xFF, bb = (b >> 16) & 0xFF, ba = (b >> 24) & 0xFF;
    return ((u32)((float)ar * ti + (float)br * t) & 0xFF) |
           (((u32)((float)ag * ti + (float)bg * t) & 0xFF) << 8) |
           (((u32)((float)ab * ti + (float)bb * t) & 0xFF) << 16) |
           (((u32)((float)aa * ti + (float)ba * t) & 0xFF) << 24);
}

u16 PSys_Play(u8 defIdx, V3 pos, Quaternion rot, u16 parentEnt) {
    if (defIdx >= MAX_PSYS_DEFS || !psysDefs[defIdx].name) return U16_MAX;
    u32 ei = psysAllocEmitter(); if (ei >= MAX_PSYS_EMITTERS) return U16_MAX;
    const PsDef* d = &psysDefs[defIdx];
    PsEmitter* e = &psysEmitters[ei]; mset(e, 0, sizeof(PsEmitter));
    e->defIdx = defIdx; e->pos = e->lastPos = pos; e->rot = rot; e->parentEnt = parentEnt;
    e->startTime = World.pauseRelativeTime; e->flags = PSYS_ALIVE;
    e->interval = d->emitRate > 0.f ? 1.f / d->emitRate : 0.f;
    e->nextEmit = World.pauseRelativeTime + (d->prewarm > 0.f ? -d->prewarm : 0.f);
    e->subCnt = d->subCnt;
    for (u8 i = 0; i < d->subCnt; ++i) { e->subDefs[i] = d->subDefs[i]; e->subEvents[i] = d->subEvents[i]; }
    if (d->emitMode == PSYS_EMIT_BURST) { e->flags |= PSYS_BURST; }
    if (d->emitMode == PSYS_EMIT_CONTINUOUS && d->emitRate > 0.f) { e->flags |= PSYS_LOOPING; }
    if (vabs(pos.x - 12.131038666f) < 0.5f && vabs(pos.y - (-43.779998779f)) < 0.5f && vabs(pos.z - 19.215139389f) < 0.5f) {
        e->flags |= PSYS_LOOPING;
        if (e->interval <= 0.f) e->interval = 1.0f / 15.0f;
    }
    DualLog("PSys_Play %u at locaton %f %f %f\n", defIdx, pos.x, pos.y, pos.z);
    return (u16)ei;
}

static void psysSpawnSub(u8 defIdx, V3 pos, Quaternion rot, u16 parentEnt) {
    if (defIdx < MAX_PSYS_DEFS && psysDefs[defIdx].name) PSys_Play(defIdx, pos, rot, parentEnt);
}

static V3 psysShapeDir(const PsDef* d, V3 fwd, V3 up, V3 right) {
    float a = psysRand(-d->shapeAngle * 0.5f, d->shapeAngle * 0.5f), b = psysRand(-d->shapeArc * 0.5f, d->shapeArc * 0.5f);
    float ca = vcosf(deg2rad(a)), sa = vsinf(deg2rad(a)), cb = vcosf(deg2rad(b)), sb = vsinf(deg2rad(b));
    return V3_Normalize(V3_AplusB(V3_AplusB(V3_ScaleByF(fwd, ca), V3_ScaleByF(right, sa * cb)), V3_ScaleByF(up, sa * sb)));
}

static V3 psysShapePos(const PsDef* d, V3 center, V3 fwd, V3 up, V3 right) {
    (void)fwd;
    switch (d->shape) {
        case PSYS_SHAPE_SPHERE: { float th = psysRand(0.f, TAU), ph = psysRand(0.f, PI), sr = vsinf(ph) * d->shapeRad; return V3_AplusB(center, (V3){vcosf(th) * sr, vcosf(ph) * d->shapeRad, vsinf(th) * sr}); }
        case PSYS_SHAPE_HEMI:   { float th = psysRand(0.f, TAU), ph = psysRand(0.f, PI * 0.5f), sr = vsinf(ph) * d->shapeRad; return V3_AplusB(center, (V3){vcosf(th) * sr, vcosf(ph) * d->shapeRad, vsinf(th) * sr}); }
        case PSYS_SHAPE_BOX:    return V3_AplusB(center, (V3){psysRand(-d->shapeRad, d->shapeRad), psysRand(-d->shapeRad, d->shapeRad), psysRand(-d->shapeRad, d->shapeRad)});
        case PSYS_SHAPE_CIRCLE: { float th = psysRand(0.f, TAU), r = psysRand(0.f, d->shapeRad); return V3_AplusB(center, V3_AplusB(V3_ScaleByF(right, vcosf(th) * r), V3_ScaleByF(up, vsinf(th) * r))); }
        case PSYS_SHAPE_EDGE:   return center;
        default:                { float r = psysRand(0.f, d->shapeRad), th = psysRand(0.f, TAU); return V3_AplusB(center, V3_AplusB(V3_ScaleByF(right, vcosf(th) * r), V3_ScaleByF(up, vsinf(th) * r))); }
    }
}

void PSys_Init(void) {
    for (u32 i = 0; i < MAX_PSYS_PARTICLES - 1; ++i) *(u32*)&psysParts[i].pos.x = i + 1;
    *(u32*)&psysParts[MAX_PSYS_PARTICLES - 1].pos.x = MAX_PSYS_PARTICLES;
    for (u32 i = 0; i < MAX_PSYS_EMITTERS - 1; ++i) *(u32*)&psysEmitters[i].defIdx = i + 1;
    *(u32*)&psysEmitters[MAX_PSYS_EMITTERS - 1].defIdx = MAX_PSYS_EMITTERS;
    psysPartFree = 0; psysEmitterFree = 0; psysAliveCount = 0;
    psysVertBuf = (float*)OS_Alloc(MAX_PSYS_PARTICLES * PSYS_QUAD_FLOATS * sizeof(float));
    psysSortBuf = (DepthSort*)OS_Alloc(MAX_PSYS_PARTICLES * sizeof(DepthSort));
    glGenVertexArrays(1, &psysVao); glGenBuffers(1, &psysVbo);
    glBindVertexArray(psysVao); glBindBuffer(GL_ARRAY_BUFFER, psysVbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_PSYS_PARTICLES * PSYS_QUAD_FLOATS * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);                 glVertexAttribBinding(0, 0); glEnableVertexAttribArray(0);
    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); glVertexAttribBinding(1, 0); glEnableVertexAttribArray(1);
    glVertexAttribFormat(2, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(float)); glVertexAttribBinding(2, 0); glEnableVertexAttribArray(2);
    glVertexAttribFormat(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float)); glVertexAttribBinding(3, 0); glEnableVertexAttribArray(3);
    glBindVertexBuffer(0, psysVbo, 0, PSYS_VTX_STRIDE * sizeof(float));
    glBindVertexArray(0);
    psysLastTime = World.pauseRelativeTime - 0.016666667;
}

u16 PSys_PlayLooping(u8 defIdx, V3 pos, Quaternion rot, u16 parentEnt, float emitRate) {
    u16 ei = PSys_Play(defIdx, pos, rot, parentEnt);
    if (ei != U16_MAX) {
        PsEmitter* e = &psysEmitters[ei];
        e->flags |= PSYS_LOOPING;
        e->flags &= ~PSYS_BURST;
        float rate = (emitRate > 0.f) ? emitRate : (psysDefs[defIdx].emitRate > 0.f ? psysDefs[defIdx].emitRate : 15.f);
        e->interval = 1.f / rate;
    }
    return ei;
}

u16 PSys_PlayAtEntity(u8 defIdx, u16 entityIdx) { return PSys_Play(defIdx, World.position[entityIdx], World.rotation[entityIdx], entityIdx); }
void PSys_Stop(u16 ei) { if (ei < MAX_PSYS_EMITTERS && psysEmitters[ei].flags & PSYS_ALIVE) psysEmitters[ei].flags |= PSYS_STOPPING; }
bool PSys_IsPlaying(u16 ei) { return ei < MAX_PSYS_EMITTERS && (psysEmitters[ei].flags & PSYS_ALIVE); }
void PSys_StopAll(void) { for (u32 i = 0; i < MAX_PSYS_EMITTERS; ++i) if (psysEmitters[i].flags & PSYS_ALIVE) psysEmitters[i].flags |= PSYS_STOPPING; }
u16 PSys_PlayBurstAt(u8 defIdx, V3 pos, V3 dir, float speed) {
    u16 e = PSys_Play(defIdx, pos, QUAT_IDENTITY, 0);
    if (e != U16_MAX) { PsEmitter* em = &psysEmitters[e]; em->lastPos = V3_AsubB(pos, V3_ScaleByF(dir, speed * 0.016f)); }
    return e;
}

static void psysEmitParticle(PsEmitter* e, const PsDef* d, V3 fwd, V3 up, V3 right) {
    u32 pi = psysAllocPart(); if (pi >= MAX_PSYS_PARTICLES) return;
    PsParticle* p = &psysParts[pi]; mset(p, 0, sizeof(PsParticle));
    V3 sp = psysShapePos(d, e->pos, fwd, up, right), sd = psysShapeDir(d, fwd, up, right);
    float s = psysRand(d->startSpd, d->startSpd + d->spdRand);
    p->pos = sp; p->vel = V3_ScaleByF(sd, s);
    p->age = 0.f; p->maxAge = psysRand(d->life, d->life + d->lifeRand);
    p->startSize = psysRand(d->startSz, d->startSz + d->szRand); p->endSize = psysRand(d->endSz, d->endSz + d->endRand);
    p->size = p->startSize; p->startCol = d->startCol; p->endCol = d->endCol;
    p->rot = psysRand(0.f, TAU); p->rotSpeed = deg2rad(psysRand(d->rotMin, d->rotMax));
    p->texAnim = d->texStart; p->emitIdx = (u8)(e - psysEmitters); p->flags = PSYS_ALIVE;
    if (d->inheritVel) {
        V3 ev = V3_AsubB(e->pos, e->lastPos); float dt = (float)(World.pauseRelativeTime - psysLastTime);
        if (dt > 0.0001f) p->vel = V3_AplusB(p->vel, V3_ScaleByF(ev, 1.f / vmax(dt, 0.0001f)));
    }
    e->emitted++; e->partCount++; psysAliveCount++;
}

void PSys_Update(void) {
    double now = World.pauseRelativeTime; if (now <= psysLastTime) { psysLastTime = now; return; }
    float dt = (float)(now - psysLastTime); psysLastTime = now; if (dt <= 0.f || dt > 0.1f) return;
    psysAliveCount = 0;
    for (u32 i = 0; i < MAX_PSYS_PARTICLES; ++i) {
        PsParticle* p = &psysParts[i]; if (!(p->flags & PSYS_ALIVE)) continue;
        p->age += dt;
        if (p->age >= p->maxAge) {
            PsEmitter* e = &psysEmitters[p->emitIdx]; const PsDef* d = &psysDefs[e->defIdx];
            for (u8 s = 0; s < d->subCnt; ++s) if (d->subEvents[s] == PSYS_SUB_DEATH) psysSpawnSub(d->subDefs[s], p->pos, e->rot, e->parentEnt);
            if (e->partCount > 0) e->partCount--; p->flags = 0; psysFreePart(i); continue;
        }
        const PsDef* d = &psysDefs[psysEmitters[p->emitIdx].defIdx];
        p->vel.y -= d->gravMod * dt; p->pos = V3_AplusB(p->pos, V3_ScaleByF(p->vel, dt));
        float t = vclamp(p->age / vmax(p->maxAge, 0.0001f), 0.f, 1.f);
        p->size = p->startSize + (p->endSize - p->startSize) * t; p->rot += p->rotSpeed * dt;
        if (d->texCnt > 1 && d->animFPS > 0) p->texAnim = d->texStart + ((u8)(p->age * (float)d->animFPS) % d->texCnt);
        psysAliveCount++;
    }
    for (u32 ei = 0; ei < MAX_PSYS_EMITTERS; ++ei) {
        PsEmitter* e = &psysEmitters[ei]; if (!(e->flags & PSYS_ALIVE)) continue;
        const PsDef* d = &psysDefs[e->defIdx]; e->lastPos = e->pos;
        if (e->parentEnt != U16_MAX && e->parentEnt < INSTANCE_COUNT) { e->pos = World.position[e->parentEnt]; e->rot = World.rotation[e->parentEnt]; }
        bool stopping = (e->flags & PSYS_STOPPING) != 0;
        if ((e->flags & PSYS_BURST) && !stopping) {
            while (e->emitted < d->burstCnt && e->partCount < d->maxParts) {
                V3 fwd = quat_rot_v3(e->rot, (V3){0,0,1}), up = quat_rot_v3(e->rot, (V3){0,1,0}), right = quat_rot_v3(e->rot, (V3){1,0,0});
                psysEmitParticle(e, d, fwd, up, right);
            }
        }
        if ((e->flags & PSYS_LOOPING) && !stopping && e->interval > 0.f) {
            while (now >= e->nextEmit && e->partCount < d->maxParts) {
                V3 fwd = quat_rot_v3(e->rot, (V3){0,0,1}), up = quat_rot_v3(e->rot, (V3){0,1,0}), right = quat_rot_v3(e->rot, (V3){1,0,0});
                psysEmitParticle(e, d, fwd, up, right); e->nextEmit += e->interval;
            }
        }
        if (stopping && e->partCount == 0) { e->flags = 0; psysFreeEmitter(ei); }
        else if (!(e->flags & PSYS_LOOPING) && !(e->flags & PSYS_BURST) && e->partCount == 0) { e->flags = 0; psysFreeEmitter(ei); }
        else if ((e->flags & PSYS_BURST) && !(e->flags & PSYS_LOOPING) && e->emitted >= d->burstCnt && e->partCount == 0) { e->flags = 0; psysFreeEmitter(ei); }
    }
}

static const float psysCorners[4][2] = {{-1.f,-1.f},{1.f,-1.f},{1.f,1.f},{-1.f,1.f}};
static const float psysUVs[4][2] = {{0.f,0.f},{1.f,0.f},{1.f,1.f},{0.f,1.f}};
static const u16 psysIndices[6] = {0,1,2, 0,2,3};

static i32 psysSortCmp(const void* a, const void* b) {
    float da = ((const DepthSort*)a)->depth, db = ((const DepthSort*)b)->depth;
    return (db > da) - (db < da);
}

void PSys_Render(float* viewProj, V3 camPos, V3 camUp, V3 camRight, float nearPlane, float farPlane, u32 depthTex) {
    if (World.pauseRelativeTime > psysLastTime) PSys_Update();
    if (!psysAliveCount) return;

    u32 wc = 0; bool anySoft = false;
    for (u32 i = 0; i < MAX_PSYS_PARTICLES && wc < psysAliveCount; ++i) {
        PsParticle* p = &psysParts[i]; if (!(p->flags & PSYS_ALIVE)) continue;
        const PsDef* d = &psysDefs[psysEmitters[p->emitIdx].defIdx];
        if (d->softPart) anySoft = true;
        V3 s = V3_AsubB(p->pos,camPos); psysSortBuf[wc].depth = V3_dot(s,s); psysSortBuf[wc].index = (u16)i; wc++;
    }
    if (!wc) return;
    if (wc > 1) {
        if (wc <= 32) {
            for (u32 i = 1; i < wc; ++i) {
                DepthSort key = psysSortBuf[i]; i32 j = (i32)i - 1;
                while (j >= 0 && psysSortBuf[j].depth < key.depth) { psysSortBuf[j + 1] = psysSortBuf[j]; j--; }
                psysSortBuf[j + 1] = key;
            }
        } else {
            qsort_new(psysSortBuf, wc, sizeof(DepthSort), psysSortCmp);
        }
    }

    for (u32 k = 0; k < wc; ++k) {
        PsParticle* p = &psysParts[psysSortBuf[k].index];
        float t = vclamp(p->age / vmax(p->maxAge, 0.0001f), 0.f, 1.f);
        u32 col = lerpColor(p->startCol, p->endCol, t);
        float cr = (float)(col & 0xFF) / 255.f, cg = (float)((col >> 8) & 0xFF) / 255.f, cb = (float)((col >> 16) & 0xFF) / 255.f, ca = (float)((col >> 24) & 0xFF) / 255.f;
        const PsDef* d = &psysDefs[psysEmitters[p->emitIdx].defIdx];
        float tex = (float)(d->texStart + ((d->texCnt > 1 && d->animFPS > 0) ? ((u32)(p->age * (float)d->animFPS) % (u32)d->texCnt) : 0u));
        float sz = p->size;

        V3 quadRight, quadUp;
        switch (d->billboard) {
            case PSYS_BILLBOARD_STRETCHED: {
                V3 vdir = p->vel; float vsq = V3_dot(vdir,vdir);
                if (vsq < 1e-6f) vdir = camUp; else vdir = V3_ScaleByF(vdir, 1.0f / vsqrtf(vsq));
                V3 toCam = V3_AsubB(camPos, p->pos), rx = V3_Cross(vdir, toCam);
                float rxsq = V3_dot(rx,rx);
                if (rxsq < 1e-6f) quadRight = camRight; else quadRight = V3_ScaleByF(rx, 1.0f / vsqrtf(rxsq));
                quadUp = vdir; break;
            }
            case PSYS_BILLBOARD_HORIZ: { quadRight = (V3){1.f, 0.f, 0.f}; quadUp = (V3){0.f, 0.f, 1.f}; break; }
            case PSYS_BILLBOARD_VERT: {
                quadUp = (V3){0.f, 1.f, 0.f};
                V3 toCam = V3_AsubB(camPos, p->pos), rx = V3_Cross(quadUp, toCam);
                float rxsq = V3_dot(rx,rx);
                if (rxsq < 1e-6f) quadRight = camRight; else quadRight = V3_ScaleByF(rx, 1.0f / vsqrtf(rxsq));
                break;
            }
            default: { quadRight = camRight; quadUp = camUp; break; }
        }

        float c = p->rot, vcos = vcosf(c), vsin = vsinf(c);
        float* v = &psysVertBuf[k * PSYS_QUAD_FLOATS];
        for (u32 j = 0; j < 6; ++j) {
            u16 idx = psysIndices[j]; float cx = psysCorners[idx][0], cy = psysCorners[idx][1];
            float rx = (cx * vcos - cy * vsin) * sz, ry = (cx * vsin + cy * vcos) * sz;
            float* vv = &v[j * PSYS_VTX_STRIDE];
            vv[0] = p->pos.x + quadRight.x * rx + quadUp.x * ry;
            vv[1] = p->pos.y + quadRight.y * rx + quadUp.y * ry;
            vv[2] = p->pos.z + quadRight.z * rx + quadUp.z * ry;
            vv[3] = psysUVs[idx][0]; vv[4] = psysUVs[idx][1];
            vv[5] = cr; vv[6] = cg; vv[7] = cb; vv[8] = ca; vv[9] = tex;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, psysVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, wc * PSYS_QUAD_FLOATS * sizeof(float), psysVertBuf);
    glUseProgram(psysSp);
    glUniformMatrix4fv(0, 1, GL_FALSE, viewProj);
    if (anySoft && depthTex) { glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, depthTex); glUniform1i(5, 5); }
    glUniform1f(7, anySoft ? 0.5f : 0.f);
    glUniform2f(8, nearPlane, farPlane);
    glUniform2f(6, (float)Sys_Settings.ScreenWidth, (float)Sys_Settings.ScreenHeight);

    glBindVertexArray(psysVao);
    glBindVertexBuffer(0, psysVbo, 0, PSYS_VTX_STRIDE * sizeof(float));
    glEnable(GL_DEPTH_TEST); glDepthMask(GL_FALSE); glDepthFunc(0x0203/*GL_LEQUAL*/);
    glDisable(GL_CULL_FACE); glEnable(GL_BLEND);
    glDrawArrays(0x0004/*GL_TRIANGLES*/, 0, wc * 6);
    drawCalls++; vertsRendered += wc * 6;
    glEnable(GL_CULL_FACE); glDepthMask(1/*GL_TRUE*/);
}
