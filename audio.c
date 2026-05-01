// audio.c - Audio System
#include "os.h"
#include "voxen.h"
#include <math.h>
#define DR_WAV_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION
#define DR_MP3_FLOAT_OUTPUT
#include "dr_wav.h"
#include "dr_mp3.h"
#include "nanoalsa.c"
#define AUDIO_RATE      48000
#define AUDIO_CHANNELS  2
#define AUDIO_PERIOD_MS 10
#define AUDIO_PERIODS   4
#define AUDIO_FRAMES    ((AUDIO_RATE * AUDIO_PERIOD_MS) / 1000)
typedef struct { float *samples; u32 frame_count,frame_pos; float volume; bool looping,positional,playing; Vector3 pos; } wav_channel_t;
typedef struct { drmp3 dec; bool open; float fade_vol,fade_target,fade_step; u32 src_rate; u64 frames_decoded,total_frames; } mp3_channel_t;
static wav_channel_t wav_ch[MAX_CHANNELS];
static i32           wav_count = 0;
static wav_channel_t *ext_ch[MAX_CHANNELS];
static i32            ext_count = 0;
static mp3_channel_t mp3_ch[2];
static i32           mp3_slot = 0;
static float        *log_samples;
static u32           log_frame_count,log_frame_pos;
static bool          log_playing;
static bool mp3_paused = false;
static float sfx_scale(void)     { return (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeEffects/100.0f); }
static float music_scale(void)   { return (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeMusic/100.0f); }
static float message_scale(void) { return (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeMessage/100.0f); }
static float spatial_atten(Vector3 pos) {
    Vector3 d = {pos.x-Sys_Global.instances[PLAYER1].position.x, pos.y-Sys_Global.instances[PLAYER1].position.y, pos.z-Sys_Global.instances[PLAYER1].position.z};
    float dist = sqrtf(d.x*d.x+d.y*d.y+d.z*d.z);
    if (dist <= 1.0f) return 1.0f;
    if (dist >= 64.0f) return 0.0f;
    return 1.0f-(dist-1.0f)/63.0f;
}

static inline i16 f32_to_s16(float s) { s = s>1.0f?1.0f:(s<-1.0f?-1.0f:s); return (i16)(s*32767.0f); }
static float *resample_stereo(float *src,u32 *frames,u32 src_rate) {
    if (src_rate == AUDIO_RATE) return src;
    u32 sf = *frames, df = (u32)((u64)sf*AUDIO_RATE/src_rate);
    float *dst = (float*)malloc(df*2*sizeof(float));
    if (!dst) return src;
    float ratio = (float)sf/(float)df;
    for (u32 i = 0; i < df; i++) {
        float pos = i*ratio; u32 a = (u32)pos, b = a+1<sf?a+1:a; float t = pos-(float)a;
        dst[i*2+0] = src[a*2+0]+t*(src[b*2+0]-src[a*2+0]);
        dst[i*2+1] = src[a*2+1]+t*(src[b*2+1]-src[a*2+1]);
    }
    free(src); *frames = df; return dst;
}

static float *load_wav(const char *path,u32 *out_frames) {
    drwav wav;
    if (!drwav_init_file(&wav,path,NULL)) return NULL;
    if (wav.channels > 2) { drwav_uninit(&wav); return NULL; }
    u64 frames = wav.totalPCMFrameCount;
    float *buf = (float*)malloc(frames*AUDIO_CHANNELS*sizeof(float));
    if (!buf) { drwav_uninit(&wav); return NULL; }
    u64 got = drwav_read_pcm_frames_f32(&wav,frames,buf);
    if (wav.channels == 1) for (i64 i=(i64)got-1;i>=0;i--) { buf[i*2+1]=buf[i]; buf[i*2]=buf[i]; }
    u32 src_rate = wav.sampleRate;
    drwav_uninit(&wav);
    *out_frames = (u32)got;
    return resample_stereo(buf,out_frames,src_rate);
}

static void audio_mix_period(i16 *out) {
    float mix[AUDIO_FRAMES*AUDIO_CHANNELS];
    SetMemoryToValueForNBytes(mix,0,sizeof(mix));

    for (i32 c = 0; c < wav_count; c++) {
        wav_channel_t *w = &wav_ch[c];
        if (!w->playing || !w->samples) continue;
        float vol = w->volume*sfx_scale();
        if (w->positional) vol *= spatial_atten(w->pos);
        for (i32 f = 0; f < AUDIO_FRAMES; f++) {
            if (w->frame_pos >= w->frame_count) { if (w->looping) w->frame_pos=0; else { w->playing=false; break; } }
            mix[f*2+0] += w->samples[w->frame_pos*2+0]*vol;
            mix[f*2+1] += w->samples[w->frame_pos*2+1]*vol;
            w->frame_pos++;
        }
    }
    
    for (i32 c=0;c<ext_count;c++) {
        wav_channel_t *w=ext_ch[c];
        if (!w->playing||!w->samples) continue;
        float vol=w->volume*sfx_scale();
        if (w->positional) vol*=spatial_atten(w->pos);
        for (i32 f=0;f<AUDIO_FRAMES;f++) {
            if (w->frame_pos>=w->frame_count) { if (w->looping) w->frame_pos=0; else { w->playing=false; break; } }
            mix[f*2+0]+=w->samples[w->frame_pos*2+0]*vol;
            mix[f*2+1]+=w->samples[w->frame_pos*2+1]*vol;
            w->frame_pos++;
        }
    }

    if (log_playing && log_samples) {
        float vol = message_scale();
        for (i32 f = 0; f < AUDIO_FRAMES; f++) {
            if (log_frame_pos >= log_frame_count) { log_playing=false; break; }
            mix[f*2+0] += log_samples[log_frame_pos*2+0]*vol;
            mix[f*2+1] += log_samples[log_frame_pos*2+1]*vol;
            log_frame_pos++;
        }
    }

    if (!mp3_paused) {
        for (i32 s = 0; s < 2; s++) {
            mp3_channel_t *m = &mp3_ch[s];
            if (!m->open) continue;
            u32 src_rate = m->src_rate ? m->src_rate : AUDIO_RATE;
            drmp3_uint64 frames_to_read = (src_rate == AUDIO_RATE) ? AUDIO_FRAMES : (drmp3_uint64)((u64)AUDIO_FRAMES*src_rate/AUDIO_RATE)+2;
            float raw[AUDIO_FRAMES*4];
            drmp3_uint64 got = drmp3_read_pcm_frames_f32(&m->dec,frames_to_read,raw);
            if (got == 0) { drmp3_uninit(&m->dec); m->open=false; continue; }
            float vol = m->fade_vol*music_scale();
            m->frames_decoded += got;
            float ratio = (float)got/(float)AUDIO_FRAMES;
            for (i32 f = 0; f < AUDIO_FRAMES; f++) {
                float pos = f*ratio; u32 a=(u32)pos, b=(a+1<(u32)got)?a+1:a; float t=pos-(float)a;
                float l = raw[a*2+0]+t*(raw[b*2+0]-raw[a*2+0]);
                float r = raw[a*2+1]+t*(raw[b*2+1]-raw[a*2+1]);
                mix[f*2+0] += l*vol; mix[f*2+1] += r*vol;
                if (m->fade_step != 0.0f) {
                    m->fade_vol += m->fade_step;
                    if (m->fade_step>0.0f && m->fade_vol>=m->fade_target) { m->fade_vol=m->fade_target; m->fade_step=0.0f; }
                    else if (m->fade_step<0.0f && m->fade_vol<=m->fade_target) {
                        m->fade_vol=m->fade_target; m->fade_step=0.0f;
                        if (m->fade_target==0.0f) { drmp3_uninit(&m->dec); m->open=false; }
                    }
                }
            }
        }
    }

    for (i32 i = 0; i < AUDIO_FRAMES*AUDIO_CHANNELS; i++) out[i] = f32_to_s16(mix[i]);
}

ENGINE_TO_MOD void play_wav(const char *path,float volume,Vector3 pos,bool positional) {
    i32 slot = -1;
    for (i32 i = 0; i < wav_count; i++) if (!wav_ch[i].playing && wav_ch[i].samples) { free(wav_ch[i].samples); wav_ch[i].samples=NULL; slot=i; break; }
    if (slot==-1 && wav_count<MAX_CHANNELS) slot=wav_count++;
    if (slot==-1) { DualLog("WARNING: Max WAV channels (%d) reached\n",MAX_CHANNELS); return; }
    u32 frames; float *buf = load_wav(path,&frames);
    if (!buf) { DualLog("ERROR: Failed to load WAV %s\n",path); return; }
    wav_channel_t *w = &wav_ch[slot];
    w->samples=buf; w->frame_count=frames; w->frame_pos=0; w->volume=volume;
    w->looping=false; w->positional=positional; w->pos=pos; w->playing=true;
}

ENGINE_TO_MOD void play_message(const char *path) {
    if (log_playing && log_samples) { log_playing=false; free(log_samples); log_samples=NULL; }
    u32 frames; float *buf = load_wav(path,&frames);
    if (!buf) { DualLog("ERROR: Failed to load message WAV %s\n",path); return; }
    log_samples=buf; log_frame_count=frames; log_frame_pos=0; log_playing=true;
}

ENGINE_TO_MOD i32 SoundInit(const char *path,ma_sound *pSound) { wav_channel_t *w=(wav_channel_t*)pSound; u32 frames; float *buf=load_wav(path,&frames); if (!buf) return -1; w->samples=buf; w->frame_count=frames; w->frame_pos=0; w->volume=1.0f; w->looping=false; w->positional=false; w->playing=false; return 0; }
ENGINE_TO_MOD i32 SoundStart(ma_sound *pSound) {
    wav_channel_t *w=(wav_channel_t*)pSound; w->frame_pos=0; w->playing=true;
    for (i32 i=0;i<ext_count;i++) if (ext_ch[i]==w) return 0;
    if (ext_count<MAX_CHANNELS) ext_ch[ext_count++]=w;
    return 0;
}
ENGINE_TO_MOD i32 SoundStop(ma_sound *pSound) { ((wav_channel_t*)pSound)->playing=false; return 0; }
ENGINE_TO_MOD void SoundUninit(ma_sound *pSound) {
    wav_channel_t *w=(wav_channel_t*)pSound;
    if (w->samples){free(w->samples);w->samples=NULL;} w->playing=false;
    for (i32 i=0;i<ext_count;i++) if (ext_ch[i]==w) { ext_ch[i]=ext_ch[--ext_count]; break; }
}
ENGINE_TO_MOD void SoundSetVolume(ma_sound *pSound,float volume) { ((wav_channel_t*)pSound)->volume=volume; }
ENGINE_TO_MOD void SoundSetLooping(ma_sound *pSound,ma_bool32 loop) { ((wav_channel_t*)pSound)->looping=(bool)loop; }
ENGINE_TO_MOD bool GetSoundIsPlaying(ma_sound *pSound)  { return ((wav_channel_t*)pSound)->playing; }
ENGINE_TO_MOD i32 SoundGetCurrentFrameCursor(const ma_sound *pSound,u64 *pCursor) { *pCursor=((wav_channel_t*)pSound)->frame_pos; return 0; }
ENGINE_TO_MOD float SoundGetLength(ma_sound *pSound) { wav_channel_t *w=(wav_channel_t*)pSound; return (w->samples&&AUDIO_RATE)?(float)w->frame_count/(float)AUDIO_RATE:0.0f; }
static void mp3_open_slot(i32 s,const char *path,float fade_from,float fade_to,i32 fade_ms) {
    mp3_channel_t *m = &mp3_ch[s];
    if (m->open) { drmp3_uninit(&m->dec); m->open=false; }
    if (!drmp3_init_file(&m->dec,path,NULL)) { DualLog("ERROR: Failed to load MP3 %s\n",path); return; }
    m->src_rate=m->dec.sampleRate;
    m->total_frames=drmp3_get_pcm_frame_count(&m->dec);
    drmp3_seek_to_pcm_frame(&m->dec,0);
    m->frames_decoded=0; m->open=true; m->fade_vol=fade_from; m->fade_target=fade_to;
    m->fade_step=(fade_ms>0)?(fade_to-fade_from)/((float)AUDIO_RATE*fade_ms/1000.0f):0.0f;
    if (m->fade_step==0.0f) m->fade_vol=fade_to;
}

void play_mp3(const char *path,i32 fade_in_ms) {
    i32 old=mp3_slot, next=mp3_slot?0:1;
    if (mp3_ch[old].open) { mp3_ch[old].fade_target=0.0f; mp3_ch[old].fade_step=(fade_in_ms>0)?-mp3_ch[old].fade_vol/((float)AUDIO_RATE*fade_in_ms/1000.0f):-1.0f; }
    mp3_open_slot(next,path,0.0f,1.0f,fade_in_ms);
    mp3_slot=next;
}

void mp3_clear(void) { for (i32 i=0;i<2;i++) if (mp3_ch[i].open) { drmp3_uninit(&mp3_ch[i].dec); mp3_ch[i].open=false; } mp3_slot=0; }
ENGINE_TO_MOD void MP3Pause(void) { mp3_paused = true; }
ENGINE_TO_MOD void MP3Resume(void) { mp3_paused = false; }
ENGINE_TO_MOD float GetMP3RemainingTime(void) {
    mp3_channel_t *m = &mp3_ch[mp3_slot];
    if (!m->open) return 0.0f;
    if (m->total_frames==0) return 1.0f;
    if (m->frames_decoded>=m->total_frames) return 0.0f;
    return (float)(m->total_frames-m->frames_decoded)/(float)(m->src_rate?m->src_rate:AUDIO_RATE);
}

float GetSFXVolume(float volume) { return sfx_scale()*volume; }
float GetMusicVolume(void)       { return music_scale(); }
float GetMessageVolume(void)     { return message_scale(); }
void set_music_volume(void)      {}
void set_sfx_volume(void)        {}
void set_message_volume(void)    {}
void set_master_volume(void)     {}
void SetPlayerListenerOrientation(void) {}
void SetPlayerListenerPos(void)         {}

#define MAX_PCM_DEVICES 8
static i32 pcm_fds[MAX_PCM_DEVICES];
static i32 pcm_fd_count = 0;
static void init_pcm_device(i32 card,i32 dev) {
    i32 r = pcm_open(card,dev,PCM_OUTPUT|PCM_NONBLOCK);
    if (r<0) return;
    
    pcm_params_t p; pcm_params_init(&p);
    pcm_set(&p,PCM_FORMAT,PCM_FORMAT_S16_LE); pcm_set(&p,PCM_ACCESS,PCM_ACCESS_RW);
    pcm_set(&p,PCM_RATE,AUDIO_RATE); pcm_set(&p,PCM_CHANNELS,AUDIO_CHANNELS);
    pcm_set(&p,PCM_PERIOD_SIZE,AUDIO_FRAMES); pcm_set(&p,PCM_PERIODS,AUDIO_PERIODS);
    if (pcm_params_setup(r,&p)>=0 && pcm_fd_count<MAX_PCM_DEVICES) {
        DualLog("Audio: opened card %d device %d\n",card,dev);
        pcm_fds[pcm_fd_count++] = r;
    } else OS_Close(r);
}

void InitAudio(void) {
    for (i32 card=0;card<8;card++)
        for (i32 dev=0;dev<8;dev++)
            init_pcm_device(card,dev);
    if (pcm_fd_count==0) DualLog("ERROR: No audio output device found\n");
    else DualLog("Audio: %d device(s) active\n",pcm_fd_count);
}

void AudioUpdate(void) {
    if (pcm_fd_count==0) return;
    
    i16 buf[AUDIO_FRAMES*AUDIO_CHANNELS]; pcm_sync_t sync;
    if (pcm_sync(pcm_fds[0],&sync,SNDRV_PCM_SYNC_PTR_HWSYNC)<0) return;
    
    snd_pcm_uframes_t hw=sync.status.hw_ptr,appl=sync.control.appl_ptr;
    snd_pcm_uframes_t buffer_size=AUDIO_FRAMES*AUDIO_PERIODS,queued=appl-hw;
    if (queued>buffer_size) queued=0;
    snd_pcm_uframes_t avail=buffer_size-queued;
    while (avail>=(snd_pcm_uframes_t)AUDIO_FRAMES) {
        audio_mix_period(buf);
        for (i32 i=0;i<pcm_fd_count;i++) { if (pcm_write(pcm_fds[i],buf,AUDIO_FRAMES)<0) {pcm_prepare(pcm_fds[i]);} }
        avail-=AUDIO_FRAMES;
    }
}
