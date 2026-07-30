// synth.c — Procedural Audio Engine
// All sounds fire-and-forget via play_synth(SND_X, vol, pitch).
// Room acoustics via synth_set_room(size, wet):
//   size 0.0 = anechoic, 0.3 = small room, 0.6 = large hall, 0.85+ = cave
//   wet  0.0 = dry, 0.25 = subtle, 0.5 = obvious tail
// Call synth_set_room once on area load; reverb applies to all subsequent sounds.

// ---------------------------------------------------------------------------
// Reverb — Schroeder design: 4 parallel combs + 2 serial allpass diffusers
// Max delay = 2.5s worth of samples at AUDIO_RATE, covers cave tails.
// Delay lengths are prime to avoid resonance beating.
// ---------------------------------------------------------------------------
#define REV_BUF_LEN 110251  // ~2.5s @ 44100; prime
static float rev_buf[4][REV_BUF_LEN];  // 4 comb delay lines, ~1.7MB static
static float ap_buf[2][3533];           // 2 allpass lines, prime lengths
static u32   rev_idx[4];
static u32   ap_idx[2];
static float rev_fb[4];     // per-comb feedback gain (set by synth_set_room)
static u32   rev_len[4];    // per-comb delay length in samples
static float rev_lp[4];     // per-comb LP filter state (damps high freqs in tail)
static float rev_wet  = 0.20f;
static float rev_dry  = 1.00f;

// Base delay lengths (small room). Scaled up for larger spaces.
// Chosen as mutually prime to prevent periodicity artifacts.
static const u32 REV_BASE[4] = { 1373, 1607, 1931, 2269 };
// Allpass delay lengths — fixed, short for diffusion not coloration.
static const u32 AP_LEN[2]   = { 379, 547 };

void synth_set_room(float size, float wet) {
    // size 0..1 scales delay lengths from base (small) up to ~5x (cave).
    // feedback derived from desired RT60: longer room = higher feedback.
    float scale = 1.0f + size * 4.0f;
    float fb    = 0.3f + size * 0.62f;  // 0.3 (dead) .. 0.92 (cave)
    for (u32 i = 0; i < 4; i++) {
        u32 len = (u32)(REV_BASE[i] * scale);
        if (len >= REV_BUF_LEN) len = REV_BUF_LEN - 1;
        rev_len[i] = len;
        rev_fb[i]  = fb * (0.97f + i * 0.007f); // slight spread across combs
        rev_lp[i]  = 0.0f;
        rev_idx[i] = 0;
    }
    ap_idx[0] = ap_idx[1] = 0;
    rev_wet = wet;
    rev_dry = 1.0f;
}

// Process one sample through the reverb network. Call after all voices mixed.
static float reverb_tick(float in) {
    // 4 parallel comb filters
    float comb_sum = 0.0f;
    for (u32 i = 0; i < 4; i++) {
        float delayed = rev_buf[i][rev_idx[i]];
        // LP inside feedback loop — damps highs each reflection, like real walls
        rev_lp[i] += 0.5f * (delayed - rev_lp[i]);
        rev_buf[i][rev_idx[i]] = in + rev_lp[i] * rev_fb[i];
        rev_idx[i] = (rev_idx[i] + 1 >= rev_len[i]) ? 0 : rev_idx[i] + 1;
        comb_sum += delayed;
    }
    comb_sum *= 0.25f;

    // 2 serial allpass diffusers — smear the echo pattern into smooth tail
    for (u32 i = 0; i < 2; i++) {
        float delayed = ap_buf[i][ap_idx[i]];
        float w = comb_sum + delayed * 0.5f;
        ap_buf[i][ap_idx[i]] = w;
        ap_idx[i] = (ap_idx[i] + 1 >= AP_LEN[i]) ? 0 : ap_idx[i] + 1;
        comb_sum = delayed - 0.5f * w;
    }
    return comb_sum;
}

// ---------------------------------------------------------------------------
// Voice pool
// ---------------------------------------------------------------------------
#define MAX_SYNTH_VOICES 16
static SynthVoice syn_ch[MAX_SYNTH_VOICES];
static SynthVoice* SynAlloc(void) {
    for (u32 i = 0; i < MAX_SYNTH_VOICES; i++)
        if (!syn_ch[i].active) return &syn_ch[i];
    return NULL;
}

static float SynRandBi(void) { return (float)rand()/(float)RAND_MAX*2.0f-1.0f; }

static void synth_mix(SynthVoice* v, float* mix) {
    float vol = v->vol * (Sys_Settings.VolumeMaster/100.0f) * (Sys_Settings.VolumeEffects/100.0f);
    if (v->positional) {
        float dist = V3_Dist(v->pos, World.position[PLAYER1]);
        vol *= (dist >= 64.0f) ? 0.0f : (dist <= 1.0f) ? 1.0f : 1.0f-(dist-1.0f)/63.0f;
    }
    for (i32 f = 0; f < AUDIO_FRAMES; f++) {
        if (v->frame >= v->frames) { v->active = false; return; }
        float s = v->fn(v) * vol;
        mix[f*2+0] += s;
        mix[f*2+1] += s;
        v->frame++;
    }
}

void synth_reverb_apply(float* mix, i32 frames) {
    if (rev_wet < 0.001f) return;
    for (i32 f = 0; f < frames; f++) {
        float mono = (mix[f*2+0] + mix[f*2+1]) * 0.5f;
        float wet  = reverb_tick(mono);
        mix[f*2+0] = mix[f*2+0] * rev_dry + wet * rev_wet;
        mix[f*2+1] = mix[f*2+1] * rev_dry + wet * rev_wet;
    }
}

// ---------------------------------------------------------------------------
// DSP primitives
// ---------------------------------------------------------------------------
static float LP(float *s, float in, float rc)          { *s += rc*(in-*s); return *s; }
static float HP(float *s, float in, float rc)          { return in - LP(s,in,rc); }
static float BP(float *s1,float *s2,float in,float rc) { float a=LP(s1,in,rc); return a-LP(s2,a,rc); }
static float Phasor(float *ph, float freq)             { *ph += freq/AUDIO_RATE; if(*ph>=1.0f) *ph-=1.0f; return *ph; }
static float Osc(float *ph, float freq)                { return vsinf(6.28318f*Phasor(ph,freq)); }
static float FMOsc(float *c,float *m,float fc,float fm,float idx) { return vsinf(6.28318f*(Phasor(c,fc)+Osc(m,fm)*idx)); }

// p[0]=base_freq  p[1]=sweep_rate  p[2]=fm_rate_ratio  p[3]=decay
static float GenLaserSS1(SynthVoice* v) {
    float t   = (float)v->frame / AUDIO_RATE;
    float env = vexp(-v->p[3]*t);
    float fc  = v->p[0] * v->pitch * (1.0f + v->p[1]*t);
    float idx = 4.0f * vexp(-35.0f*t);
    float tone  = FMOsc(&v->s[0], &v->s[1], fc, v->p[2]*fc, idx);
    float click = (t < 0.012f) ? LP(&v->s[2], SynRandBi(), 0.4f)*(1.0f - t/0.012f)*0.4f : 0.0f;
    return (tone + click) * env;
}

// p[0]=pitch
static float GenDoor(SynthVoice* v) {
    float t   = (float)v->frame / AUDIO_RATE;
    float dur = (float)v->frames / AUDIO_RATE;
    float thud = vsinf(6.28318f * v->p[0]*v->pitch * t) * vexp(-8.0f*t) * 1.5f;
    float rc   = 0.08f + 0.05f*vsinf(6.28318f*3.0f*t);
    float hiss = BP(&v->s[0], &v->s[1], SynRandBi(), rc) * 0.6f;
    return thud + hiss * vsinf(3.14159265f*(t/dur));
}

// p[0]=ring_freq  p[1]=decay  p[2]=noise_amt  p[3]=ring_amt
static float GenImpact(SynthVoice* v) {
    float t   = (float)v->frame / AUDIO_RATE;
    float env = vexp(-v->p[1]*t);
    if (v->frame%4==0) v->s[2] = SynRandBi();
    float noise = LP(&v->s[0], v->s[2], 0.3f);
    float ring  = vsinf(6.28318f * v->p[0]*v->pitch * t) * env;
    return noise*env*v->p[2] + ring*v->p[3];
}

// p[0]=rumble_freq  p[1]=decay
static float GenBoom(SynthVoice* v) {
    float env = vexp(-v->p[1]*(float)v->frame/AUDIO_RATE);
    return Osc(&v->s[0], v->p[0]*v->pitch)*env*0.7f + LP(&v->s[1],SynRandBi(),0.15f)*env*0.8f;
}

static float GenHiss(SynthVoice* v)  { return HP(&v->s[0], SynRandBi(), v->p[0]); }
static float GenPipe(SynthVoice* v)  { return BP(&v->s[0], &v->s[1], SynRandBi(), v->p[0]) * 2.0f; }

// p[0]=freq  p[1]=decay
static float GenShieldHit(SynthVoice* v) {
    float t      = (float)v->frame / AUDIO_RATE;
    float wobble = vsinf(6.28318f*12.0f*t)*60.0f;
    return Osc(&v->s[0], v->p[0]*v->pitch + wobble) * vexp(-v->p[1]*t);
}

static float GenFootstep(SynthVoice* v) {
    float t   = (float)v->frame / AUDIO_RATE;
    float dur = (float)v->frames / AUDIO_RATE;
    float raw = SynRandBi();
    v->s[0] += 0.12f*(raw*raw*raw*0.35f - v->s[0]);
    v->s[1] += 0.12f*(v->s[0] - v->s[1]);
    if (v->s[5] <= 0.0f) {
        if ((rand()%100)<35) { v->s[5]=AUDIO_RATE*random_range(0.006f,0.018f); v->s[6]=random_range(0.4f,1.2f); }
        else v->s[6] = 0.0f;
    } else v->s[5] -= 1.0f;
    float rc = (0.22f - 0.08f*(t/dur)) * v->pitch;
    v->s[2] += rc*(raw*v->s[6] - v->s[2]);
    v->s[3] += rc*(v->s[2] - v->s[3]);
    float thump = (t < 0.07f) ? raw*vsinf(3.14159265f*(t/0.07f))*0.8f : 0.0f;
    v->s[4] += 0.025f*(thump - v->s[4]);
    return v->s[1] + (v->s[2]-v->s[3])*1.5f + v->s[4];
}

static float GenSandFootstep(SynthVoice* v) {
    float t   = (float)v->frame / AUDIO_RATE;
    float env = vexp(-25.0f*t);
    float noise = 0.0f;
    if (v->frame%12==0) {
        float raw = SynRandBi();
        noise = (raw>0.0f?1.0f:-1.0f)*(raw*raw);
        if (t<0.05f && (rand()%100)>85) noise += SynRandBi()*0.75f;
    }
    v->s[0] += 0.25f*(noise - v->s[0]);
    v->s[1] += 0.25f*(v->s[0] - v->s[1]);
    return (v->s[0]-v->s[1])*env*2.0f;
}

static float GenTapCase(SynthVoice* v) {
    float t   = (float)v->frame / AUDIO_RATE;
    float dur = (float)v->frames / AUDIO_RATE;
    if (v->frame%12==0) {
        float raw = SynRandBi();
        v->s[3] = (raw>0.0f?1.0f:-1.0f)*(raw*raw);
        if (t<0.06f && (rand()%100)>75) v->s[3] += SynRandBi()*0.75f;
    }
    if (v->frame%4==0) v->s[4] = SynRandBi();
    float rc = (0.35f-0.15f*(t/dur)) * v->pitch;
    v->s[0] += rc*(v->s[3]-v->s[0]); v->s[1] += rc*(v->s[0]-v->s[1]);
    v->s[2] += 0.04f*(v->s[4]-v->s[2]);
    return (v->s[0]-v->s[1])*vexp(-22.0f*t) + v->s[2]*vexp(-14.0f*t)*1.8f;
}

static float GenPlasticTap(SynthVoice* v) {
    float t  = (float)v->frame / AUDIO_RATE;
    float tr = SynRandBi() * vexp(-600.0f*t) * 0.5f;
    float f1 = 800.0f*v->pitch, f2 = 1100.0f*v->pitch;
    float bd = (vsinf(6.28318f*f1*t)*0.6f + vsinf(6.28318f*f2*t)*0.4f)*vexp(-45.0f*t)*0.4f;
    return tr + bd;
}

static float GenSparkSmall(SynthVoice* v) {
    float t   = (float)v->frame / AUDIO_RATE;
    float dur = (float)v->frames / AUDIO_RATE;
    float env = vexp(-14.0f*t);
    if (v->s[3]<=0.0f) {
        if ((rand()%100)<22) { v->s[3]=AUDIO_RATE*random_range(0.004f,0.012f); v->s[4]=1.0f; v->s[5]=0.93f-0.15f*(t/dur); }
        else v->s[4] = 0.0f;
    } else { v->s[3]-=1.0f; v->s[4]*=v->s[5]; }
    float active = (v->s[4]>0.01f) ? SynRandBi()*v->s[4] : 0.0f;
    v->s[1] += 0.28f*(active-v->s[1]); v->s[2] += 0.28f*(v->s[1]-v->s[2]);
    float thump = (t<0.08f) ? SynRandBi()*vsinf(3.14159265f*(t/0.08f)) : 0.0f;
    v->s[0] += 0.02f*(thump-v->s[0]);
    return ((v->s[1]-v->s[2])*3.5f + v->s[0]*1.2f)*env;
}

// p[0]=rc  p[1]=decay  p[2]=burst_chance
static float GenCrackle(SynthVoice* v) {
    float env = vexp(-v->p[1]*(float)v->frame/AUDIO_RATE);
    if (v->s[3]<=0.0f) {
        if ((i32)(rand()%100)<(i32)v->p[2]) { v->s[3]=AUDIO_RATE*random_range(0.004f,0.012f); v->s[4]=1.0f; v->s[5]=0.93f; }
        else v->s[4] = 0.0f;
    } else { v->s[3]-=1.0f; v->s[4]*=v->s[5]; }
    float active = (v->s[4]>0.01f) ? SynRandBi()*v->s[4] : 0.0f;
    v->s[1] += v->p[0]*(active-v->s[1]); v->s[2] += v->p[0]*(v->s[1]-v->s[2]);
    return (v->s[1]-v->s[2])*3.5f*env;
}

static float GenSine(SynthVoice* v) { return vsinf(6.28318f * v->p[0]*v->pitch * (float)v->frame/AUDIO_RATE); }

// p[0]=freq  p[1]=decay
static float GenClink(SynthVoice* v) {
    float t = (float)v->frame / AUDIO_RATE;
    return vsinf(6.28318f * v->p[0]*v->pitch * t) * vexp(-v->p[1]*t);
}

// Multi-partial ring for struck glass/metal objects.
// Models the inharmonic overtone series of a real resonant body.
// Each partial has its own amplitude weight and decay rate —
// high partials die fast (shimmer), fundamental sustains (body).
//
// p[0]=fundamental_freq  p[1]=base_decay  p[2]=partial_spread  p[3]=noise_attack
//
// Partial ratios below approximate a cylindrical glass vessel.
// s[0..3] = phasor state for 4 partials (no extra filter state needed)
static float GenRing(SynthVoice* v) {
    float t    = (float)v->frame / AUDIO_RATE;
    float f0   = v->p[0] * v->pitch;
    float dec  = v->p[1];
    float sprd = v->p[2]; // how much faster upper partials decay vs fundamental

    // Inharmonic partial ratios for glass (not integer multiples — real glass isn't harmonic).
    // Ratios approximate a cylindrical glass vessel from spectral analysis.
    static const float ratios[4] = { 1.0f, 2.76f, 5.41f, 8.93f };
    static const float amps[4]   = { 1.0f, 0.35f, 0.15f, 0.06f };

    float out = 0.0f;
    for (u32 i = 0; i < 4; i++) {
        float decay_i = dec * (1.0f + i * sprd);   // upper partials decay faster
        float env_i   = vexp(-decay_i * t);
        // s[0..3] hold phasor state for each partial; s[4] is LP filter for click
        out += Osc(&v->s[i], f0 * ratios[i]) * amps[i] * env_i;
    }

    // Short noise burst at attack — the physical contact transient
    float click = (t < v->p[3]) ? LP(&v->s[4], SynRandBi(), 0.35f) * (1.0f - t/v->p[3]) * 0.3f : 0.0f;
    return out + click;
}

// Glass-on-plastic floor thud.
// Contact transient (noise burst through BP for the floor body) +
// damped glass ring (GenRing logic inline, shortened decay from contact damping).
// p[0]=glass_freq  p[1]=ring_decay  p[2]=floor_resonance_rc  p[3]=noise_attack
static float GenBeakerThud(SynthVoice* v) {
    float t  = (float)v->frame / AUDIO_RATE;
    float f0 = v->p[0] * v->pitch;

    // Glass partials — same inharmonic ratios, but faster decay (contact damped)
    static const float ratios[4] = { 1.0f, 2.76f, 5.41f, 8.93f };
    static const float amps[4]   = { 0.7f, 0.25f, 0.08f, 0.03f };
    float glass = 0.0f;
    for (u32 i = 0; i < 4; i++) {
        float decay_i = v->p[1] * (1.0f + i * 1.8f);
        glass += Osc((float*)&v->s[i], f0 * ratios[i]) * amps[i] * vexp(-decay_i * t);
    }

    // Floor body: noise burst filtered through BP at plastic panel resonance
    // The floor resonates at ~200-400Hz for thin plastic panels
    if (v->frame%4==0) v->s[4] = SynRandBi();
    float floor_body = BP(&v->s[5], &v->s[6], v->s[4], v->p[2]) * vexp(-30.0f*t) * 1.4f;

    // Contact click
    float click = (t < v->p[3]) ? LP(&v->s[7], SynRandBi(), 0.4f)*(1.0f - t/v->p[3])*0.5f : 0.0f;

    return glass + floor_body + click;
}

// ---------------------------------------------------------------------------
// Preset table
// ---------------------------------------------------------------------------
typedef struct { SynthFn fn; float dur, vol; float p[4]; } SynthPreset;

static const SynthPreset SynthPresets[SND_COUNT] = {
    //                                                  dur    vol     p0        p1      p2     p3
    // freq     sweep  fm_ratio  decay
    [SND_LASER_PISTOL]  = { GenLaserSS1,              0.30f, 0.55f, { 900.0f,  -1.8f,  0.5f, 10.0f } },
    [SND_LASER_RIFLE]   = { GenLaserSS1,              0.40f, 0.70f, { 600.0f,  -0.8f,  0.4f,  7.0f } },
    // pitch
    [SND_DOOR]          = { GenDoor,                  1.20f, 0.65f, { 50.0f,    0,      0,     0    } },
    // ring_freq  decay  noise  ring
    [SND_IMPACT_GLASS]  = { GenImpact,                0.50f, 0.60f, { 4500.0f, 18.0f,  0.3f,  0.6f } },
    [SND_IMPACT_METAL]  = { GenImpact,                0.50f, 0.65f, { 1800.0f, 30.0f,  0.5f,  0.3f } },
    // rumble_freq  decay
    [SND_EXPLOSION]     = { GenBoom,                  1.00f, 0.80f, { 55.0f,    3.5f,   0,     0    } },
    [SND_HISS]          = { GenHiss,                  1.50f, 0.35f, { 0.12f,    0,      0,     0    } },
    [SND_PIPE]          = { GenPipe,                  1.50f, 0.40f, { 0.07f,    0,      0,     0    } },
    // freq  decay
    [SND_SHIELD_HIT]    = { GenShieldHit,             0.35f, 0.50f, { 180.0f,   8.0f,   0,     0    } },
    [SND_FOOTSTEP]      = { GenFootstep,              0.26f, 0.70f, { 0 } },
    [SND_SAND_FOOTSTEP] = { GenSandFootstep,          0.30f, 0.65f, { 0 } },
    [SND_TAP_CASE]      = { GenTapCase,               0.25f, 0.55f, { 0 } },
    [SND_PLASTIC_TAP]   = { GenPlasticTap,            0.09f, 0.50f, { 0 } },
    [SND_SPARK_SMALL]   = { GenSparkSmall,            0.24f, 0.50f, { 0 } },
    // rc     decay  burst_chance
    [SND_CRACKLE]       = { GenCrackle,               2.00f, 0.45f, { 0.28f,   2.5f,  20.0f,  0    } },
    [SND_SINE]          = { GenSine,                  1.00f, 0.50f, { 440.0f,   0,      0,     0    } },
    // freq   decay
    [SND_CLINK]         = { GenClink,                 0.40f, 0.50f, { 1200.0f, 12.0f,   0,     0    } },

    // Glass beaker tapped against another beaker.
    // f0=2400Hz (medium beaker), base_decay=4.0, partial_spread=1.4, click_dur=0.008s
    // Tune f0 down for larger/fuller beaker, up for smaller. Spread controls shimmer duration.
    [SND_BEAKER_CLINK]  = { GenRing,                  1.20f, 0.50f, { 2400.0f,  4.0f,   1.4f,  0.008f } },

    // Glass beaker set down on plastic floor panel.
    // f0=2200Hz (glass ring, contact-damped), ring_decay=18.0 (fast, muffled by contact),
    // floor_rc=0.06 (BP centered ~300Hz plastic panel thump), click_dur=0.012s
    [SND_BEAKER_THUD]   = { GenBeakerThud,            0.60f, 0.55f, { 2200.0f, 18.0f,   0.06f, 0.012f } },
};

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
void play_synth(SoundID id, float vol, float pitch) {
    if ((u32)id >= SND_COUNT) return;
    const SynthPreset* pr = &SynthPresets[id];
    SynthVoice* v = SynAlloc();
    if (!v) return;
    *v = (SynthVoice){ .fn=pr->fn, .frames=(u32)(AUDIO_RATE*pr->dur), .vol=pr->vol*vol, .pitch=pitch, .active=true };
    v->p[0]=pr->p[0]; v->p[1]=pr->p[1]; v->p[2]=pr->p[2]; v->p[3]=pr->p[3];
}

void play_synth_at(SoundID id, float vol, float pitch, V3 pos) {
    if ((u32)id >= SND_COUNT) return;
    const SynthPreset* pr = &SynthPresets[id];
    SynthVoice* v = SynAlloc();
    if (!v) return;
    *v = (SynthVoice){ .fn=pr->fn, .frames=(u32)(AUDIO_RATE*pr->dur), .vol=pr->vol*vol, .pitch=pitch, .positional=true, .pos=pos, .active=true };
    v->p[0]=pr->p[0]; v->p[1]=pr->p[1]; v->p[2]=pr->p[2]; v->p[3]=pr->p[3];
}

// ---------------------------------------------------------------------------
// Area load examples:
//   synth_set_room(0.0f,  0.00f);  // anechoic (outdoor, dead room)
//   synth_set_room(0.25f, 0.20f);  // small corridor / panel room
//   synth_set_room(0.45f, 0.30f);  // medium room / storage bay
//   synth_set_room(0.70f, 0.45f);  // large open area / hangar
//   synth_set_room(0.88f, 0.55f);  // cave / underground
//
// Audio callback placement — insert synth_reverb_apply AFTER wavs+synth, BEFORE logs+mp3:
//
//   for (wav)   wave_mix(...)
//   for (ext)   wave_mix(...)
//   for (synth) synth_mix(...)
//   synth_reverb_apply(mix, AUDIO_FRAMES);   // <— here
//   // log playback
//   // mp3 playback
//   // clamp + convert to i16
