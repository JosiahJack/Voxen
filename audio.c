// audio.c - Audio System
#define AUDIO_RATE      48000
#define AUDIO_CHANNELS  2
#define AUDIO_PERIOD_MS 10
#define AUDIO_PERIODS   4
#define AUDIO_FRAMES    ((AUDIO_RATE * AUDIO_PERIOD_MS) / 1000)
#define AUDBUF_SIZE (AUDIO_FRAMES*AUDIO_PERIODS)
void* mcpy(void *dst, const void *src, size_t n);
extern SettingsSystem Sys_Settings; extern GlobalContext World;
#include "audimport.h" // Audio Import System for .mp3 and .wav file formats
typedef struct { mp3 dec; bool open; float fade_vol,fade_target,fade_step; u32 src_rate; u64 frames_decoded,total_frames; } mp3_channel_t;
static wav_channel_t wav_ch[MAX_CHANNELS],*ext_ch[MAX_CHANNELS]; static u32 wav_count,ext_count,mp3_slot,log_frame_count,log_frame_pos; 
static mp3_channel_t mp3_ch[2]; static float *log_samples; static size_t log_allocSize=0; static bool log_playing,mp3_paused = false;
static float *resample_stereo(float *src, size_t srcSize, u32 *frames, u32 src_rate, size_t* sz) {
    if (src_rate == AUDIO_RATE) return src;
    u32 sf = *frames, df = (u32)((u64)sf*AUDIO_RATE/src_rate); float *dst = (float*)OS_Alloc(df*2*sizeof(float)); *sz = df*2*sizeof(float); float ratio = (float)sf/(float)df;
    for (u32 i = 0; i < df; i++) { float pos = i*ratio; u32 a = (u32)pos, b = a+1<sf?a+1:a; float t = pos-(float)a; dst[i*2+0] = src[a*2+0]+t*(src[b*2+0]-src[a*2+0]); dst[i*2+1] = src[a*2+1]+t*(src[b*2+1]-src[a*2+1]); }
    OS_Free(src,srcSize); *frames = df; return dst;
}

static void WavUnInit(WaveFile *w) { if (w->fp != INVALID_FHANDLE) { OS_Close(w->fp); w->fp = INVALID_FHANDLE; } }
static float *load_wav(const char *path,u32 *out_frames, size_t* sz) {
    WaveFile wav; if (!WavInit(&wav,path)) {return NULL;} if (wav.channels > 2) { WavUnInit(&wav); return NULL; }
    u64 frames = wav.totalPCMFrameCount; float *buf = (float*)OS_Alloc(frames*AUDIO_CHANNELS*sizeof(float)); size_t bufSize = frames*AUDIO_CHANNELS*sizeof(float); u64 got = WavReadPCMFrames(&wav,frames,buf);
    if (wav.channels == 1) for (i64 i=(i64)got-1;i>=0;i--) { buf[i*2+1]=buf[i]; buf[i*2]=buf[i]; }
    u32 src_rate = wav.sampleRate; WavUnInit(&wav); *out_frames = (u32)got; return resample_stereo(buf,bufSize,out_frames,src_rate,sz); // Reallocates and returns new buffer, freeing the buf alloc'ed here
}

i32 GetFreeWavSlot() { i32 retval = -1; for (u32 i = 0; i < wav_count; i++) { if (!wav_ch[i].playing && wav_ch[i].samples) {OS_Free(wav_ch[i].samples,wav_ch[i].allocSize); wav_ch[i].samples = NULL; wav_ch[i].allocSize = 0; retval=i; } } return retval; }
#include "synth.c" // Audio Synthesis Engine
static void wave_mix(wav_channel_t* w, float* mix) {
    float vol = w->volume * (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeEffects/100.0f); V3 pos = w->pos; float dist = V3_Dist(pos,World.instances[PLAYER1].position); float spatial_atten = (dist >= 64.0f) ? 0.0f : ((dist <= 1.0f) ? 1.0f : 1.0f-(dist-1.0f)/63.0f);
    if (w->positional) vol *= spatial_atten;
    for (i32 f = 0; f < AUDIO_FRAMES; f++) { if (w->frame_pos >= w->frame_count){ if (w->looping){w->frame_pos=0;}else{w->playing=false; break;} } mix[f*2+0] += w->samples[w->frame_pos*2+0]*vol; mix[f*2+1] += w->samples[w->frame_pos*2+1]*vol; w->frame_pos++; }
}

static void audio_mix_period(i16 *out) {
    float mix[AUDIO_FRAMES*AUDIO_CHANNELS]; mset(mix,0,sizeof(mix));
    for (u32 c=0;c<wav_count;c++) { if ( wav_ch[c].playing &&  wav_ch[c].samples) {wave_mix(&wav_ch[c],mix);} } // General wav file playback
    for (u32 c=0;c<ext_count;c++) { if (ext_ch[c]->playing && ext_ch[c]->samples) {wave_mix(ext_ch[c], mix);} } // Looped Ambients
    for (u32 c=0;c<MAX_SYNTH_VOICES;c++) if (syn_ch[c].active) synth_mix(&syn_ch[c],mix); // Synthesized audio (oh yes!)
    if (log_playing && log_samples) {
        float vol = (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeMessage/100.0f);
        for (i32 f = 0; f < AUDIO_FRAMES; f++) {
            if (log_frame_pos >= log_frame_count) { log_playing=false; break; }
            mix[f*2+0] += log_samples[log_frame_pos*2+0] * vol; mix[f*2+1] += log_samples[log_frame_pos*2+1] * vol; log_frame_pos++;
        }
    }
    if (!mp3_paused) {
        for (u32 s = 0; s < 2; s++) {
            mp3_channel_t *m = &mp3_ch[s];
            if (!m->open) continue;
            u32 src_rate = m->src_rate ? m->src_rate : AUDIO_RATE;
            u64 frames_to_read = (src_rate == AUDIO_RATE) ? AUDIO_FRAMES : (u64)((u64)AUDIO_FRAMES*src_rate/AUDIO_RATE)+2;
            float raw[AUDIO_FRAMES*4];
            u64 got = mp3_read_pcm_frames_f32(&m->dec,frames_to_read,raw);
            if (got == 0) { mp3_uninit(&m->dec); m->open=false; continue; }
            float vol = m->fade_vol * (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeMusic/100.0f);
            m->frames_decoded += got; float ratio = (float)got/(float)AUDIO_FRAMES;
            for (i32 f = 0; f < AUDIO_FRAMES; f++) {
                float pos = f*ratio; u32 a=(u32)pos, b=(a+1<(u32)got)?a+1:a; float t=pos-(float)a;
                float l = raw[a*2+0]+t*(raw[b*2+0]-raw[a*2+0]), r = raw[a*2+1]+t*(raw[b*2+1]-raw[a*2+1]);
                mix[f*2+0] += l*vol; mix[f*2+1] += r*vol;
                if (m->fade_step != 0.0f) {
                    m->fade_vol += m->fade_step;
                    if (m->fade_step>0.0f && m->fade_vol>=m->fade_target) { m->fade_vol=m->fade_target; m->fade_step=0.0f; }
                    else if (m->fade_step<0.0f && m->fade_vol<=m->fade_target) { m->fade_vol=m->fade_target; m->fade_step=0.0f; if (m->fade_target==0.0f) { mp3_uninit(&m->dec); m->open=false; } }
                }
            }
        }
    }
    for (u32 i = 0; i < AUDIO_FRAMES * AUDIO_CHANNELS; i++) { float s = mix[i]; s = s > 1.0f ? 1.0f : (s < -1.0f ? -1.0f : s); out[i] = (i16)(s * 32767.0f); }
}

ENGINE_TO_MOD void play_wav(const char *path,float volume,V3 pos,bool positional) {
    if (sEqual(path,"./Audio/misc/null.wav")) return;
    i32 slot = GetFreeWavSlot();
    if (slot==-1 && wav_count<MAX_CHANNELS) slot=wav_count++;
    if (slot==-1) { DualLog("WARNING: Max WAV channels (%d) reached\n",MAX_CHANNELS); return; }
    u32 frames; size_t sz=0; float *buf = load_wav(path,&frames,&sz);
    if (!buf) { DualLog("ERROR: Failed to load WAV %s\n",path); return; }
    wav_ch[slot] = (wav_channel_t){ .samples = buf, .allocSize = sz, .frame_count = frames, .frame_pos = 0, .volume = volume, .looping = false, .positional = positional, .pos=pos, .playing = true };
}

ENGINE_TO_MOD void play_message(const char *path) {
    if (log_playing && log_samples && log_allocSize > 0) { log_playing=false; OS_Free(log_samples,log_allocSize); log_samples=NULL; log_allocSize=0; }
    u32 frames; float *buf = load_wav(path,&frames,&log_allocSize); if (!buf) { DualLogError("Failed to load %s\n",path); return; }
    log_samples=buf; log_frame_count=frames; log_frame_pos=0; log_playing=true;
}

ENGINE_TO_MOD i32 SndInit(const char *path, wav_channel_t *w) { u32 frames; size_t sz=0; float *buf=load_wav(path,&frames,&sz); if(!buf){return -1;} w->samples=buf; w->allocSize=sz; w->frame_count=frames; w->frame_pos=0; w->volume=1.0f; w->looping=w->positional=w->playing=false; return 0; }
ENGINE_TO_MOD i32 SndStart(wav_channel_t* w) { w->frame_pos = 0; w->playing = true; for (u32 i=0;i<ext_count;++i) if (ext_ch[i] == w) return 0; if (ext_count < MAX_CHANNELS) ext_ch[ext_count++] = w; return 0; }
ENGINE_TO_MOD void SndStop(wav_channel_t* w) { w->playing=false; }
ENGINE_TO_MOD void SndUninit(wav_channel_t* w) { if (w->samples) { OS_Free(w->samples,w->allocSize); w->samples = NULL; w->allocSize = 0; } w->playing = false; for (u32 i=0;i<ext_count;++i) if (ext_ch[i] == w) { ext_ch[i] = ext_ch[--ext_count]; break; } }
ENGINE_TO_MOD void SndSetVolume(wav_channel_t* w, float volume) { w->volume=volume; }
ENGINE_TO_MOD void SoundSetLooping(wav_channel_t* w, bool loop) { w->looping=loop; }
ENGINE_TO_MOD bool SndPlaying(wav_channel_t* w)  { return w->playing; }
ENGINE_TO_MOD i32 SndFrmCurpos(wav_channel_t* w,u64 *pCursor) { *pCursor=w->frame_pos; return 0; }
ENGINE_TO_MOD float SndLen(wav_channel_t* w) {  return (float)w->frame_count / (float)AUDIO_RATE; }
static void mp3_open_slot(i32 s, const char *path, float fade_from, float fade_to, i32 fade_ms) {
    mp3_channel_t *m = &mp3_ch[s]; if (m->open) { mp3_uninit(&m->dec); m->open=false; } if (!mp3_init_file(&m->dec,path)) { DualLog("ERROR: Failed to load MP3 %s\n",path); return; }
    m->src_rate = m->dec.sampleRate; m->total_frames = mp3_get_pcm_frame_count(&m->dec); mp3_seek_to_pcm_frame(&m->dec,0); m->frames_decoded = 0; m->open = true; m->fade_target = fade_to;
    m->fade_vol = (m->fade_step = (fade_ms > 0) ? (fade_to - fade_from) / (AUDIO_RATE * fade_ms / 1000.0f) : 0.0f) == 0.0f ? fade_to : fade_from;
}

void play_mp3(const char *path, i32 fade_ms) { i32 old = mp3_slot, next = mp3_slot ? 0 : 1; if (mp3_ch[old].open) { mp3_ch[old].fade_target = 0.0f; mp3_ch[old].fade_step = (fade_ms > 0) ? -mp3_ch[old].fade_vol / (AUDIO_RATE * fade_ms / 1000.0f) : -1.0f; } mp3_open_slot(mp3_slot = next,path,0.0f,1.0f,fade_ms); }
void mp3_clear() { for (i32 i=0;i<2;i++) if (mp3_ch[i].open) { mp3_uninit(&mp3_ch[i].dec); mp3_ch[i].open=false; } mp3_slot=0; }
ENGINE_TO_MOD void MP3Pause() { mp3_paused = true; }
ENGINE_TO_MOD void MP3Resume() { mp3_paused = false; }
ENGINE_TO_MOD float GetMP3RemainingTime() { mp3_channel_t *m = &mp3_ch[mp3_slot]; return (!m->open || m->frames_decoded >= m->total_frames) ? 0.0f : (!m->total_frames ? 1.0f : (float)(m->total_frames - m->frames_decoded) / (m->src_rate ? m->src_rate : AUDIO_RATE)); }
static FHandle pcm_fds[8]; static i32 pcm_fd_count = 0;
pthread_t audThreadID; void* AudThread(void* arg); 
#ifdef WINDOWS
    void AudioUpdate() {
        if (pcm_fd_count==0) {return;} 
        i16 buf[AUDIO_FRAMES*AUDIO_CHANNELS]; pcm_sync_t sync; if (pcm_sync(pcm_fds[0],&sync) < 0) {return;}
        u32 avail = AUDBUF_SIZE - ((sync.control.appl_ptr - sync.status.hw_ptr > AUDBUF_SIZE) ? 0 : sync.control.appl_ptr - sync.status.hw_ptr);
        while (avail>=(u32)AUDIO_FRAMES) { audio_mix_period(buf); for (i32 i=0;i<pcm_fd_count;i++) { if (pcm_write(buf,AUDIO_FRAMES)<0) {pcm_prepare(pcm_fds[i]);} } avail-=AUDIO_FRAMES; }
    }
    
    void InitAudio() { FHandle first = pcm_open_all(AUDIO_RATE,AUDIO_CHANNELS,AUDIO_FRAMES,AUDIO_PERIODS); if (first == INVALID_FHANDLE) { DualLog("ERROR: No WASAPI audio device found\n"); return; } pcm_fds[0] = first; pcm_fd_count = 1; pthread_create(&audThreadID,NULL,AudThread,NULL); }
#else // Linux
    typedef void snd_pcm_t;
    typedef int (*pfnspo)(snd_pcm_t**,const char*,int,int); typedef int (*pfn_snd_pcm_close)(snd_pcm_t*);    typedef int (*pfnspw)(snd_pcm_t*,const void*,u32);
    typedef int (*pfnspr)(snd_pcm_t*,int,int);              typedef int (*pfnspp)(snd_pcm_t*);               typedef int (*pfnsphps)();
    typedef int (*pfnsphpa)(snd_pcm_t*,void*);              typedef int (*pfnsphpsa)(snd_pcm_t*,void*,u32);  typedef int (*pfnsphpsf)(snd_pcm_t*,void*,int);
    typedef int (*pfnsphp)(snd_pcm_t*,void*);               typedef int (*pfnsphpsc)(snd_pcm_t*, void*,u32); typedef int (*pfnsphpsrn)(snd_pcm_t*,void*,u32*,int*);
    typedef int (*pfnsphpspsn)(snd_pcm_t*,void*,u64*,int*); typedef int (*pfnsphpspn)(snd_pcm_t*,void*,u32*,int*); static snd_pcm_t *apcm; static pfnspw snd_pcm_writei; static pfnspr snd_pcm_recover;
    static bool alsa_try_open_default() {
        void *so = dlopen("libasound.so.2",2); if (!so) {so = dlopen("libasound.so",2);} if (!so) { DualLog("Audio: libasound not found\n"); return false; }
        pfnspo spo = dlsym(so,"snd_pcm_open");                              pfnsphpa sphpa = dlsym(so,"snd_pcm_hw_params_any");            pfnsphps sphps = dlsym(so,"snd_pcm_hw_params_sizeof");            pfnsphpsa sphpsa = dlsym(so,"snd_pcm_hw_params_set_access");
        pfnsphpsf sphpsf = dlsym(so,"snd_pcm_hw_params_set_format");        pfnsphpsc sphpsc = dlsym(so,"snd_pcm_hw_params_set_channels"); pfnsphpsrn sphpsrn = dlsym(so,"snd_pcm_hw_params_set_rate_near"); pfnsphpspsn sphpspsn = dlsym(so,"snd_pcm_hw_params_set_period_size_near");
        pfnsphpspn sphpspn= dlsym(so,"snd_pcm_hw_params_set_periods_near"); pfnsphp snd_pcm_hw_params = dlsym(so,"snd_pcm_hw_params");     snd_pcm_writei  = dlsym(so,"snd_pcm_writei");                     snd_pcm_recover = dlsym(so,"snd_pcm_recover"); 
        pfnspp spp = dlsym(so,"snd_pcm_prepare");
        if (!spo || !sphps || !sphpa || !sphpsa || !sphpsf || !sphpsc || !sphpsrn || !sphpspsn || !sphpspn || !snd_pcm_hw_params || !snd_pcm_writei || !snd_pcm_recover || !spp) { DualLogError("Audio: libasound missing required symbols\n"); return false; }
        int r = spo(&apcm,"default",0,0);  if (r < 0 ||                            !apcm) { DualLogError("snd_pcm_open('default') failed: %d\n",r); return false; }
        int sz = sphps(); u8 hwp_buf[640]; if (sz > (int)sizeof(hwp_buf)                ) { DualLogError("hw_params_t too large (%d)\n",sz); return false; }
        void *hwp = hwp_buf;               if ((r = sphpa(apcm,hwp))                 < 0) { DualLogError("hw_params_any failed\n"); return false; }
                                           if ((r = sphpsa(apcm,hwp,3))              < 0) { DualLogError("set_access failed\n"); return false; }
                                           if ((r = sphpsf(apcm,hwp,2)   )           < 0) { DualLogError("set_format S16_LE failed\n"); return false; }
                                           if ((r = sphpsc(apcm,hwp,AUDIO_CHANNELS)) < 0) { DualLogError("set_channels(%d) failed\n",AUDIO_CHANNELS); return false; }
        u32 rate   =AUDIO_RATE; int dir=0; if ((r = sphpsrn(apcm,hwp,&rate,&dir))    < 0) { DualLogError("set_rate(%u) failed\n", AUDIO_RATE); return false; }
        u64 period =AUDIO_FRAMES;   dir=0; if ((r = sphpspsn(apcm,hwp,&period,&dir)) < 0) { DualLogError("set_period_size(%d) failed\n", AUDIO_FRAMES); return false; }
        u32 periods=AUDIO_PERIODS;  dir=0; if ((r = sphpspn(apcm,hwp,&periods,&dir)) < 0) { DualLogError("set_periods(%d) failed\n", AUDIO_PERIODS); return false; }
                                           if ((r = snd_pcm_hw_params(apcm,hwp))     < 0) { DualLogError("hw_params apply failed\n"); return false; }
                                           if ((r = spp(apcm))                       < 0) { DualLogError("snd_pcm_prepare failed\n"); return false; }
        return true;
    }

    static void init_pcm_device(i32 card, i32 dev) {
        FHandle r = pcm_open(card,dev,1|(1<<1)); if (r == INVALID_FHANDLE) return;
        pcm_params_t p; hw_params_fill(&p.hw_params); pcm_sw_params_t *sw = &p.sw_params; mset(sw,0,sizeof(*sw)); sw->start_threshold = 1; sw->period_step = 1;
        hw_params_set(&p.hw_params,0/*PCM_FORMAT*/,3);                  hw_params_set(&p.hw_params,11/*PCM_RATE*/,AUDIO_RATE); hw_params_set(&p.hw_params,10/*PCM_CHANNELS*/,AUDIO_CHANNELS);
        hw_params_set(&p.hw_params,13/*PCM_PERIOD_SIZE*/,AUDIO_FRAMES); hw_params_set(&p.hw_params,15,AUDIO_PERIODS);
        if (pcm_params_setup(r,&p) >= 0 && pcm_fd_count < 8) pcm_fds[pcm_fd_count++] = r;
        else { DualLogError("Audio: raw device card=%d dev=%d setup failed, closing\n",card,dev); OS_Close(r); }
    }

    void AudioUpdate() { if (!apcm) {return;} i16 buf[AUDIO_FRAMES*AUDIO_CHANNELS]; audio_mix_period(buf); int r = snd_pcm_writei(apcm,buf,(u32)AUDIO_FRAMES); if (r < 0 && snd_pcm_recover(apcm,r,0) >= 0) { snd_pcm_writei(apcm,buf,(u32)AUDIO_FRAMES); } }
    void InitAudio() { if (!alsa_try_open_default()) { for (i32 card = 0; card < 8; card++) { for (i32 dev = 0; dev < 8; dev++) init_pcm_device(card,dev); } if (pcm_fd_count == 0) {DualLogError("Audio: no output device found\n"); return; } } pthread_create(&audThreadID,NULL,AudThread,NULL); }
#endif

void* AudThread(void* arg) { (void)arg; while (1) { AudioUpdate(); OS_USleep(1000); } return NULL; }
