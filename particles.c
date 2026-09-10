// particles.c - CPU-simulated, GPU-instanced particle system for Voxen
#include "common.h"
extern u32 psysquadVAO,psysquadVBO,psysInstancesID,particleSP,psysTrailsID,trailSP; PSys psys = {0}; void PSys_SpawnTrail(V3 p0, V3 p1, u32 texIndex, u32 emitterIndex, float lifetime, float birth0);
INLINE u32 pack_rgba8(float r, float g, float b, float a) { return (((u32)(vclamp(a,0,1.f)*255.0f + .5f)) << 24) | (((u32)(vclamp(b,0,1.f)*255.0f + .5f)) << 16) | (((u32)(vclamp(g,0,1.f)*255.0f + .5f)) << 8) | ((u32)(vclamp(r,0,1.f)*255.0f + .5f)); }
INLINE void unpack_rgba8(u32 p, float* r, float* g, float* b, float* a) { *r = (float)((p >> 0) & 0xFF)/255.0f; *g = (float)((p >> 8) & 0xFF)/255.0f; *b = (float)((p >> 16) & 0xFF)/255.0f; *a = (float)((p >> 24) & 0xFF)/255.0f; }
INLINE u32 ColorToU32(Color c) { return pack_rgba8(c.r, c.g, c.b, c.a); }
INLINE void build_color_ramp(Emitter* e, const Color* c, const float* d, int n) {for(int i=0;i<64;++i){float t=(float)i/63.0f; int k=0; while(k<n-1&&d[k+1]<=t){k++;} if (k>=n-1){e->colorRamp[i]=ColorToU32(c[n-1]);}else{float a=(t-d[k])/(d[k+1]-d[k]); Color c0=c[k],c1=c[k+1]; e->colorRamp[i]=pack_rgba8(c0.r+(c1.r-c0.r)*a,c0.g+(c1.g-c0.g)*a,c0.b+(c1.b-c0.b)*a,c0.a+(c1.a-c0.a)*a);}}}
INLINE void build_curve(float* curve, const float* keys, const float* times, int numKeys) { for (int i=0;i<32;++i){float t=(float)i/31.0f; int k=0; while(k<numKeys-1 && times[k+1]<=t){k++;} if(k>=numKeys-1){curve[i]=keys[numKeys-1];}else{float localT=(t-times[k])/(times[k+1]-times[k]); curve[i]=keys[k]+(keys[k+1]-keys[k])*localT;}} }
INLINE u32 sample_color_ramp(Emitter* em, float t) { int idx = (int)(t * 63.0f); if (idx < 0) idx = 0; if (idx >= 64) idx = 63; return em->colorRamp[idx]; }
INLINE float sample_curve(const float* curve, float t) { int idx = (int)(t * 31.0f); if (idx < 0) idx = 0; if (idx >= 31) return curve[31]; float localT = t * 31.0f - idx; return curve[idx] + (curve[idx + 1] - curve[idx]) * localT; }
u16 PSysAdd(const PSysDef* pd) {
    for (u16 i = 0; i < MAX_EMITTERS; i++) {
        if (psys.emitters[i].active || psys.emitters[i].aliveCount){continue;} Emitter* em=&psys.emitters[i]; u16 fc=0; while(fc<16 && pd->textures[fc]!=MAX_TXRS){fc++;} if(fc == 0){fc=1;} em->active=true; em->position = pd->pos; em->emitAccumulator=em->age=0; em->emitRate = pd->emitRate; em->duration = pd->duration; em->aliveCount = 0;
        em->maxAlive=2000; em->physicsMode=(pd->gravity != 0) ? 1 : 0; em->trail=pd->trail; em->trailTexture = (u16)(pd->trailTexture & 0xFFFFu); em->lifetimeMin=pd->lifetimeMin>0.0f ? pd->lifetimeMin : 0.5f; em->lifetimeMax=pd->lifetimeMax>0 ? pd->lifetimeMax : 2.0f; if(em->lifetimeMax <= em->lifetimeMin){em->lifetimeMax=em->lifetimeMin + 0.01f;}
        em->sizeMin=pd->sizeMin; em->sizeMax=pd->sizeMax; em->speedMin = pd->speedMin; em->speedMax = pd->speedMax; em->rotMin=0; em->rotMax = 6.2831853f; em->aVelMin=pd->rotCount >= 1 ? pd->rotKeys[0] : -1.0f; em->aVelMax=em->aVelMin; em->gravity=pd->gravity; em->trailLifetime=pd->trailLifetime>0.0f ? pd->trailLifetime : 1.0f; 
        em->trailColorStart=ColorToU32(pd->trailColorStart); em->trailColorEnd=ColorToU32(pd->trailColorEnd); em->trailWidthStart=pd->trailWidthStart>0 ? pd->trailWidthStart : 0.05f; em->trailWidthEnd=(pd->trailWidthEnd>0) ? pd->trailWidthEnd : em->trailWidthStart; em->texBaseIdx=pd->textures[0]; em->textureFrameCount = fc; em->animSpeed = 10.0f;
        em->animWindow=pd->animWindow>0 ? pd->animWindow : 1.0f; em->softness=pd->softness>0 ? pd->softness : 1.0f; for(int c=0;c<32;++c){em->scaleCurve[c]=1.0f; em->velocityCurve[c]=1.0f; em->rotationCurve[c]=0; em->emissionCurve[c] = 1.0f; }
        if(pd->rampCount >= 2){build_color_ramp(em,pd->rampColors,pd->rampTimes,pd->rampCount);}else{Color cc[2]={pd->colorStart,pd->colorEnd}; float tt[2]={0,1.0f}; build_color_ramp(em,cc,tt,2);} if(pd->scaleCount >= 2){build_curve(em->scaleCurve,pd->scaleKeys,pd->scaleTimes,pd->scaleCount);}
        if(pd->velCount >= 2){build_curve(em->velocityCurve,pd->velKeys,pd->velTimes,pd->velCount);} if(pd->rotCount >= 2){build_curve(em->rotationCurve,pd->rotKeys,pd->rotTimes,pd->rotCount);} if(pd->emissCount >= 2){build_curve(em->emissionCurve,pd->emissKeys,pd->emissTimes,pd->emissCount);} return i;
    } return U16_MAX;
}

void PSys_UpdateEmitters(float dt) {
    for (u16 i = 0; i < MAX_EMITTERS; i++) {
        Emitter* em = &psys.emitters[i]; if(!em->active){continue;} em->age+=dt; if(em->duration > 0.0f&& em->age>=em->duration){em->active=false; continue;}
        float rate = em->emitRate; if (em->duration > 0.0f && em->duration < 1e6f){rate*=sample_curve(em->emissionCurve,em->age/em->duration);} em->emitAccumulator+=rate*dt; int count=(int)em->emitAccumulator; em->emitAccumulator-=(float)count; 
        for (int p = 0; p < count; p++) {
            if ((psys.aliveCount >= MAX_PARTICLES) || (em->aliveCount >= em->maxAlive)){break;} Particle* part = &psys.particles[psys.aliveCount]; float angle = random_range(0.0f,6.2831853f), speed = random_range(em->speedMin,em->speedMax);
            part->pos=em->position; part->vel.x=vcosf(angle)*speed; part->vel.y=random_range(-0.5f,0.5f)*speed; part->vel.z=vsinf(angle)*speed; part->age=0.0f; part->invLifetime=1.0f/random_range(em->lifetimeMin,em->lifetimeMax); part->baseSize=random_range(em->sizeMin,em->sizeMax); part->rotation=random_range(em->rotMin,em->rotMax);
            part->angularVelocity = random_range(em->aVelMin,em->aVelMax); part->color = sample_color_ramp(em,0.0f); part->emitterIndex = i; part->flags=0; part->blendMode=particleBlendTexture[em->texBaseIdx]; if (part->blendMode == 1) { part->flags |= PARTICLE_FLAG_ADDITIVE; } else if (part->blendMode == 2) { part->flags |= PARTICLE_FLAG_MULTIPLY; } 
            if (em->softness > 0.0f) { part->flags |= PARTICLE_FLAG_SOFT; } part->textureIndex=em->texBaseIdx; part->animFrame=0; part->trailSample=em->position; part->trailBirth=(float)World.pauseRelativeTime; psys.aliveCount++; em->aliveCount++;
        }
    }
}

void PSys_Simulate(float dt) {
    u32 i=0; V3 camPos=World.position[PLAYER1], camForward=World.instances[PLAYER1].forward;
    while (i < psys.aliveCount) {
        Particle* p = &psys.particles[i]; Emitter* em = &psys.emitters[p->emitterIndex]; p->age += dt; float t=p->age*p->invLifetime,s,vs,r; if (t >= 1.0f) { psys.aliveCount--; em->aliveCount--; psys.particles[i] = psys.particles[psys.aliveCount]; continue; }
        if(em->active){s=sample_curve(em->scaleCurve,t); vs=sample_curve(em->velocityCurve,t); r=sample_curve(em->rotationCurve,t)*dt; p->color=sample_color_ramp(em,t); if(em->textureFrameCount>1){float it=t/(em->animWindow>0.0f ? em->animWindow : 1.0f); p->animFrame=(u16)(it*em->textureFrameCount) % em->textureFrameCount;}} else{s=1.0f; vs=1.0f; r=0;}
        p->pos.x+=p->vel.x*vs*dt; p->pos.y+=p->vel.y*vs*dt; p->pos.z+=p->vel.z*vs*dt; p->rotation+=p->angularVelocity*dt+r; if(em->physicsMode){p->vel.y-=em->gravity*dt;} 
        if(em->trail){float dx=p->pos.x-p->trailSample.x, dy=p->pos.y-p->trailSample.y, dz=p->pos.z-p->trailSample.z; if(dx*dx+dy*dy+dz*dz >= 0.0004f){PSys_SpawnTrail(p->trailSample,p->pos,em->trailTexture,p->emitterIndex,em->trailLifetime,p->trailBirth); p->trailSample=p->pos; p->trailBirth=(float)World.pauseRelativeTime;}} 
        GpuPartInst* gpu=&psys.gpuInstances[i]; gpu->x=p->pos.x; gpu->y=p->pos.y; gpu->z=p->pos.z; gpu->size=p->baseSize*s; gpu->color=p->color; gpu->data0=(p->flags << 24) | (((p->textureIndex+p->animFrame) & 0xFFFF) << 8) | (((u32)(vclamp(p->rotation/6.2831853f,0.0f,1.0f)*255.0f)) & 0xFF);
        gpu->data1=(u32)(vclamp(em->softness,0,255.f/16.f) * 16.f+.5f) & 0xFF; gpu->pad=0; V3 d={p->pos.x-camPos.x,p->pos.y-camPos.y,p->pos.z-camPos.z}; float dist=d.x*camForward.x+d.y*camForward.y+d.z*camForward.z; psys.sortKeys[i].sortKey=((u32)p->blendMode << 24) | (0xFFFFFFu - ((u32)vclamp((dist+1000.0f)*10.0f,0,16777215.f))); psys.sortKeys[i].index=(u16)i; i++;
    }
}

INLINE int sort_cmp(const void* a, const void* b) { u32 ka = ((const PartSortEntry*)a)->sortKey; u32 kb = ((const PartSortEntry*)b)->sortKey; return (ka > kb) - (ka < kb); }
void PSys_Sort(void) { if (psys.aliveCount > 1) { qsort_new(psys.sortKeys, psys.aliveCount, sizeof(PartSortEntry), sort_cmp); } }
void PSys_Upload(void) {if(psys.aliveCount==0){return;} GpuPartInst* s=OS_AllocScratch(psys.aliveCount*sizeof(GpuPartInst)); for(u32 i=0;i<psys.aliveCount;++i){s[i]=psys.gpuInstances[psys.sortKeys[i].index];} glBindBuffer(GL_SSBO, psysInstancesID); glBufferSubData(GL_SSBO,0,psys.aliveCount*sizeof(GpuPartInst),s);}
static V3 trailSortCam;
void PSys_SpawnTrail(V3 p0, V3 p1, u32 texIndex, u32 emitterIndex, float lifetime, float birth0) {
    if(psys.trailCount >= MAX_TRAIL_SEGS){return;} TrlSegInst* seg=&psys.trailSegments[psys.trailCount++]; float now=(float)World.pauseRelativeTime;
    seg->p0x=p0.x; seg->p0y=p0.y; seg->p0z=p0.z; seg->p1x=p1.x; seg->p1y=p1.y; seg->p1z=p1.z; seg->padA=seg->padB=0; seg->c00x=seg->c00y=seg->c00z=seg->c00w=seg->c01x=seg->c01y=seg->c01z=seg->c01w=seg->c10x=seg->c10y=seg->c10z=seg->c10w=seg->c11x=seg->c11y=seg->c11z=seg->c11w=0; seg->color0=seg->color1=0; seg->uvData = (texIndex & 0xFFFF) | ((emitterIndex & 0xFFFF) << 16); seg->birth0 = birth0; seg->birth1 = now; seg->deathTime = now + lifetime; seg->pad0 = 0.0f; seg->pad1 = 0.0f;
}

void PSys_PruneTrails(void) { float now = (float)World.pauseRelativeTime; u32 out = 0; for(u32 i=0;i<psys.trailCount;++i){TrlSegInst* s=&psys.trailSegments[i]; if(s->deathTime > now){if(out != i){psys.trailSegments[out]=*s;} out++;}} psys.trailCount=out;}
INLINE int trail_cmp(const void* a, const void* b) {
    const TrlSegInst* ta=(const TrlSegInst*)a; const TrlSegInst* tb=(const TrlSegInst*)b; float ax=(ta->p0x + ta->p1x)*0.5f - trailSortCam.x, ay=(ta->p0y + ta->p1y)*0.5f - trailSortCam.y, az=(ta->p0z + ta->p1z)*0.5f - trailSortCam.z, bx=(tb->p0x + tb->p1x)*0.5f - trailSortCam.x, by=(tb->p0y + tb->p1y)*0.5f - trailSortCam.y, bz=(tb->p0z + tb->p1z)*0.5f - trailSortCam.z; float da=ax*ax + ay*ay + az*az, db=bx*bx + by*by + bz*bz; return da>db ? -1 : (da<db ? 1 : 0);
}

void PSys_Render(float* viewProj, V3 camPos, V3 camRight, V3 camUp, V3 camForward, u32 depthTex, float near, float far, float viewW, float viewH) {
    if (psys.aliveCount == 0) return; glUseProgram(particleSP); glBindVertexArray(psysquadVAO); glBindBufferBase(GL_SSBO,PARTICLE_SSBO_BINDING,psysInstancesID); glUniformMatrix4fv(0, 1, GL_FALSE, viewProj); glUniform3f(1, camPos.x, camPos.y, camPos.z); glUniform3f(2, camRight.x, camRight.y, camRight.z); glUniform3f(3, camUp.x, camUp.y, camUp.z); glUniform3f(4, camForward.x, camForward.y, camForward.z);
    glUniform1i(10, 8); glUniform2f(11, viewW, viewH); glUniform1f(12, near); glUniform1f(13, far); glActiveTexture(GL_TEXTURE0 + 8); glBindTexture(GL_TEXTURE_2D, depthTex); glEnable(GL_BLEND); glDepthMask(GL_FALSE); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glDisable(GL_CULL_FACE); u32 runStart = 0;
    while (runStart < psys.aliveCount) {
        i32 blend = (i32)(psys.sortKeys[runStart].sortKey >> 24); u32 runEnd = runStart + 1; while (runEnd < psys.aliveCount && (psys.sortKeys[runEnd].sortKey >> 24) == (u32)blend) runEnd++; glUniform1i(5, (i32)runStart); glUniform1i(7, blend); if (blend == 1) glBlendFunc(1,1); else if (blend == 2) glBlendFunc(GL_DST_COLOR, GL_ZERO); else glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        u32 count = runEnd - runStart; glDrawArraysInstanced(GL_TRIANGLE_STRIP,0,4,count); drawCalls++; vertsRendered += count * 4; runStart = runEnd;
    }
    glEnable(GL_CULL_FACE); glDepthMask(GL_TRUE); glDisable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); glDepthFunc(GL_LESS);
    if (psys.trailCount == 0){return;} // Particles barrier <<<<<<<<
    float now = (float)World.pauseRelativeTime;
    for (u32 i = 0; i < psys.trailCount; i++) {
        TrlSegInst* s = &psys.trailSegments[i]; u16 emIdx = (u16)(s->uvData >> 16); Emitter* em = (emIdx < MAX_EMITTERS && psys.emitters[emIdx].active) ? &psys.emitters[emIdx] : NULL;
        float life = em ? em->trailLifetime : 1.0f; float t0=(life > 0.0f) ? vclamp((now - s->birth0) / life, 0.0f, 1.0f) : 0.0f, t1=(life > 0.0f) ? vclamp((now - s->birth1) / life, 0.0f, 1.0f) : 0.0f,r0,g0,b0,a0,r1,g1,b1,a1;
        if (em) unpack_rgba8(em->trailColorStart, &r0, &g0, &b0, &a0); else { r0 = g0 = b0 = a0 = 1.0f; }   if (em) unpack_rgba8(em->trailColorEnd, &r1, &g1, &b1, &a1); else { r1 = g1 = b1 = a1 = 1.0f; }
        s->color0 = pack_rgba8(r0 + (r1 - r0) * t0, g0 + (g1 - g0) * t0, b0 + (b1 - b0) * t0, a0 + (a1 - a0) * t0); s->color1 = pack_rgba8(r0 + (r1 - r0) * t1, g0 + (g1 - g0) * t1, b0 + (b1 - b0) * t1, a0 + (a1 - a0) * t1);
        float w0 = em ? em->trailWidthStart : 0.05f, w1 = em ? em->trailWidthEnd : 0.05f; float w0o = w0 + (w1 - w0) * t0, w1o = w0 + (w1 - w0) * t1;
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
        TrlSegInst* a = &psys.trailSegments[i];
        for (u32 j = 0; j < psys.trailCount; j++) {
            if (j == i){continue;} TrlSegInst* b=&psys.trailSegments[j]; if (!(a->p1x == b->p0x && a->p1y == b->p0y && a->p1z == b->p0z)){continue;} a->c10x = b->c00x = (a->c10x + b->c00x) * 0.5f; a->c10y = b->c00y = (a->c10y + b->c00y) * 0.5f; a->c10z = b->c00z = (a->c10z + b->c00z) * 0.5f; a->c11x = b->c01x = (a->c11x + b->c01x) * 0.5f; a->c11y = b->c01y = (a->c11y + b->c01y) * 0.5f; a->c11z = b->c01z = (a->c11z + b->c01z) * 0.5f;
        }
    }
    trailSortCam=camPos; if(psys.trailCount > 1){qsort_new(psys.trailSegments,psys.trailCount,sizeof(TrlSegInst),trail_cmp);} glUseProgram(trailSP); glBindVertexArray(psysquadVAO); glBindBufferBase(GL_SSBO,TRAIL_SSBO_BINDING,psysTrailsID);
    glUniformMatrix4fv(0,1,GL_FALSE,viewProj); glBufferSubData(GL_SSBO,0,psys.trailCount * sizeof(TrlSegInst), psys.trailSegments); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glDisable(GL_CULL_FACE);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP,0,4,psys.trailCount); drawCalls++; vertsRendered += psys.trailCount * 4; glEnable(GL_CULL_FACE); glDepthMask(GL_TRUE); glDisable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthFunc(GL_LESS);
}

void PSys_Update(float dt) { PSys_PruneTrails(); PSys_UpdateEmitters(dt); PSys_Simulate(dt); PSys_Sort(); PSys_Upload(); }
