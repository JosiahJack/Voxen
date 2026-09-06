// particles.c - CPU-simulated, GPU-instanced particle system for Voxen
#include "common.h"
typedef struct Particle { V3 pos,vel; float age,invLifetime,baseSize,rotation,angularVelocity; u32 color,emitterIndex; u16 flags,textureIndex,animFrame; u8 blendMode; u32 seed; V3 trailSample; float trailBirth; } Particle;
typedef struct Emitter { bool active; V3 position; float emitAccumulator,emitRate,age,duration; u16 aliveCount,maxAlive; u8 blendMode,physicsMode,trail; u16 trailTexture; float lifetimeMin,lifetimeMax,sizeMin,sizeMax,speedMin,speedMax,rotationMin,rotationMax,angularVelocityMin,angularVelocityMax,gravity,trailLifetime,trailWidthStart,trailWidthEnd; u32 trailColorStart,trailColorEnd;
                         u32 textureBaseIndex,textureFrameCount; float animSpeed,animWindow,softness,scaleCurve[32],velocityCurve[32],rotationCurve[32],emissionCurve[32]; u32 colorRamp[64]; u64 rngState; } Emitter;
typedef struct GpuParticleInstance { float x,y,z,size; u32 color,data0,data1,pad; } GpuParticleInstance;
typedef struct TrailSegmentInstance { float p0x,p0y,p0z,padA,p1x,p1y,p1z,padB; float c00x,c00y,c00z,c00w,c01x,c01y,c01z,c01w,c10x,c10y,c10z,c10w,c11x,c11y,c11z,c11w; u32 color0,color1,uvData; float deathTime; float birth0,birth1,pad0,pad1; } TrailSegmentInstance;
typedef struct { u32 sortKey; u16 index; } SortEntry;
typedef struct { Particle particles[MAX_PARTICLES]; Emitter emitters[MAX_EMITTERS]; GpuParticleInstance gpuInstances[MAX_PARTICLES]; TrailSegmentInstance trailSegments[MAX_TRAIL_SEGMENTS]; SortEntry sortKeys[MAX_PARTICLES]; u32 aliveCount,trailCount,instanceBuffer,trailBuffer,quadVAO,quadVBO,particleSP,trailSP; bool initialized; } ParticleSystem;
ParticleSystem psys = {0};
void ParticleSystem_SpawnTrail(V3 p0, V3 p1, u32 texIndex, u32 emitterIndex, float lifetime, float birth0);
INLINE u32 xs32(u64* state) { u64 x = *state; x ^= x << 13; x ^= x >> 7; x ^= x << 17; *state = x; return (u32)(x ^ (x >> 32)); }
INLINE float randf(u64* state) { return (float)(xs32(state) & 0xFFFFFF) * (1.0f / 16777216.0f); }
INLINE float randf_range(u64* state, float a, float b) { return a + (b - a) * randf(state); }
INLINE u32 pack_rgba8(float r, float g, float b, float a) { u32 ri = (u32)(vclamp(r, 0.0f, 1.0f) * 255.0f + 0.5f); u32 gi = (u32)(vclamp(g, 0.0f, 1.0f) * 255.0f + 0.5f); u32 bi = (u32)(vclamp(b, 0.0f, 1.0f) * 255.0f + 0.5f); u32 ai = (u32)(vclamp(a, 0.0f, 1.0f) * 255.0f + 0.5f); return (ai << 24) | (bi << 16) | (gi << 8) | ri; }
INLINE void unpack_rgba8(u32 packed, float* r, float* g, float* b, float* a) { *r = ((packed >> 0) & 0xFF) * (1.0f / 255.0f); *g = ((packed >> 8) & 0xFF) * (1.0f / 255.0f); *b = ((packed >> 16) & 0xFF) * (1.0f / 255.0f); *a = ((packed >> 24) & 0xFF) * (1.0f / 255.0f); }
INLINE u32 ColorToU32(Color c) { return pack_rgba8(c.r, c.g, c.b, c.a); }
INLINE void build_color_ramp(Emitter* em, const Color* colors, const float* times, int numKeys) {
    for (int i = 0; i < 64; i++) {
        float t = (float)i / 63.0f; int keyIdx = 0; while (keyIdx < numKeys - 1 && times[keyIdx + 1] <= t) keyIdx++;
        if (keyIdx >= numKeys - 1) { em->colorRamp[i] = ColorToU32(colors[numKeys - 1]); }
        else { float localT = (t - times[keyIdx]) / (times[keyIdx + 1] - times[keyIdx]); Color c0 = colors[keyIdx], c1 = colors[keyIdx + 1]; em->colorRamp[i] = pack_rgba8(c0.r + (c1.r - c0.r) * localT, c0.g + (c1.g - c0.g) * localT, c0.b + (c1.b - c0.b) * localT, c0.a + (c1.a - c0.a) * localT); }
    }
}

INLINE void build_curve(float* curve, const float* keys, const float* times, int numKeys) {
    for (int i = 0; i < 32; i++) {
        float t = (float)i / 31.0f; int keyIdx = 0; while (keyIdx < numKeys - 1 && times[keyIdx + 1] <= t) keyIdx++;
        if (keyIdx >= numKeys - 1) { curve[i] = keys[numKeys - 1]; }
        else { float localT = (t - times[keyIdx]) / (times[keyIdx + 1] - times[keyIdx]); curve[i] = keys[keyIdx] + (keys[keyIdx + 1] - keys[keyIdx]) * localT; }
    }
}

INLINE u32 sample_color_ramp(Emitter* em, float t) { int idx = (int)(t * 63.0f); if (idx < 0) idx = 0; if (idx >= 64) idx = 63; return em->colorRamp[idx]; }
INLINE float sample_curve(const float* curve, float t) { int idx = (int)(t * 31.0f); if (idx < 0) idx = 0; if (idx >= 31) return curve[31]; float localT = t * 31.0f - idx; return curve[idx] + (curve[idx + 1] - curve[idx]) * localT; }
void ParticleSystem_Init(void) {
    if (psys.initialized) return;
    glGenVertexArrays(1, &psys.quadVAO); glGenBuffers(1, &psys.quadVBO);
    static const float quadVerts[16] = {-1.0f, -1.0f, 0.0f, 0.0f,1.0f, -1.0f, 1.0f, 0.0f,-1.0f,  1.0f, 0.0f, 1.0f,1.0f,  1.0f, 1.0f, 1.0f,};
    glBindVertexArray(psys.quadVAO); glBindBuffer(GL_ARRAY_BUFFER, psys.quadVBO); glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW); 
    glVertexAttribFormat(0,2,GL_FLOAT,GL_FALSE,0); glVertexAttribBinding(0, 0); glEnableVertexAttribArray(0);
    glVertexAttribFormat(1,2,GL_FLOAT,GL_FALSE,2 * sizeof(float)); glVertexAttribBinding(1, 0); glEnableVertexAttribArray(1); glBindVertexBuffer(0, psys.quadVBO, 0, 4 * sizeof(float));
    glGenBuffers(1, &psys.instanceBuffer); glBindBuffer(GL_SHADER_STORAGE_BUFFER, psys.instanceBuffer); glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof(GpuParticleInstance), NULL, GL_DYNAMIC_DRAW); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PARTICLE_SSBO_BINDING, psys.instanceBuffer);
    glGenBuffers(1, &psys.trailBuffer); glBindBuffer(GL_SHADER_STORAGE_BUFFER, psys.trailBuffer); glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_TRAIL_SEGMENTS * sizeof(TrailSegmentInstance), NULL, GL_DYNAMIC_DRAW); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TRAIL_SSBO_BINDING, psys.trailBuffer);
    psys.initialized = true; DualLog("Particle system initialized: %d max particles, %d max trail segments\n", MAX_PARTICLES, MAX_TRAIL_SEGMENTS);
}

u16 ParticleSystem_AddEmitter(V3 position, u32 textureIndex, float emitRate, float lifetime, float sizeMin, float sizeMax, float speedMin, float speedMax, Color colorStart, Color colorEnd, u8 blendMode) {
    for (u16 i = 0; i < MAX_EMITTERS; i++) {
        if (!psys.emitters[i].active && psys.emitters[i].aliveCount == 0) {
            Emitter* em = &psys.emitters[i]; em->active = true; em->position = position;
            em->emitAccumulator = 0.0f; em->emitRate = emitRate; em->age = 0.0f; em->duration = lifetime; em->aliveCount = 0; em->maxAlive = 2000; em->blendMode = blendMode; em->physicsMode = 0; em->trail = 0; em->trailTexture = 0; em->lifetimeMin = 0.5f; em->lifetimeMax = 2.0f;
            em->sizeMin = sizeMin; em->sizeMax = sizeMax; em->speedMin = speedMin; em->speedMax = speedMax; em->rotationMin = 0.0f; em->rotationMax = 6.2831853f; em->angularVelocityMin = -1.0f; em->angularVelocityMax = 1.0f; em->gravity = 1.0f; em->trailLifetime = 1.0f; em->trailColorStart = 0xFFFFFFFFu; em->trailColorEnd = 0xFFFFFFFFu; em->trailWidthStart = 0.05f; em->trailWidthEnd = 0.05f; em->textureBaseIndex = textureIndex;
            em->textureFrameCount = 1; em->animSpeed = 10.0f; em->animWindow = 1.0f; em->softness = 1.0f; for (int c = 0; c < 32; c++) { em->scaleCurve[c] = 1.0f; em->velocityCurve[c] = 1.0f; em->rotationCurve[c] = 0.0f; em->emissionCurve[c] = 1.0f; }
            Color colors[2] = {colorStart, colorEnd}; float times[2] = {0.0f, 1.0f}; build_color_ramp(em, colors, times, 2); em->rngState = globalframe * 1234567 + i * 98765 + 1; return i;
        }
    } return U16_MAX;
}

void ParticleSystem_SetEmitterPhysics(u16 index, float bounce, float gravity) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; (void)bounce; em->physicsMode = 1; em->gravity = gravity; }
void ParticleSystem_SetEmitterTrail(u16 index, u8 enabled, u32 textureIndex) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; em->trail = enabled ? 1 : 0; em->trailTexture = (u16)(textureIndex & 0xFFFF); }
void ParticleSystem_SetEmitterTrailLifetime(u16 index, float lifetime) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; if (lifetime > 0.0f) em->trailLifetime = lifetime; }
void ParticleSystem_SetEmitterTrailColor(u16 index, Color start, Color end) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; em->trailColorStart = ColorToU32(start); em->trailColorEnd = ColorToU32(end); }
void ParticleSystem_SetEmitterTrailWidth(u16 index, float start, float end) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; em->trailWidthStart = start; em->trailWidthEnd = end; }
void ParticleSystem_SetEmitterColorRamp(u16 index, const Color* colors, const float* times, int numKeys) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; build_color_ramp(em, colors, times, numKeys); }
void ParticleSystem_SetEmitterScaleCurve(u16 index, const float* keys, const float* times, int numKeys) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; build_curve(em->scaleCurve, keys, times, numKeys); }
void ParticleSystem_SetEmitterVelocityCurve(u16 index, const float* keys, const float* times, int numKeys) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; build_curve(em->velocityCurve, keys, times, numKeys); }
void ParticleSystem_SetEmitterAnimation(u16 index, u16 frameCount) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; if (frameCount > 0) em->textureFrameCount = frameCount; }
void ParticleSystem_SetEmitterAnimationWindow(u16 index, float window) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; if (window > 0.0f) em->animWindow = window; }
void ParticleSystem_SetEmitterRotation(u16 index, float angularVelocity) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; em->angularVelocityMin = angularVelocity; em->angularVelocityMax = angularVelocity; }
void ParticleSystem_SetEmitterLifetime(u16 index, float lifetimeMin, float lifetimeMax) { if (index >= MAX_EMITTERS) return; Emitter* em = &psys.emitters[index]; if (!em->active) return; em->lifetimeMin = lifetimeMin; em->lifetimeMax = lifetimeMax; }
void ParticleSystem_UpdateEmitters(float dt) {
    for (u16 i = 0; i < MAX_EMITTERS; i++) {
        Emitter* em = &psys.emitters[i]; if(!em->active){continue;} em->age+=dt; if(em->duration > 0.0f&& em->age>=em->duration){em->active=false; continue;}
        float rate = em->emitRate; if (em->duration > 0.0f && em->duration < 1e6f){rate*=sample_curve(em->emissionCurve,em->age/em->duration);} em->emitAccumulator+=rate*dt; int count=(int)em->emitAccumulator; em->emitAccumulator-=(float)count; 
        for (int p = 0; p < count; p++) {
            if ((psys.aliveCount >= MAX_PARTICLES) || (em->aliveCount >= em->maxAlive)){break;} Particle* part = &psys.particles[psys.aliveCount]; float angle = randf_range(&em->rngState,0.0f,6.2831853f), speed = randf_range(&em->rngState,em->speedMin,em->speedMax);
            part->pos = em->position; part->vel.x=vcosf(angle)*speed; part->vel.y=randf_range(&em->rngState,-0.5f,0.5f)*speed; part->vel.z=vsinf(angle)*speed; part->age=0.0f; part->invLifetime=1.0f/randf_range(&em->rngState,em->lifetimeMin,em->lifetimeMax); part->baseSize=randf_range(&em->rngState,em->sizeMin,em->sizeMax); part->rotation=randf_range(&em->rngState,em->rotationMin,em->rotationMax);
            part->angularVelocity = randf_range(&em->rngState, em->angularVelocityMin, em->angularVelocityMax); part->color = sample_color_ramp(em,0.0f); part->emitterIndex = i; part->flags=0; part->blendMode=particleBlendTexture[em->textureBaseIndex];
            if (part->blendMode == 1) { part->flags |= PARTICLE_FLAG_ADDITIVE; } else if (part->blendMode == 2) { part->flags |= PARTICLE_FLAG_MULTIPLY; }
            if (em->softness > 0.0f) { part->flags |= PARTICLE_FLAG_SOFT; }
            part->textureIndex = em->textureBaseIndex; part->animFrame = 0; part->seed = xs32(&em->rngState); part->trailSample = em->position; part->trailBirth = (float)World.pauseRelativeTime; psys.aliveCount++; em->aliveCount++;
        }
    }
}

void ParticleSystem_Simulate(float dt) {
    u32 i = 0; V3 camPos = World.position[PLAYER1], camForward = World.instances[PLAYER1].forward;
    while (i < psys.aliveCount) {
        Particle* p = &psys.particles[i]; Emitter* em = &psys.emitters[p->emitterIndex]; p->age += dt;
        float t = p->age * p->invLifetime;
        if (t >= 1.0f) { psys.aliveCount--; em->aliveCount--; psys.particles[i] = psys.particles[psys.aliveCount]; continue; }
        float scale, velScale, rotAdd;
        if (em->active) {
            scale = sample_curve(em->scaleCurve, t); velScale = sample_curve(em->velocityCurve, t); rotAdd = sample_curve(em->rotationCurve, t) * dt;
            p->color = sample_color_ramp(em, t);
            if (em->textureFrameCount > 1) { float it = t / (em->animWindow > 0.0f ? em->animWindow : 1.0f); p->animFrame = (u16)(it * em->textureFrameCount) % em->textureFrameCount; }
        } else { scale = 1.0f; velScale = 1.0f; rotAdd = 0.0f; }
        p->pos.x += p->vel.x * velScale * dt; p->pos.y += p->vel.y * velScale * dt; p->pos.z += p->vel.z * velScale * dt; p->rotation += p->angularVelocity * dt + rotAdd;
        if (em->physicsMode) { p->vel.y -= em->gravity * dt; }
        if (em->trail) { float dx = p->pos.x - p->trailSample.x, dy = p->pos.y - p->trailSample.y, dz = p->pos.z - p->trailSample.z; if (dx * dx + dy * dy + dz * dz >= 0.0004f) { ParticleSystem_SpawnTrail(p->trailSample, p->pos, em->trailTexture, p->emitterIndex, em->trailLifetime, p->trailBirth); p->trailSample = p->pos; p->trailBirth = (float)World.pauseRelativeTime; } }
        GpuParticleInstance* gpu = &psys.gpuInstances[i];
        gpu->x = p->pos.x; gpu->y = p->pos.y; gpu->z = p->pos.z;
        gpu->size = p->baseSize * scale; gpu->color = p->color;
        u32 rotBits = (u32)(vclamp(p->rotation / 6.2831853f, 0.0f, 1.0f) * 255.0f); u32 texBits = p->textureIndex + p->animFrame;
        u32 flags = p->flags; gpu->data0 = (flags << 24) | ((texBits & 0xFFFF) << 8) | (rotBits & 0xFF);
        u32 softByte = (u32)(vclamp(em->softness, 0.0f, 255.0f / 16.0f) * 16.0f + 0.5f) & 0xFF;
        gpu->data1 = softByte | ((u32)p->blendMode << 8); gpu->pad = 0;
        V3 delta = {p->pos.x - camPos.x, p->pos.y - camPos.y, p->pos.z - camPos.z}; float dist = delta.x * camForward.x + delta.y * camForward.y + delta.z * camForward.z;
        u32 depthKey = (u32)vclamp((dist + 1000.0f) * 10.0f, 0.0f, 16777215.0f);
        psys.sortKeys[i].sortKey = ((u32)p->blendMode << 24) | (0xFFFFFFu - depthKey); psys.sortKeys[i].index = (u16)i; i++;
    }
}

INLINE int sort_cmp(const void* a, const void* b) { u32 ka = ((const SortEntry*)a)->sortKey; u32 kb = ((const SortEntry*)b)->sortKey; return (ka > kb) - (ka < kb); }
void ParticleSystem_Sort(void) { if (psys.aliveCount > 1) { qsort_new(psys.sortKeys, psys.aliveCount, sizeof(SortEntry), sort_cmp); } }
void ParticleSystem_Upload(void) {
    if (psys.aliveCount == 0) return;
    GpuParticleInstance* sortedInstances = (GpuParticleInstance*)OS_AllocScratch(psys.aliveCount * sizeof(GpuParticleInstance)); for (u32 i = 0; i < psys.aliveCount; i++) { sortedInstances[i] = psys.gpuInstances[psys.sortKeys[i].index]; }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, psys.instanceBuffer); glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, psys.aliveCount * sizeof(GpuParticleInstance), sortedInstances);
}

void ParticleSystem_Render(float* viewProj, V3 camPos, V3 camRight, V3 camUp, V3 camForward, u32 depthTex, float near, float far, float viewW, float viewH) {
    if (psys.aliveCount == 0) return;
    glUseProgram(psys.particleSP); glBindVertexArray(psys.quadVAO); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PARTICLE_SSBO_BINDING, psys.instanceBuffer);
    glUniformMatrix4fv(0, 1, GL_FALSE, viewProj); glUniform3f(1, camPos.x, camPos.y, camPos.z); glUniform3f(2, camRight.x, camRight.y, camRight.z); glUniform3f(3, camUp.x, camUp.y, camUp.z); glUniform3f(4, camForward.x, camForward.y, camForward.z);
    glUniform1i(10, 8); glUniform2f(11, viewW, viewH); glUniform1f(12, near); glUniform1f(13, far); glActiveTexture(GL_TEXTURE0 + 8); glBindTexture(GL_TEXTURE_2D, depthTex); glEnable(GL_BLEND); glDepthMask(GL_FALSE); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glDisable(GL_CULL_FACE); 
    u32 runStart = 0;
    while (runStart < psys.aliveCount) {
        i32 blend = (i32)(psys.sortKeys[runStart].sortKey >> 24); u32 runEnd = runStart + 1; while (runEnd < psys.aliveCount && (psys.sortKeys[runEnd].sortKey >> 24) == (u32)blend) runEnd++;
        glUniform1i(5, (i32)runStart); glUniform1i(7, blend); if (blend == 1) glBlendFunc(GL_ONE, GL_ONE); else if (blend == 2) glBlendFunc(GL_DST_COLOR, GL_ZERO); else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        u32 count = runEnd - runStart; glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count); drawCalls++; vertsRendered += count * 4; runStart = runEnd;
    }
    glEnable(GL_CULL_FACE); glDepthMask(GL_TRUE); glDisable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthFunc(GL_LESS);
}

void ParticleSystem_SpawnTrail(V3 p0, V3 p1, u32 texIndex, u32 emitterIndex, float lifetime, float birth0) {
    if (psys.trailCount >= MAX_TRAIL_SEGMENTS) return;
    TrailSegmentInstance* seg = &psys.trailSegments[psys.trailCount++]; float now = (float)World.pauseRelativeTime;
    seg->p0x = p0.x; seg->p0y = p0.y; seg->p0z = p0.z; seg->p1x = p1.x; seg->p1y = p1.y; seg->p1z = p1.z; seg->padA = 0.0f; seg->padB = 0.0f; seg->c00x = seg->c00y = seg->c00z = seg->c00w = 0.0f; seg->c01x = seg->c01y = seg->c01z = seg->c01w = 0.0f; seg->c10x = seg->c10y = seg->c10z = seg->c10w = 0.0f; seg->c11x = seg->c11y = seg->c11z = seg->c11w = 0.0f; seg->color0 = 0; seg->color1 = 0; seg->uvData = (texIndex & 0xFFFF) | ((emitterIndex & 0xFFFF) << 16); seg->birth0 = birth0; seg->birth1 = now; seg->deathTime = now + lifetime; seg->pad0 = 0.0f; seg->pad1 = 0.0f;
}

void ParticleSystem_PruneTrails(void) {
    float now = (float)World.pauseRelativeTime; u32 out = 0;
    for (u32 i = 0; i < psys.trailCount; i++) { TrailSegmentInstance* s = &psys.trailSegments[i]; if (s->deathTime > now) { if (out != i) psys.trailSegments[out] = *s; out++; } }
    psys.trailCount = out;
}
static V3 trailSortCam;
INLINE int trail_cmp(const void* a, const void* b) {
    const TrailSegmentInstance* ta = (const TrailSegmentInstance*)a; const TrailSegmentInstance* tb = (const TrailSegmentInstance*)b;
    float ax = (ta->p0x + ta->p1x) * 0.5f - trailSortCam.x, ay = (ta->p0y + ta->p1y) * 0.5f - trailSortCam.y, az = (ta->p0z + ta->p1z) * 0.5f - trailSortCam.z;
    float bx = (tb->p0x + tb->p1x) * 0.5f - trailSortCam.x, by = (tb->p0y + tb->p1y) * 0.5f - trailSortCam.y, bz = (tb->p0z + tb->p1z) * 0.5f - trailSortCam.z;
    float da = ax * ax + ay * ay + az * az, db = bx * bx + by * by + bz * bz;
    return (da > db) ? -1 : ((da < db) ? 1 : 0);
}
void ParticleSystem_RenderTrails(float* viewProj, V3 camPos, V3 camRight, V3 camUp) {
    (void)camUp;
    if (psys.trailCount == 0) return;
    float now = (float)World.pauseRelativeTime;
    for (u32 i = 0; i < psys.trailCount; i++) {
        TrailSegmentInstance* s = &psys.trailSegments[i]; u16 emIdx = (u16)(s->uvData >> 16); Emitter* em = (emIdx < MAX_EMITTERS && psys.emitters[emIdx].active) ? &psys.emitters[emIdx] : NULL;
        float life = em ? em->trailLifetime : 1.0f;
        float t0 = (life > 0.0f) ? vclamp((now - s->birth0) / life, 0.0f, 1.0f) : 0.0f;
        float t1 = (life > 0.0f) ? vclamp((now - s->birth1) / life, 0.0f, 1.0f) : 0.0f;
        float r0, g0, b0, a0, r1, g1, b1, a1;
        if (em) unpack_rgba8(em->trailColorStart, &r0, &g0, &b0, &a0); else { r0 = g0 = b0 = a0 = 1.0f; }
        if (em) unpack_rgba8(em->trailColorEnd, &r1, &g1, &b1, &a1); else { r1 = g1 = b1 = a1 = 1.0f; }
        s->color0 = pack_rgba8(r0 + (r1 - r0) * t0, g0 + (g1 - g0) * t0, b0 + (b1 - b0) * t0, a0 + (a1 - a0) * t0);
        s->color1 = pack_rgba8(r0 + (r1 - r0) * t1, g0 + (g1 - g0) * t1, b0 + (b1 - b0) * t1, a0 + (a1 - a0) * t1);
        float w0 = em ? em->trailWidthStart : 0.05f, w1 = em ? em->trailWidthEnd : 0.05f;
        float w0o = w0 + (w1 - w0) * t0, w1o = w0 + (w1 - w0) * t1;
        float sx = s->p1x - s->p0x, sy = s->p1y - s->p0y, sz = s->p1z - s->p0z; float sl = vsqrtf(sx*sx + sy*sy + sz*sz); if (sl < 1e-6f) { sx = 0.0f; sy = 0.0f; sz = 1.0f; sl = 1.0f; } else { sx /= sl; sy /= sl; sz /= sl; }
        float cax = 2.0f * s->p0x - s->p1x, cay = 2.0f * s->p0y - s->p1y, caz = 2.0f * s->p0z - s->p1z; float wa = 2.0f * w0o - w1o;
        float vx = camPos.x - cax, vy = camPos.y - cay, vz = camPos.z - caz; float vl = vsqrtf(vx*vx + vy*vy + vz*vz); if (vl < 1e-6f) { vx = 0.0f; vy = 0.0f; vz = 1.0f; vl = 1.0f; } else { vx /= vl; vy /= vl; vz /= vl; }
        float a0x = sy*vz - sz*vy, a0y = sz*vx - sx*vz, a0z = sx*vy - sy*vx; float a0l = vsqrtf(a0x*a0x + a0y*a0y + a0z*a0z); if (a0l < 1e-6f) { a0x = camRight.x; a0y = camRight.y; a0z = camRight.z; } else { a0x /= a0l; a0y /= a0l; a0z /= a0l; }
        s->c00x = cax - a0x * wa; s->c00y = cay - a0y * wa; s->c00z = caz - a0z * wa; s->c01x = cax + a0x * wa; s->c01y = cay + a0y * wa; s->c01z = caz + a0z * wa;
        vx = camPos.x - s->p1x; vy = camPos.y - s->p1y; vz = camPos.z - s->p1z; vl = vsqrtf(vx*vx + vy*vy + vz*vz); if (vl < 1e-6f) { vx = 0.0f; vy = 0.0f; vz = 1.0f; vl = 1.0f; } else { vx /= vl; vy /= vl; vz /= vl; }
        float a1x = sy*vz - sz*vy, a1y = sz*vx - sx*vz, a1z = sx*vy - sy*vx; float a1l = vsqrtf(a1x*a1x + a1y*a1y + a1z*a1z); if (a1l < 1e-6f) { a1x = camRight.x; a1y = camRight.y; a1z = camRight.z; } else { a1x /= a1l; a1y /= a1l; a1z /= a1l; }
        s->c10x = s->p1x - a1x * w1o; s->c10y = s->p1y - a1y * w1o; s->c10z = s->p1z - a1z * w1o; s->c11x = s->p1x + a1x * w1o; s->c11y = s->p1y + a1y * w1o; s->c11z = s->p1z + a1z * w1o;
    }
    for (u32 i = 0; i < psys.trailCount; i++) {
        TrailSegmentInstance* a = &psys.trailSegments[i];
        for (u32 j = 0; j < psys.trailCount; j++) {
            if (j == i) continue;
            TrailSegmentInstance* b = &psys.trailSegments[j];
            if (!(a->p1x == b->p0x && a->p1y == b->p0y && a->p1z == b->p0z)) continue;
            a->c10x = b->c00x = (a->c10x + b->c00x) * 0.5f; a->c10y = b->c00y = (a->c10y + b->c00y) * 0.5f; a->c10z = b->c00z = (a->c10z + b->c00z) * 0.5f;
            a->c11x = b->c01x = (a->c11x + b->c01x) * 0.5f; a->c11y = b->c01y = (a->c11y + b->c01y) * 0.5f; a->c11z = b->c01z = (a->c11z + b->c01z) * 0.5f;
        }
    }
    trailSortCam = camPos; if (psys.trailCount > 1) qsort_new(psys.trailSegments, psys.trailCount, sizeof(TrailSegmentInstance), trail_cmp);
    glUseProgram(psys.trailSP); glBindVertexArray(psys.quadVAO); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TRAIL_SSBO_BINDING, psys.trailBuffer);
    glUniformMatrix4fv(0, 1, GL_FALSE, viewProj);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, psys.trailCount * sizeof(TrailSegmentInstance), psys.trailSegments);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glDisable(GL_CULL_FACE);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, psys.trailCount); drawCalls++; vertsRendered += psys.trailCount * 4;
    glEnable(GL_CULL_FACE); glDepthMask(GL_TRUE); glDisable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthFunc(GL_LESS);
}

void ParticleSystem_Update(float dt) { ParticleSystem_PruneTrails(); ParticleSystem_UpdateEmitters(dt); ParticleSystem_Simulate(dt); ParticleSystem_Sort(); ParticleSystem_Upload(); }
void ParticleSystem_SetPrograms(u32 particleSP, u32 trailSP) { psys.particleSP = particleSP; psys.trailSP = trailSP; }
