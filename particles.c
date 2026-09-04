// particles.c - CPU-simulated, GPU-instanced particle system for Voxen
#include "common.h"
typedef struct Particle { float pos[3],vel[3],age,invLifetime,baseSize,rotation,angularVelocity; u32 color,emitterIndex; u16 flags,textureIndex,animFrame; u32 seed; } Particle;
typedef struct Emitter { bool active; float transform[16],position[3],emitAccumulator,emitRate,age,duration; u16 aliveCount,maxAlive; u8 blendMode,materialMode,lightingMode,physicsMode; float lifetimeMin,lifetimeMax,sizeMin,sizeMax,speedMin,speedMax,rotationMin,rotationMax,angularVelocityMin,angularVelocityMax,bounce,particleRadius; 
                         u32 textureBaseIndex,textureFrameCount; float animSpeed,softness,nearCameraFade,scaleCurve[32],velocityCurve[32],rotationCurve[32],emissionCurve[32]; u32 colorRamp[64]; u64 rngState; } Emitter;
typedef struct GpuParticleInstance { float x,y,z,size; u32 color,data0,data1,pad; } GpuParticleInstance;
typedef struct TrailSegmentInstance { float p0x,p0y,p0z,width0,p1x,p1y,p1z,width1; u32 color0,color1,uvData,pad; } TrailSegmentInstance;
typedef struct { u32 sortKey; u16 index; } SortEntry;
typedef struct { Particle particles[MAX_PARTICLES]; Emitter emitters[MAX_EMITTERS]; GpuParticleInstance gpuInstances[MAX_PARTICLES]; TrailSegmentInstance trailSegments[MAX_TRAIL_SEGMENTS]; SortEntry sortKeys[MAX_PARTICLES]; u32 aliveCount,trailCount,instanceBuffer,trailBuffer,quadVAO,quadVBO,particleSP,trailSP; bool initialized; } ParticleSystem;
ParticleSystem psys = {0};
#define PARTICLE_FLAG_ADDITIVE      (1u << 0)
#define PARTICLE_FLAG_SOFT          (1u << 1)
#define PARTICLE_FLAG_LIT           (1u << 2)
#define PARTICLE_FLAG_MULTIPLY      (1u << 3)
#define PARTICLE_FLAG_SOFT_OCCLUDE  (1u << 4)
#define PARTICLE_FLAG_PHYSICS       (1u << 5)
#define PARTICLE_FLAG_TRAIL         (1u << 6)
INLINE u32 xs32(u64* state) { u64 x = *state; x ^= x << 13; x ^= x >> 7; x ^= x << 17; *state = x; return (u32)(x ^ (x >> 32)); }
INLINE float randf(u64* state) { return (float)(xs32(state) & 0xFFFFFF) * (1.0f / 16777216.0f); }
INLINE float randf_range(u64* state, float a, float b) { return a + (b - a) * randf(state); }
INLINE u32 pack_rgba8(float r, float g, float b, float a) { u32 ri = (u32)(vclamp(r, 0.0f, 1.0f) * 255.0f + 0.5f); u32 gi = (u32)(vclamp(g, 0.0f, 1.0f) * 255.0f + 0.5f); u32 bi = (u32)(vclamp(b, 0.0f, 1.0f) * 255.0f + 0.5f); u32 ai = (u32)(vclamp(a, 0.0f, 1.0f) * 255.0f + 0.5f); return (ai << 24) | (bi << 16) | (gi << 8) | ri; }
INLINE void unpack_rgba8(u32 packed, float* r, float* g, float* b, float* a) { *r = ((packed >> 0) & 0xFF) * (1.0f / 255.0f); *g = ((packed >> 8) & 0xFF) * (1.0f / 255.0f); *b = ((packed >> 16) & 0xFF) * (1.0f / 255.0f); *a = ((packed >> 24) & 0xFF) * (1.0f / 255.0f); }
INLINE void build_color_ramp(Emitter* em, const u32* keys, int numKeys) {
    for (int i = 0; i < 64; i++) {
        float t = (float)i / 63.0f; int keyIdx = 0; while (keyIdx < numKeys - 1 && keys[keyIdx * 2 + 1] <= (u32)(t * 65535.0f)) keyIdx++;
        if (keyIdx >= numKeys - 1) { em->colorRamp[i] = keys[(numKeys - 1) * 2]; }
        else {
            float t0 = keys[keyIdx * 2 + 1] * (1.0f / 65535.0f); float t1 = keys[(keyIdx + 1) * 2 + 1] * (1.0f / 65535.0f); float localT = (t - t0) / (t1 - t0); u32 c0 = keys[keyIdx * 2]; u32 c1 = keys[(keyIdx + 1) * 2]; float r0, g0, b0, a0, r1, g1, b1, a1; unpack_rgba8(c0, &r0, &g0, &b0, &a0); unpack_rgba8(c1, &r1, &g1, &b1, &a1);
            float r = r0 + (r1 - r0) * localT, g = g0 + (g1 - g0) * localT, b = b0 + (b1 - b0) * localT, a = a0 + (a1 - a0) * localT; em->colorRamp[i] = pack_rgba8(r, g, b, a);
        }
    }
}

INLINE u32 sample_color_ramp(Emitter* em, float t) { int idx = (int)(t * 63.0f); if (idx < 0) idx = 0; if (idx >= 64) idx = 63; return em->colorRamp[idx]; }
INLINE float sample_curve(const float* curve, float t) { int idx = (int)(t * 31.0f); if (idx < 0) idx = 0; if (idx >= 31) return curve[31]; float localT = t * 31.0f - idx; return curve[idx] + (curve[idx + 1] - curve[idx]) * localT; }
void ParticleSystem_Init(void) {
    if (psys.initialized) return;
    glGenVertexArrays(1, &psys.quadVAO); glGenBuffers(1, &psys.quadVBO);
    static const float quadVerts[16] = {-1.0f, -1.0f, 0.0f, 0.0f,1.0f, -1.0f, 1.0f, 0.0f,-1.0f,  1.0f, 0.0f, 1.0f,1.0f,  1.0f, 1.0f, 1.0f,};
    glBindVertexArray(psys.quadVAO); glBindBuffer(GL_ARRAY_BUFFER, psys.quadVBO); glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW); 
    glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0); glVertexAttribBinding(0, 0); glEnableVertexAttribArray(0);
    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float)); glVertexAttribBinding(1, 0); glEnableVertexAttribArray(1); glBindVertexBuffer(0, psys.quadVBO, 0, 4 * sizeof(float));
    glGenBuffers(1, &psys.instanceBuffer); glBindBuffer(GL_SHADER_STORAGE_BUFFER, psys.instanceBuffer); glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof(GpuParticleInstance), NULL, GL_DYNAMIC_DRAW); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PARTICLE_SSBO_BINDING, psys.instanceBuffer);
    glGenBuffers(1, &psys.trailBuffer); glBindBuffer(GL_SHADER_STORAGE_BUFFER, psys.trailBuffer); glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_TRAIL_SEGMENTS * sizeof(TrailSegmentInstance), NULL, GL_DYNAMIC_DRAW); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TRAIL_SSBO_BINDING, psys.trailBuffer);
    psys.initialized = true; DualLog("Particle system initialized: %d max particles, %d max trail segments\n", MAX_PARTICLES, MAX_TRAIL_SEGMENTS);
}

u16 ParticleSystem_AddEmitter(const float* position, u32 textureIndex, float emitRate, float lifetime, float sizeMin, float sizeMax, float speedMin, float speedMax, u32 colorStart, u32 colorEnd, u8 blendMode) {
    for (u16 i = 0; i < MAX_EMITTERS; i++) {
        if (!psys.emitters[i].active) {
            Emitter* em = &psys.emitters[i]; em->active = true; mset(em->transform, 0, 16 * sizeof(float)); em->transform[0] = em->transform[5] = em->transform[10] = em->transform[15] = 1.0f; em->position[0] = position[0]; em->position[1] = position[1]; em->position[2] = position[2];
            em->emitAccumulator = 0.0f; em->emitRate = emitRate; em->age = 0.0f; em->duration = lifetime; em->aliveCount = 0; em->maxAlive = 2000; em->blendMode = blendMode; em->materialMode = 0; em->lightingMode = 0; em->physicsMode = 0; em->lifetimeMin = 0.5f; em->lifetimeMax = 2.0f;
            em->sizeMin = sizeMin; em->sizeMax = sizeMax; em->speedMin = speedMin; em->speedMax = speedMax; em->rotationMin = 0.0f; em->rotationMax = 6.2831853f; em->angularVelocityMin = -1.0f; em->angularVelocityMax = 1.0f; em->bounce = 0.3f; em->particleRadius = 0.1f; em->textureBaseIndex = textureIndex;
            em->textureFrameCount = 1; em->animSpeed = 10.0f; em->softness = 1.0f; em->nearCameraFade = 1.0f; for (int c = 0; c < 32; c++) { em->scaleCurve[c] = 1.0f; em->velocityCurve[c] = 1.0f; em->rotationCurve[c] = 0.0f; em->emissionCurve[c] = 1.0f; }
            u32 rampKeys[4] = {colorStart, 0, colorEnd, 65535}; build_color_ramp(em, rampKeys, 2); em->rngState = globalframe * 1234567 + i * 98765 + 1; return i;
        }
    } return U16_MAX;
}

void ParticleSystem_UpdateEmitters(float dt) {
    for (u16 i = 0; i < MAX_EMITTERS; i++) {
        Emitter* em = &psys.emitters[i]; if (!em->active) continue;
        em->age += dt; if (em->duration > 0.0f && em->age >= em->duration) { em->active = false; continue; }
        float rate = em->emitRate * sample_curve(em->emissionCurve, em->age / (em->duration > 0 ? em->duration : 1.0f));
        em->emitAccumulator += rate * dt; int count = (int)em->emitAccumulator; em->emitAccumulator -= (float)count; 
        for (int p = 0; p < count; p++) {
            if (psys.aliveCount >= MAX_PARTICLES) break;
            if (em->aliveCount >= em->maxAlive) break;
            Particle* part = &psys.particles[psys.aliveCount];
            float angle = randf_range(&em->rngState, 0.0f, 6.2831853f);
            float elevation = randf_range(&em->rngState, -0.5f, 0.5f);
            float speed = randf_range(&em->rngState, em->speedMin, em->speedMax);
            float lifetime = randf_range(&em->rngState, em->lifetimeMin, em->lifetimeMax);
            part->pos[0] = em->position[0]; part->pos[1] = em->position[1]; part->pos[2] = em->position[2];
            part->vel[0] = vcosf(angle) * speed; part->vel[1] = elevation * speed; part->vel[2] = vsinf(angle) * speed;
            part->age = 0.0f; part->invLifetime = 1.0f / lifetime;
            part->baseSize = randf_range(&em->rngState, em->sizeMin, em->sizeMax); part->rotation = randf_range(&em->rngState, em->rotationMin, em->rotationMax);
            part->angularVelocity = randf_range(&em->rngState, em->angularVelocityMin, em->angularVelocityMax);
            part->color = sample_color_ramp(em, 0.0f); part->emitterIndex = i; part->flags = 0; if (em->blendMode == BLEND_PREMULT) { part->flags |= PARTICLE_FLAG_ADDITIVE; }
            part->textureIndex = em->textureBaseIndex; part->animFrame = 0; part->seed = xs32(&em->rngState); psys.aliveCount++; em->aliveCount++;
        }
    }
}

void ParticleSystem_Simulate(float dt) {
    u32 i = 0; V3 camPos = World.position[PLAYER1], camForward = World.instances[PLAYER1].forward;
    while (i < psys.aliveCount) {
        Particle* p = &psys.particles[i]; Emitter* em = &psys.emitters[p->emitterIndex]; p->age += dt;
        if (p->age >= 1.0f / p->invLifetime) { psys.aliveCount--; em->aliveCount--; psys.particles[i] = psys.particles[psys.aliveCount]; continue; }
        float t = p->age * p->invLifetime; float scale = sample_curve(em->scaleCurve, t); float velScale = sample_curve(em->velocityCurve, t); float rotAdd = sample_curve(em->rotationCurve, t) * dt;
        p->pos[0] += p->vel[0] * velScale * dt; p->pos[1] += p->vel[1] * velScale * dt; p->pos[2] += p->vel[2] * velScale * dt; p->rotation += p->angularVelocity * dt + rotAdd;
        if (em->physicsMode && (p->flags & PARTICLE_FLAG_PHYSICS)) { p->vel[1] -= 9.81f * dt; if (p->pos[1] < 0.0f) { p->pos[1] = 0.0f; p->vel[1] *= -em->bounce; p->vel[0] *= 0.9f; p->vel[2] *= 0.9f; } }
        u32 rampColor = sample_color_ramp(em, t);
        float sr, sg, sb, sa; unpack_rgba8(rampColor, &sr, &sg, &sb, &sa);
        float pr, pg, pb, pa; unpack_rgba8(p->color, &pr, &pg, &pb, &pa);
        p->color = pack_rgba8(pr * sr, pg * sg, pb * sb, pa * sa);
        if (em->textureFrameCount > 1) { p->animFrame = (u16)(t * em->animSpeed * em->textureFrameCount) % em->textureFrameCount; }
        GpuParticleInstance* gpu = &psys.gpuInstances[i];
        gpu->x = p->pos[0]; gpu->y = p->pos[1]; gpu->z = p->pos[2];
        gpu->size = p->baseSize * scale; gpu->color = p->color;
        u32 rotBits = (u32)(vclamp(p->rotation / 6.2831853f, 0.0f, 1.0f) * 255.0f); u32 texBits = p->textureIndex + p->animFrame;
        u32 flags = p->flags; gpu->data0 = (flags << 24) | ((texBits & 0xFFFF) << 8) | (rotBits & 0xFF); gpu->data1 = 0; gpu->pad = 0;
        V3 delta = {p->pos[0] - camPos.x, p->pos[1] - camPos.y, p->pos[2] - camPos.z}; float dist = delta.x * camForward.x + delta.y * camForward.y + delta.z * camForward.z;
        psys.sortKeys[i].sortKey = (u32)(vclamp((dist + 1000.0f) * 10.0f, 0.0f, 4294967295.0f)); psys.sortKeys[i].index = (u16)i; i++;
    }
}

INLINE int sort_cmp(const void* a, const void* b) { u32 ka = ((const SortEntry*)a)->sortKey; u32 kb = ((const SortEntry*)b)->sortKey; return (ka > kb) - (ka < kb); }
void ParticleSystem_Sort(void) { if (psys.aliveCount > 1) { qsort_new(psys.sortKeys, psys.aliveCount, sizeof(SortEntry), sort_cmp); } }
void ParticleSystem_Upload(void) {
    if (psys.aliveCount == 0) return;
    GpuParticleInstance* sortedInstances = (GpuParticleInstance*)OS_AllocScratch(psys.aliveCount * sizeof(GpuParticleInstance)); for (u32 i = 0; i < psys.aliveCount; i++) { sortedInstances[i] = psys.gpuInstances[psys.sortKeys[i].index]; }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, psys.instanceBuffer); glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, psys.aliveCount * sizeof(GpuParticleInstance), sortedInstances);
}

void ParticleSystem_Render(float* viewProj, V3 camPos, V3 camRight, V3 camUp, V3 camForward, u32 depthTex) {
    if (psys.aliveCount == 0) return;
    glUseProgram(psys.particleSP); glBindVertexArray(psys.quadVAO); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PARTICLE_SSBO_BINDING, psys.instanceBuffer);
    glUniformMatrix4fv(0, 1, GL_FALSE, viewProj); glUniform3f(1, camPos.x, camPos.y, camPos.z); glUniform3f(2, camRight.x, camRight.y, camRight.z); glUniform3f(3, camUp.x, camUp.y, camUp.z); glUniform3f(4, camForward.x, camForward.y, camForward.z);
    u32 tex = 0; u8 blend = 0; for (u16 i = 0; i < MAX_EMITTERS; ++i) { if (psys.emitters[i].active) { tex = psys.emitters[i].textureBaseIndex; blend = particleBlendTexture[tex]; break; } }
    glUniform1i(5, 0); glUniform1i(6, 0); glUniform1i(7, (i32)blend); glUniform1ui(8, tex); glUniform1i(9, 1);
    glActiveTexture(GL_TEXTURE0 + 8); glBindTexture(GL_TEXTURE_2D, depthTex); 
    glEnable(GL_BLEND); if (blend == 1) glBlendFunc(GL_ONE, GL_ONE); else if (blend == 2) glBlendFunc(GL_DST_COLOR, GL_ZERO); else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glDisable(GL_CULL_FACE);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, psys.aliveCount); drawCalls++; vertsRendered += psys.aliveCount * 4;
    glEnable(GL_CULL_FACE); glDepthMask(GL_TRUE); glDisable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthFunc(GL_LESS);
}

void ParticleSystem_SpawnTrail(V3 p0, V3 p1, float width0, float width1, u32 color0, u32 color1) {
    if (psys.trailCount >= MAX_TRAIL_SEGMENTS) return;
    TrailSegmentInstance* seg = &psys.trailSegments[psys.trailCount++]; seg->p0x = p0.x; seg->p0y = p0.y; seg->p0z = p0.z; seg->width0 = width0; seg->p1x = p1.x; seg->p1y = p1.y; seg->p1z = p1.z; seg->width1 = width1; seg->color0 = color0; seg->color1 = color1; seg->uvData = 0; seg->pad = 0;
}

void ParticleSystem_RenderTrails(float* viewProj, V3 camPos, V3 camRight, V3 camUp) {
    if (psys.trailCount == 0) return;
    glUseProgram(psys.trailSP); glBindVertexArray(psys.quadVAO); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TRAIL_SSBO_BINDING, psys.trailBuffer);
    glUniformMatrix4fv(0, 1, GL_FALSE, viewProj); glUniform3f(1, camPos.x, camPos.y, camPos.z); glUniform3f(2, camRight.x, camRight.y, camRight.z); glUniform3f(3, camUp.x, camUp.y, camUp.z);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, psys.trailCount * sizeof(TrailSegmentInstance), psys.trailSegments);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glDisable(GL_CULL_FACE);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, psys.trailCount); drawCalls++; vertsRendered += psys.trailCount * 4;
    glEnable(GL_CULL_FACE); glDepthMask(GL_TRUE); glDisable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthFunc(GL_LESS); psys.trailCount = 0;
}

void ParticleSystem_Update(float dt) { ParticleSystem_UpdateEmitters(dt); ParticleSystem_Simulate(dt); ParticleSystem_Sort(); ParticleSystem_Upload(); }
void ParticleSystem_SetPrograms(u32 particleSP, u32 trailSP) { psys.particleSP = particleSP; psys.trailSP = trailSP; }
