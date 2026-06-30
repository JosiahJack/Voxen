// synth.c - Audio Synthesis Engine
#define MAX_SYNTH_VOICES 16
typedef struct SynthVoice SynthVoice; typedef float (*SynthFn)(SynthVoice*); struct SynthVoice { SynthFn fn; u32 frame,frames; float vol; V3 pos; bool positional,active; float p[4],s[4]; };
static SynthVoice syn_ch[MAX_SYNTH_VOICES];
static SynthVoice* SynAlloc(void) { for (u32 i=0;i<MAX_SYNTH_VOICES;i++) if (!syn_ch[i].active) return &syn_ch[i]; return NULL; }
static SynthVoice* SynTrigger(SynthFn fn,float seconds,float vol) { SynthVoice* v=SynAlloc(); if(!v) return NULL; *v=(SynthVoice){.fn=fn,.frames=(u32)(AUDIO_RATE*seconds),.vol=vol,.active=true}; return v; }
void SynStop(SynthVoice* v) { v->active=false; } // for frames==0 (indefinite) voices later
static float SynRandBi(void) { return (float)rand()/(float)RAND_MAX*2.0f-1.0f; }
static void synth_mix(SynthVoice* v, float* mix) {
    float vol = v->vol*(Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeEffects/100.0f);
    if (v->positional) { float dist=V3_Dist(v->pos,World.instances[PLAYER1].position); vol *= (dist>=64.0f)?0.0f:((dist<=1.0f)?1.0f:1.0f-(dist-1.0f)/63.0f); }
    for (i32 f=0;f<AUDIO_FRAMES;f++) {
        if (v->frames && v->frame>=v->frames) { v->active=false; return; }
        float s = v->fn(v)*vol;
        mix[f*2+0]+=s; mix[f*2+1]+=s;
        v->frame++;
    }
}

static float LP(float *s, float in, float rc) { *s += rc*(in-*s); return *s; }
static float HP(float *s, float in, float rc) { return in-LP(s,in,rc); }
static float BP(float *s1,float *s2,float in,float rc) { float a=LP(s1,in,rc); float b=LP(s2,a,rc); return a-b; }
static float Phasor(float *ph, float freq) { *ph += freq/AUDIO_RATE; if (*ph>=1.0f) *ph-=1.0f; return *ph; }
static float Osc(float *ph, float freq) { return vsinf(6.28318f*Phasor(ph,freq)); }
static float FMOsc(float *ph_c,float *ph_m,float freq_c,float freq_m,float idx) { float m=Osc(ph_m,freq_m); return Osc(ph_c,freq_c+m*idx); }
static float GenLaser(SynthVoice* v) { float t=(float)v->frame/AUDIO_RATE, env=vexp(-v->p[3]*t); float fc=v->p[0]*(1.0f+v->p[1]*t); float fm=FMOsc(&v->s[0],&v->s[1],fc,v->p[2],fc*0.8f*env); return fm*env; }
void play_synth_laser(float volume,float freq,float sweep,float fmrate,float decay) { SynthVoice* v=SynTrigger(GenLaser,0.3f,volume); if (v) { v->p[0]=freq; v->p[1]=sweep; v->p[2]=fmrate; v->p[3]=decay; } }
static float GenDoor(SynthVoice* v) { float t=(float)v->frame/AUDIO_RATE, dur=(float)v->frames/AUDIO_RATE; float thud=vsinf(6.28318f*v->p[0]*t)*vexp(-8.0f*t)*1.5f; float rc=0.08f+0.05f*vsinf(6.28318f*3.0f*t); float hiss=BP(&v->s[0],&v->s[1],SynRandBi(),rc)*0.6f; return thud+hiss*vsinf(3.14159265f*(t/dur)); }
void play_synth_door(float volume,float pitch) { SynthVoice* v=SynTrigger(GenDoor,1.2f,volume); if(v) v->p[0]=pitch; }
static float GenHiss(SynthVoice* v) { return HP(&v->s[0],SynRandBi(),v->p[0])*v->p[1]; }
SynthVoice* play_synth_hiss(float volume,float cutoff) { SynthVoice* v=SynTrigger(GenHiss,0.0f,volume); if(v){v->p[0]=cutoff;v->p[1]=1.0f;} return v; }
static float GenPipe(SynthVoice* v) { return BP(&v->s[0],&v->s[1],SynRandBi(),v->p[0])*v->p[1]; }
SynthVoice* play_synth_pipe(float volume,float resonance_rc) { SynthVoice* v=SynTrigger(GenPipe,0.0f,volume); if(v){v->p[0]=resonance_rc;v->p[1]=2.0f;} return v; }
static float GenImpact(SynthVoice* v) { float t=(float)v->frame/AUDIO_RATE, env=vexp(-v->p[1]*t); if (v->frame%4==0) {v->s[2]=SynRandBi();} float noise=LP(&v->s[0],v->s[2],0.3f), ring=vsinf(6.28318f*v->p[0]*t)*env; return (noise*env*v->p[2])+(ring*v->p[3]); }
void play_synth_impact(float volume,float ring_freq,float decay,float noise_amt,float ring_amt) { SynthVoice* v=SynTrigger(GenImpact,0.5f,volume); if (v) { v->p[0]=ring_freq; v->p[1]=decay; v->p[2]=noise_amt; v->p[3]=ring_amt; } }
static float GenCrackle(SynthVoice* v) {
    float env=vexp(-v->p[1]*(float)v->frame/AUDIO_RATE);
    if (v->s[3]<=0.0f) { if ((i32)(rand()%100)<(i32)v->p[2]) { v->s[3]=AUDIO_RATE*random_range(0.004f,0.012f); v->s[4]=1.0f; v->s[5]=0.93f; } else v->s[4]=0.0f; }
    else { v->s[3]-=1.0f; v->s[4]*=v->s[5]; }
    float active=(v->s[4]>0.01f)?SynRandBi()*v->s[4]:0.0f;
    v->s[1]+=v->p[0]*(active-v->s[1]); v->s[2]+=v->p[0]*(v->s[1]-v->s[2]);
    return (v->s[1]-v->s[2])*3.5f*env;
}
void play_synth_crackle(float volume,float rc,float decay,float burst_chance,float seconds) { SynthVoice* v=SynTrigger(GenCrackle,seconds,volume); if (v) { v->p[0]=rc; v->p[1]=decay; v->p[2]=burst_chance; } }
static float GenBoom(SynthVoice* v) { float env=vexp(-v->p[1]*(float)v->frame/AUDIO_RATE); return Osc(&v->s[0],v->p[0])*env*0.7f + LP(&v->s[1],SynRandBi(),0.15f)*env*0.8f; }
void play_synth_explosion(float volume,float rumble_freq,float decay) { SynthVoice* v=SynTrigger(GenBoom,1.0f,volume); if(v){v->p[0]=rumble_freq;v->p[1]=decay;} }
static float GenShield(SynthVoice* v) { float wobble=vsinf(6.28318f*6.0f*(float)v->frame/AUDIO_RATE)*30.0f; return Osc(&v->s[0],v->p[0]+wobble)*LP(&v->s[1],v->p[1],0.002f); }
SynthVoice* play_synth_shield(float volume,float freq) { SynthVoice* v=SynTrigger(GenShield,0.0f,volume); if(v){v->p[0]=freq;v->p[1]=1.0f;} return v; }
void synth_shield_set(SynthVoice* v,float target) { v->p[1]=target; } // 1=power up, 0=deplete; LP smooths the ramp

// simple one pole Low Pass Filter (Very efficient for C)
// This makes things sound like they are behind a door or inside a pipe.
float apply_lpf(float input, float* prev_out, float cutoff) {
    // Simple alpha calculation based on sample rate
    float alpha = cutoff / (cutoff + 1000.0f); 
    *prev_out = *prev_out + alpha * (input - *prev_out);
    return *prev_out;
}

// FM Synthesis: A "Gritty" Sine wave
// Instead of a pure sine, this creates the metallic 'zing' of lasers/electronics.
float GenFM(SynthVoice* v) { float t = (float)v->frame / AUDIO_RATE; float mod = vsinf(2.0f * PI * 1.0f * t) * v->p[1]; return vsinf(2.0f * PI * v->p[0] * t + mod); }
float GetNoise(SynthVoice* v) { float raw = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f; v->s[3] = raw; return raw; }
float get_env(SynthVoice* v, float attack_time) { float t = (float)v->frame / AUDIO_RATE; if (t < attack_time) return t / attack_time; return 1.0f; }
static float GenVent(SynthVoice* v) { float raw = GetNoise(v); float filtered = apply_lpf(raw, &v->s[2], v->p[2]); return filtered; }
static float GenSpark(SynthVoice* v) { float raw = GetNoise(v); if (rand() % 10 > 8) {raw *= 2.0f;} return raw * 0.5f; }
void play_synth_vent(float vol) { SynthVoice* v = SynTrigger(GenVent, 2.0f, vol); if(v) { v->p[2] = 800.0f; } }
void play_synth_spark(float vol) { SynTrigger(GenSpark, 0.1f, vol); }
static float GenPlasticTap(SynthVoice* v) { float t=(float)v->frame/AUDIO_RATE; float tr=SynRandBi()*vexp(-600.0f*t)*0.5f; float bd=(vsinf(6.28318f*800.0f*t)*0.6f+vsinf(6.28318f*1100.0f*t)*0.4f)*vexp(-45.0f*t)*0.4f; return tr+bd; }
void play_synth_plastic_tap(float volume) { SynTrigger(GenPlasticTap,0.09f,volume); }
static float GenSandFootstep(SynthVoice* v) {
    float t=(float)v->frame/AUDIO_RATE, env=vexp(-25.0f*t), noise=0.0f;
    if (v->frame%12==0) { float raw=SynRandBi(); noise=(raw>0.0f?1.0f:-1.0f)*(raw*raw); if (t<0.05f && (rand()%100)>85) noise+=SynRandBi()*0.75f; }
    v->s[0]+=0.25f*(noise-v->s[0]); v->s[1]+=0.25f*(v->s[0]-v->s[1]);
    return (v->s[0]-v->s[1])*env*2.0f;
}

void play_synth_sand_footstep(float volume) { SynTrigger(GenSandFootstep,0.3f,volume); }
static float GenTapCase(SynthVoice* v) {
    float t=(float)v->frame/AUDIO_RATE, dur=(float)v->frames/AUDIO_RATE;
    float pebble_env=vexp(-22.0f*t), thud_env=vexp(-14.0f*t);
    if (v->frame%12==0) { float raw=SynRandBi(); v->s[3]=(raw>0.0f?1.0f:-1.0f)*(raw*raw); if (t<0.06f && (rand()%100)>75) v->s[3]+=SynRandBi()*0.75f; }
    if (v->frame%4==0) v->s[4]=SynRandBi();
    float rc=0.35f-0.15f*(t/dur);
    v->s[0]+=rc*(v->s[3]-v->s[0]); v->s[1]+=rc*(v->s[0]-v->s[1]);
    v->s[2]+=0.04f*(v->s[4]-v->s[2]);
    return (v->s[0]-v->s[1])*pebble_env + v->s[2]*thud_env*1.8f;
}

void play_synth_tap_case(float volume) { SynTrigger(GenTapCase,0.25f,volume); }
static float GenSparkSmall(SynthVoice* v) {
    float t=(float)v->frame/AUDIO_RATE, dur=(float)v->frames/AUDIO_RATE, env=vexp(-14.0f*t);
    if (v->s[3]<=0.0f) {
        if ((rand()%100)<22) { v->s[3]=AUDIO_RATE*random_range(0.004f,0.012f); v->s[4]=1.0f; v->s[5]=0.93f-0.15f*(t/dur); }
        else v->s[4]=0.0f;
    } else { v->s[3]-=1.0f; v->s[4]*=v->s[5]; }
    float active=(v->s[4]>0.01f)?SynRandBi()*v->s[4]:0.0f;
    v->s[1]+=0.28f*(active-v->s[1]); v->s[2]+=0.28f*(v->s[1]-v->s[2]);
    float thump_in=0.0f; if (t<0.08f) thump_in=SynRandBi()*vsinf(3.14159265f*(t/0.08f));
    v->s[0]+=0.02f*(thump_in-v->s[0]);
    return ((v->s[1]-v->s[2])*3.5f + v->s[0]*1.2f)*env;
}

void play_synth_electrical_spark_small(float volume) { SynTrigger(GenSparkSmall,0.24f,volume); }
static float GenFootstep(SynthVoice* v) {
    float t=(float)v->frame/AUDIO_RATE, dur=(float)v->frames/AUDIO_RATE, raw=SynRandBi();
    v->s[0]+=0.12f*(raw*raw*raw*0.35f-v->s[0]); v->s[1]+=0.12f*(v->s[0]-v->s[1]);
    if (v->s[5]<=0.0f) { if ((rand()%100)<35) { v->s[5]=AUDIO_RATE*random_range(0.006f,0.018f); v->s[6]=random_range(0.4f,1.2f); } else v->s[6]=0.0f; }
    else v->s[5]-=1.0f;
    float rc=0.22f-0.08f*(t/dur);
    v->s[2]+=rc*(raw*v->s[6]-v->s[2]); v->s[3]+=rc*(v->s[2]-v->s[3]);
    float thump_in=0.0f; if (t<0.07f) thump_in=raw*vsinf(3.14159265f*(t/0.07f))*0.8f;
    v->s[4]+=0.025f*(thump_in-v->s[4]);
    return v->s[1]+(v->s[2]-v->s[3])*1.5f+v->s[4];
}

void play_synth_footstep(float volume) { SynTrigger(GenFootstep,0.26f,volume); }
static float GenSine(SynthVoice* v) { return vsinf(6.28318f*v->p[0]*(float)v->frame/AUDIO_RATE); }
void play_synth_sine(float frequency,float duration_seconds,float volume) { SynthVoice* v=SynTrigger(GenSine,duration_seconds,volume); if (v) v->p[0]=frequency; }
static float GenClink(SynthVoice* v) { float t=(float)v->frame/AUDIO_RATE; return vsinf(6.28318f*v->p[0]*t)*vexp(-v->p[1]*t); }
void play_synth_clink(float freq,float decay,float volume) { SynthVoice* v=SynTrigger(GenClink,0.4f,volume); if (v) { v->p[0]=freq; v->p[1]=decay; } }

static float GenSearchPing(SynthVoice* v) {
    float t=(float)v->frame/AUDIO_RATE, dur=(float)v->frames/AUDIO_RATE;
    float env=vsinf(3.14159265f*(t/dur));
    if (v->s[1]<=0.0f) { v->s[0]=random_range(v->p[0],v->p[1]); v->s[1]=AUDIO_RATE*random_range(0.02f,0.09f); } else v->s[1]-=1.0f;
    v->s[2]+=v->s[0]/AUDIO_RATE; if (v->s[2]>=1.0f) v->s[2]-=1.0f;
    float tone=vsinf(6.28318f*v->s[2]);
    if (v->s[4]<=0.0f) { v->s[3]=random_range(0.35f,1.0f); v->s[4]=AUDIO_RATE*random_range(0.006f,0.02f); } else v->s[4]-=1.0f;
    return tone*v->s[3]*env;
}
void play_synth_search(float volume,float freq_lo,float freq_hi) {
    SynthVoice* v=SynTrigger(GenSearchPing,0.73f,volume); if (v) { v->p[0]=freq_lo; v->p[1]=freq_hi; }
}
