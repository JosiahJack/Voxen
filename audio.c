// audio.c - Audio System
#include "os.h"
#include "voxen.h"
#include "dr_mp3.h"
#ifdef WINDOWS
    #include "nanowasapi.c"
#else
    #include "nanoalsa.c"
#endif
#define AUDIO_RATE      48000
#define AUDIO_CHANNELS  2
#define AUDIO_PERIOD_MS 10
#define AUDIO_PERIODS   4
#define AUDIO_FRAMES    ((AUDIO_RATE * AUDIO_PERIOD_MS) / 1000)

// Wav parsing
typedef struct { OsFileHandle fp; u16 channels,bitsPerSample,fmtTag; u32 sampleRate; u64 totalPCMFrameCount,dataChunkDataPos,bytesRemaining; } WaveFile;
static u16 WavU16LE(const u8 *d) { return (u16)(d[0]|(d[1]<<8)); }
static u32 WavU32LE(const u8 *d) { return (u32)(d[0]|(d[1]<<8)|(d[2]<<16)|(d[3]<<24)); }
static bool WavInit(WaveFile *w, const char *path) {
    u8 buf[36]; MemSetToValueForNBytes(w,0,sizeof(*w));
    w->fp = OS_OpenReadonly(path);
    if (w->fp == OS_INVALID_HANDLE) return false;
    if (OS_Read(w->fp, buf, 12) != 12) goto fail;
    if (CompareMemoryForNBytes(buf,"RIFF",4) != 0) goto fail;
    if (CompareMemoryForNBytes(buf+8,"WAVE",4) != 0) goto fail;
    bool got_fmt=false,got_data=false;
    for (;;) {
        u8 chunkId[4],szBuf[4];
        if (OS_Read(w->fp,chunkId,4) != 4) break;
        if (OS_Read(w->fp,szBuf,4) != 4) break;
        
        u32 chunkSize = WavU32LE(szBuf);
        if (CompareMemoryForNBytes(chunkId, "fmt ", 4) == 0) {
            if (chunkSize < 16) goto fail;
            
            u8 fmt[18]; u32 toRead = chunkSize < 18 ? chunkSize : 18;
            if (OS_Read(w->fp,fmt,toRead) != (long)toRead) goto fail;
            if (chunkSize > toRead) OS_Seek(w->fp, (i64)(chunkSize - toRead),SEEK_CUR);
            w->fmtTag = WavU16LE(fmt+0); w->channels = WavU16LE(fmt+2); w->sampleRate = WavU32LE(fmt+4); w->bitsPerSample = WavU16LE(fmt+14);
            if (w->fmtTag == 0xFFFE && toRead >= 18) {
                u16 cbSize = WavU16LE(fmt + 16);
                if (cbSize >= 22) {
                    u8 ext[22];
                    if (OS_Read(w->fp,ext,22) == 22) w->fmtTag = WavU16LE(ext + 6);
                }
            }
            if (w->fmtTag != 0x1) goto fail; // PCM format
            if (w->bitsPerSample != 8 && w->bitsPerSample != 16) goto fail;
            got_fmt = true;
        } else if (CompareMemoryForNBytes(chunkId,"data",4) == 0) {
            w->dataChunkDataPos = (u64)OS_Tell(w->fp);
            u32 bpf = (u32)w->channels * (w->bitsPerSample / 8);
            if (bpf == 0) goto fail;
            w->bytesRemaining = chunkSize - (chunkSize % bpf);
            w->totalPCMFrameCount = w->bytesRemaining / bpf;
            got_data = true;
            break; /* data chunk is last thing we need */
        } else {
            OS_Seek(w->fp,(i64)(chunkSize + (chunkSize & 1)),SEEK_CUR);
        }
    }

    if (got_fmt && got_data) return true;
    fail:
    if (w->fp != OS_INVALID_HANDLE) { OS_Close(w->fp); w->fp = OS_INVALID_HANDLE; }
    return false;
}

static u64 WavReadPCMFrames(WaveFile *w, u64 framesToRead, float *out) {
    if (!w || !out || framesToRead == 0) return 0;
    u32 bps = w->bitsPerSample; u32 bpf = (u32)w->channels * (bps / 8); if (bpf == 0) return 0;

    u64 framesLeft = w->bytesRemaining / bpf;
    if (framesToRead > framesLeft) framesToRead = framesLeft;
    u64 totalRead = 0; u8  tmp[4096];
    while (framesToRead > 0) {
        u64 batchFrames=framesToRead; u64 batchBytes=batchFrames * bpf;
        if (batchBytes > sizeof(tmp)) { batchFrames = sizeof(tmp) / bpf; batchBytes  = batchFrames * bpf; }
        size_t got = OS_Read(w->fp, tmp, (size_t)batchBytes);
        u64 gotFrames = got / bpf;
        u64 samples   = gotFrames * w->channels;
        if (bps == 8) {
            for (u64 i = 0; i < samples; i++) *out++ = (tmp[i] / 255.0f) * 2.0f - 1.0f;
        } else { // 16bit LE
            for (u64 i = 0; i < samples; i++) {
                i16 s;
                MemSetToValueForNBytes(&s,(i64)tmp + i*2,2);
                *out++ = s * (1.0f / 32768.0f);
            }
        }

        w->bytesRemaining -= gotFrames * bpf; framesToRead -= gotFrames; totalRead += gotFrames; if (gotFrames < batchFrames) break;
    }
    return totalRead;
}

static void WavUnInit(WaveFile *w) { if (w && w->fp != OS_INVALID_HANDLE) { OS_Close(w->fp); w->fp = OS_INVALID_HANDLE; } }

typedef struct { float *samples; u32 frame_count,frame_pos; float volume; bool looping,positional,playing; Vector3 pos; size_t allocSize; } wav_channel_t;
typedef struct { drmp3 dec; bool open; float fade_vol,fade_target,fade_step; u32 src_rate; u64 frames_decoded,total_frames; } mp3_channel_t;
static wav_channel_t wav_ch[MAX_CHANNELS];
static i32           wav_count = 0;
static wav_channel_t *ext_ch[MAX_CHANNELS];
static i32            ext_count = 0;
static mp3_channel_t mp3_ch[2];
static i32           mp3_slot = 0;
static float        *log_samples;
static size_t log_allocSize=0;
static u32           log_frame_count,log_frame_pos;
static bool          log_playing;
static bool mp3_paused = false;
static float sfx_scale(void)     { return (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeEffects/100.0f); }
static float music_scale(void)   { return (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeMusic/100.0f); }
static float message_scale(void) { return (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeMessage/100.0f); }
static float spatial_atten(Vector3 pos) {
    Vector3 d = {pos.x-Sys_Global.instances[PLAYER1].position.x, pos.y-Sys_Global.instances[PLAYER1].position.y, pos.z-Sys_Global.instances[PLAYER1].position.z};
    float dist = vsqrtf(d.x*d.x+d.y*d.y+d.z*d.z);
    if (dist <= 1.0f) return 1.0f;
    if (dist >= 64.0f) return 0.0f;
    return 1.0f-(dist-1.0f)/63.0f;
}

static inline i16 f32_to_s16(float s) { s = s>1.0f?1.0f:(s<-1.0f?-1.0f:s); return (i16)(s*32767.0f); }
static float *resample_stereo(float *src, size_t srcSize, u32 *frames, u32 src_rate, size_t* allocSize) {
    if (src_rate == AUDIO_RATE) return src;
    
    u32 sf = *frames, df = (u32)((u64)sf*AUDIO_RATE/src_rate);
    float *dst = (float*)OS_Alloc(df*2*sizeof(float)); *allocSize = df*2*sizeof(float);
    float ratio = (float)sf/(float)df;
    for (u32 i = 0; i < df; i++) {
        float pos = i*ratio; u32 a = (u32)pos, b = a+1<sf?a+1:a; float t = pos-(float)a;
        dst[i*2+0] = src[a*2+0]+t*(src[b*2+0]-src[a*2+0]);
        dst[i*2+1] = src[a*2+1]+t*(src[b*2+1]-src[a*2+1]);
    }
    OS_DeallocateRAM(src,srcSize); *frames = df; return dst;
}

static float *load_wav(const char *path,u32 *out_frames, size_t* allocSize) {
    WaveFile wav; if (!WavInit(&wav,path)) return NULL;
    if (wav.channels > 2) { WavUnInit(&wav); return NULL; }
    
    u64 frames = wav.totalPCMFrameCount;
    float *buf = (float*)OS_Alloc(frames*AUDIO_CHANNELS*sizeof(float)); size_t bufSize = frames*AUDIO_CHANNELS*sizeof(float);
    u64 got = WavReadPCMFrames(&wav,frames,buf);
    if (wav.channels == 1) for (i64 i=(i64)got-1;i>=0;i--) { buf[i*2+1]=buf[i]; buf[i*2]=buf[i]; }
    u32 src_rate = wav.sampleRate;
    WavUnInit(&wav);
    *out_frames = (u32)got;
    return resample_stereo(buf,bufSize,out_frames,src_rate,allocSize);
}

static void audio_mix_period(i16 *out) {
    float mix[AUDIO_FRAMES*AUDIO_CHANNELS];
    MemSetToValueForNBytes(mix,0,sizeof(mix));
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
            u64 frames_to_read = (src_rate == AUDIO_RATE) ? AUDIO_FRAMES : (u64)((u64)AUDIO_FRAMES*src_rate/AUDIO_RATE)+2;
            float raw[AUDIO_FRAMES*4];
            u64 got = drmp3_read_pcm_frames_f32(&m->dec,frames_to_read,raw);
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
    for (i32 i = 0; i < wav_count; i++) if (!wav_ch[i].playing && wav_ch[i].samples) { OS_DeallocateRAM(wav_ch[i].samples,wav_ch[i].allocSize); wav_ch[i].samples=NULL; wav_ch[i].allocSize=0; slot=i; break; }
    if (slot==-1 && wav_count<MAX_CHANNELS) slot=wav_count++;
    if (slot==-1) { DualLog("WARNING: Max WAV channels (%d) reached\n",MAX_CHANNELS); return; }
    u32 frames; size_t allocSize=0; float *buf = load_wav(path,&frames,&allocSize);
    if (!buf) { DualLog("ERROR: Failed to load WAV %s\n",path); return; }
    wav_channel_t *w = &wav_ch[slot];
    w->samples=buf; w->allocSize = allocSize; w->frame_count=frames; w->frame_pos=0; w->volume=volume;
    w->looping=false; w->positional=positional; w->pos=pos; w->playing=true;
}

ENGINE_TO_MOD void play_message(const char *path) {
    if (log_playing && log_samples && log_allocSize > 0) { log_playing=false; OS_DeallocateRAM(log_samples,log_allocSize); log_samples=NULL; log_allocSize=0; }
    u32 frames; float *buf = load_wav(path,&frames,&log_allocSize);
    if (!buf) { DualLog("ERROR: Failed to load message WAV %s\n",path); return; }
    log_samples=buf; log_frame_count=frames; log_frame_pos=0; log_playing=true;
}

ENGINE_TO_MOD i32 SoundInit(const char *path,ma_sound *pSound) { wav_channel_t *w=(wav_channel_t*)pSound; u32 frames; size_t allocSize=0; float *buf=load_wav(path,&frames,&allocSize); if (!buf) return -1; w->samples=buf; w->allocSize=allocSize; w->frame_count=frames; w->frame_pos=0; w->volume=1.0f; w->looping=false; w->positional=false; w->playing=false; return 0; }
ENGINE_TO_MOD i32 SoundStart(ma_sound *pSound) {
    wav_channel_t *w=(wav_channel_t*)pSound; w->frame_pos=0; w->playing=true;
    for (i32 i=0;i<ext_count;i++) if (ext_ch[i]==w) return 0;
    if (ext_count<MAX_CHANNELS) ext_ch[ext_count++]=w;
    return 0;
}
ENGINE_TO_MOD i32 SoundStop(ma_sound *pSound) { ((wav_channel_t*)pSound)->playing=false; return 0; }
ENGINE_TO_MOD void SoundUninit(ma_sound *pSound) {
    wav_channel_t *w=(wav_channel_t*)pSound;
    if (w->samples){OS_DeallocateRAM(w->samples,w->allocSize);w->samples=NULL;w->allocSize=0;} w->playing=false;
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
static OsFileHandle pcm_fds[MAX_PCM_DEVICES];
static i32 pcm_fd_count = 0;
#ifndef WINDOWS
    static void init_pcm_device(i32 card,i32 dev) {
        OsFileHandle r = pcm_open(card,dev,1|(1 << 1));
        if (r==OS_INVALID_HANDLE) return;
        
        pcm_params_t p; pcm_params_init(&p);
        pcm_set(&p,PCM_FORMAT,SNDRV_PCM_FORMAT_S16_LE); pcm_set(&p,SNDRV_PCM_HW_PARAM_ACCESS,SNDRV_PCM_ACCESS_RW_INTERLEAVED);
        pcm_set(&p,PCM_RATE,AUDIO_RATE); pcm_set(&p,PCM_CHANNELS,AUDIO_CHANNELS);
        pcm_set(&p,PCM_PERIOD_SIZE,AUDIO_FRAMES); pcm_set(&p,SNDRV_PCM_HW_PARAM_PERIODS,AUDIO_PERIODS);
        if (pcm_params_setup(r,&p)>=0 && pcm_fd_count<MAX_PCM_DEVICES) {
            DualLog("Audio: opened card %d device %d\n",card,dev);
            pcm_fds[pcm_fd_count++]=r;
        } else OS_Close(r);
    }
#endif

void InitAudio(void) {
#ifdef WINDOWS
    OsFileHandle first = pcm_open_all(AUDIO_RATE,AUDIO_CHANNELS,AUDIO_FRAMES,AUDIO_PERIODS);
    if (first==OS_INVALID_HANDLE) { DualLog("ERROR: No WASAPI audio device found\n"); return; }
    pcm_fds[0]=first; pcm_fd_count=1;
    DualLog("Audio: WASAPI %d device(s) active\n",wasapi_dev_count);
#else
    for (i32 card=0;card<8;card++)
        for (i32 dev=0;dev<8;dev++)
            init_pcm_device(card,dev);
    if (pcm_fd_count==0) DualLog("ERROR: No audio output device found\n");
    else DualLog("Audio: %d device(s) active\n",pcm_fd_count);
#endif
}

void AudioUpdate(void) {
    if (pcm_fd_count==0) return;
    i16 buf[AUDIO_FRAMES*AUDIO_CHANNELS]; pcm_sync_t sync;
    if (pcm_sync(pcm_fds[0],&sync,SNDRV_PCM_SYNC_PTR_HWSYNC)<0) return;
    u32 hw=sync.status.hw_ptr,appl=sync.control.appl_ptr;
    u32 buffer_size=AUDIO_FRAMES*AUDIO_PERIODS,queued=appl-hw;
    if (queued>buffer_size) queued=0;
    u32 avail=buffer_size-queued;
    while (avail>=(u32)AUDIO_FRAMES) {
        audio_mix_period(buf);
        for (i32 i=0;i<pcm_fd_count;i++) if (pcm_write(pcm_fds[i],buf,AUDIO_FRAMES)<0) pcm_prepare(pcm_fds[i]);
        avail-=AUDIO_FRAMES;
    }
}
