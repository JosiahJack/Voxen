// audio.c - Audio System supporting .mp3 and .wav filetypes only, uses Windows WASAPI and Linux ALSA (uses "default" to work on PulseAudio and PipeWire or just ALSA+dmix systems, with raw ioctl fallback to all ALSA devices if "default" unavailable).  Mixes synthesized sounds as well.
#include "tables_audio.h"
#define AUDIO_RATE      48000
#define AUDIO_CHANNELS  2
#define AUDIO_PERIOD_MS 10
#define AUDIO_PERIODS   4
#define AUDIO_FRAMES    ((AUDIO_RATE * AUDIO_PERIOD_MS) / 1000)
#define AUDBUF_SIZE (AUDIO_FRAMES*AUDIO_PERIODS)
#ifdef WINDOWS
    #define FAILED(hr)    ((i32)(hr) <  0)
    #define PCM_NONBLOCK (1<<1)
    #define PCM_FORMAT_S16_LE 2
    #define REFIID const GUID *const
    typedef struct IMMDevice IMMDevice; typedef struct IMMDeviceEnumerator IMMDeviceEnumerator;
    typedef struct{ i32(__stdcall*q)(void*,const void*,void**); u32(__stdcall*a)(void*); u32(__stdcall*Release)(void*); i32(__stdcall* Activate)(void*,const void*,u32,void*,void**);} IMMDeviceVtbl; struct IMMDevice{IMMDeviceVtbl*lpVtbl;};
    typedef struct{ i32(__stdcall*q)(void*,const void*,void**); u32(__stdcall*a)(void*); u32(__stdcall*Release)(void*); i32(__stdcall*e)(void*,int,u32,void**); i32(__stdcall*GetDefaultAudioEndpoint)(void*,int,int,IMMDevice**);}IMMDeviceEnumeratorVtbl;struct IMMDeviceEnumerator{IMMDeviceEnumeratorVtbl*lpVtbl;};
    typedef struct IAudioClient IAudioClient; typedef struct IAudioRenderClient IAudioRenderClient; typedef struct { u16 t,n; u32 s, a; u16 b,w,c; } WAVEFORMATEX;
    typedef struct IAudioClientVtbl { i32 (__stdcall *QueryInterface)(void*, const void*,void**); u32 (__stdcall *AddRef)(void*); u32 (__stdcall *Release)(void*); i32 (__stdcall *Initialize)(void*,int,u32,i64,i64,const WAVEFORMATEX*,const void*); i32 (__stdcall *GetBufferSize)(void*,u32*); i32 (__stdcall *GetStreamLength)(void*,i64*); i32 (__stdcall *GetCurrentPadding)(void*,u32*);
        i32 (__stdcall *IsFormatSupported)(void*, int, const WAVEFORMATEX*, WAVEFORMATEX**); i32 (__stdcall *GetMixFormat)(void*,WAVEFORMATEX**); i32 (__stdcall *GetDevicePeriod)(void*,i64*,i64*); i32 (__stdcall *Start)(void*); i32 (__stdcall *Stop)(void*); i32 (__stdcall *Reset)(void*); i32 (__stdcall *SetEventHandle)(void*,void*); i32 (__stdcall *GetService)(void*,const void*,void**); } IAudioClientVtbl;
    struct IAudioClient { IAudioClientVtbl* lpVtbl; }; typedef struct IAudioRenderClientVtbl { i32 (__stdcall *QueryInterface)(void*,const void*,void**); u32 (__stdcall *AddRef)(void*); u32 (__stdcall *Release)(void*); i32 (__stdcall *GetBuffer)(void*,u32,u8**); i32 (__stdcall *ReleaseBuffer)(void*,u32,u32); } IAudioRenderClientVtbl;
    struct IAudioRenderClient { IAudioRenderClientVtbl* lpVtbl; }; typedef struct IUnknown IUnknown; typedef struct IUnknownVtbl { i32 (__stdcall *QueryInterface)(IUnknown* This, const GUID* riid, void** ppvObject); u32 (__stdcall *AddRef)(IUnknown* This); u32 (__stdcall *Release)(IUnknown* This); } IUnknownVtbl; struct IUnknown { const IUnknownVtbl* lpVtbl; };
    typedef u32 snd_pcm_uframes_t; typedef struct { int format,access,rate,channels,period_frames,periods; } pcm_params_t; typedef struct { snd_pcm_uframes_t hw_ptr; }  pcm_status_t; typedef struct { snd_pcm_uframes_t appl_ptr; } pcm_control_t; typedef struct { pcm_status_t status; pcm_control_t control; } pcm_sync_t; typedef struct {IAudioClient *client; IAudioRenderClient *render; u32 buffer_frames; i32 rate,channels,period_frames; bool open; } wasapi_dev_t;
    i32 WINAPI CoInitializeEx(void*,u32); i32 WINAPI CoCreateInstance(const GUID*,IUnknown*,u32,REFIID,void**); static wasapi_dev_t wasapi_devs[8]; static int wasapi_dev_count = 0;
    #define FD_TO_IDX(fd) ((int)(intptr_t)(fd)-100)
    #define IDX_TO_FD(i)  ((FHandle)(intptr_t)((i)+100))
    static const GUID IID_IAudioClient = {0x1CB9AD4C,0xDBFA,0x4C32,{0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2}}; static const GUID IID_IAudioRenderClient = {0xF294ACFC,0x3146,0x4483,{0xA7,0xBF,0xAD,0xDC,0xA7,0xC2,0x60,0xE2}};
    static int wasapi_init_device(IMMDevice *dev,int r,int ch,int period_frames,int p) {
        if(wasapi_dev_count>=8){return -1;}
        wasapi_dev_t *w=&wasapi_devs[wasapi_dev_count]; i32 hr=dev->lpVtbl->Activate(dev,&IID_IAudioClient,23,NULL,(void**)&w->client); if(FAILED(hr)){DualLogError("WASAPI Activate failed, %u\n",hr); return -1;}
        WAVEFORMATEX fmt = {1,(u16)ch,(u32)r,(u32)(r*ch*2),(u16)(ch*2),16,0}; i64 buf_dur = (i64)(period_frames*p)*10000000LL/r; hr = w->client->lpVtbl->Initialize(w->client,0,524288,buf_dur,0,&fmt,NULL); if(FAILED(hr)){w->client->lpVtbl->Release(w->client); return -1;}
        w->client->lpVtbl->GetBufferSize(w->client,&w->buffer_frames); hr = w->client->lpVtbl->GetService(w->client,&IID_IAudioRenderClient,(void**)&w->render); if(FAILED(hr)){ w->client->lpVtbl->Release(w->client); return -1;}
        w->client->lpVtbl->Start(w->client);w->rate=r; w->channels=ch; w->period_frames=period_frames; w->open=true; return wasapi_dev_count++;
    }
    
    static const GUID CLSID_MMDeviceEnumerator_ = {0xBCDE0395,0xE52F,0x467C,{0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E}}; static const GUID IID_IMMDeviceEnumerator_ = {0xA95664D2,0x9614,0x4F35,{0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6}};
    FHandle pcm_open_all(int rate,int channels,int period_frames,int periods) {
        CoInitializeEx(NULL,0); IMMDeviceEnumerator *en = NULL; if (FAILED(CoCreateInstance(&CLSID_MMDeviceEnumerator_,NULL,23,&IID_IMMDeviceEnumerator_,(void**)&en))) { DualLogError("CoCreateInstance fail\n"); return INVALID_FHANDLE; }
        IMMDevice *dev = NULL; i32 hr = en->lpVtbl->GetDefaultAudioEndpoint(en,0,0,&dev); en->lpVtbl->Release(en); if (FAILED(hr)||!dev) return INVALID_FHANDLE;
        int idx = wasapi_init_device(dev,rate,channels,period_frames,periods); dev->lpVtbl->Release(dev); if (idx<0) return INVALID_FHANDLE;
        return IDX_TO_FD(0);
    }

    int pcm_sync(FHandle fd, pcm_sync_t *sync) { int idx=FD_TO_IDX(fd); if(idx<0||idx>=wasapi_dev_count||!wasapi_devs[idx].open){return -1;} wasapi_dev_t *w=&wasapi_devs[idx]; u32 padding=0; w->client->lpVtbl->GetCurrentPadding(w->client,&padding); snd_pcm_uframes_t base = (w->buffer_frames>(u32)(w->period_frames*4)) ? w->buffer_frames-(u32)(w->period_frames*4) : 0; sync->status.hw_ptr=base; sync->control.appl_ptr=base+padding; return 0; }
    int pcm_prepare(FHandle fd) { int i = FD_TO_IDX(fd); if (i < 0 || i >= wasapi_dev_count) return -1; wasapi_dev_t *w = &wasapi_devs[i]; return w->client->lpVtbl->Stop(w->client),w->client->lpVtbl->Reset(w->client),w->client->lpVtbl->Start(w->client), 0; }
    int pcm_write(void *buf, int frames) { for (int i=0;i<wasapi_dev_count;i++) { wasapi_dev_t *w=&wasapi_devs[i]; if(!w->open){continue;} u8* data = NULL; if(FAILED(w->render->lpVtbl->GetBuffer(w->render,(u32)frames,&data))){pcm_prepare(IDX_TO_FD(i)); continue;} mcpy(data,buf,frames*w->channels*2); w->render->lpVtbl->ReleaseBuffer(w->render,(u32)frames,0); } return frames; }
#else
    int ioctl(int fd, u64 request, ...);
    #define _IOC(dir, type, nr, size) (((dir) << 30) | ((type) << 8) | ((nr) << 0) | ((size) << 16))
    #define _IO(type, nr) _IOC(0U, (type), (nr), 0)
    #define _IOR(type, nr, size) _IOC(2U, (type), (nr), sizeof(size))
    #define _IOW(type, nr, size) _IOC(1U, (type), (nr), sizeof(size))
    #define _IOWR(type, nr, size) _IOC(2U | 1U, (type), (nr), sizeof(size))
    typedef u64 snd_pcm_uframes_t; typedef i64 snd_pcm_sframes_t; struct snd_mask { u32 bits[8]; }; struct snd_interval { u32 min,max,openmin:1, openmax:1, integer:1, empty:1; };
    struct snd_pcm_hw_params { u32 flags; struct snd_mask masks[3]; struct snd_mask mres[5]; struct snd_interval intervals[12]; struct snd_interval ires[9]; u32 rmask,cmask,info,msbits,rate_num,rate_den; snd_pcm_uframes_t fifo_size; u8 reserved[64]; };
    struct snd_pcm_sw_params { int tstamp_mode; u32 period_step,sleep_min; snd_pcm_uframes_t avail_min,xfer_align,start_threshold,stop_threshold,silence_threshold,silence_size,boundary; u32 proto,tstamp_type; u8 reserved[56]; };
    struct snd_pcm_mmap_status { int state,pad1; snd_pcm_uframes_t hw_ptr; struct timespec tstamp; int suspended_state; struct timespec audio_tstamp; };
    struct snd_pcm_mmap_control { snd_pcm_uframes_t appl_ptr; snd_pcm_uframes_t avail_min; };
    struct snd_pcm_sync_ptr { u32 flags; union { struct snd_pcm_mmap_status  status; u8 reserved[64]; } s; union { struct snd_pcm_mmap_control control; u8 reserved[64]; } c; };
    struct snd_pcm_status { int state; struct timespec trigger_tstamp; struct timespec tstamp; snd_pcm_uframes_t appl_ptr,hw_ptr; snd_pcm_sframes_t delay; snd_pcm_uframes_t avail,avail_max,overrange; int suspended_state; u32 audio_tstamp_data; struct timespec audio_tstamp; struct timespec driver_tstamp; u32 audio_tstamp_accuracy; u8 reserved[20]; };
    typedef struct snd_pcm_mmap_status  pcm_status_t; typedef struct snd_pcm_mmap_control pcm_control_t; typedef struct snd_pcm_hw_params pcm_hw_params_t; typedef struct snd_pcm_sw_params pcm_sw_params_t;
    struct pcm_params { pcm_hw_params_t hw_params; pcm_sw_params_t sw_params; }; typedef struct pcm_params pcm_params_t;
    typedef enum pcm_param {PCM_ACCESS=0,PCM_FORMAT=1,PCM_RATE=11,PCM_CHANNELS=10,PCM_PERIOD_SIZE=13,PCM_BUFFER_SIZE=17,PCM_PERIODS=15,PCM_INTERRUPT=20,PCM_TSTAMP_TYPE=21,PCM_AVAIL_MIN=22,PCM_START_THRESHOLD=23,PCM_XRUN_THRESHOLD=24,PCM_SILENCE_THRESHOLD=25,PCM_SILENCE_SIZE=26} pcm_param_t;
    static inline struct snd_mask* get_mask_struct(struct snd_pcm_hw_params *p, u32 parameter) { return &p->masks[parameter - 0]; }
    static inline struct snd_interval* get_interval_struct(struct snd_pcm_hw_params *p, u32 parameter) { return &p->intervals[parameter - 8]; }
    static void hw_params_set_mask(struct snd_pcm_hw_params *p, int parameter, u32 value) { struct snd_mask *m = get_mask_struct(p,parameter); if (m->bits[((value) / 32)] & (1 << ((value) % 32))) {mset(m, 0x00, sizeof(*m));} m->bits[((value) / 32)] |= (1 << ((value) % 32)); }
    static void hw_params_set_interval(struct snd_pcm_hw_params *p, int parameter, u32 min, u32 max) { struct snd_interval *i = get_interval_struct(p,parameter); i->openmin = i->openmax = 0; i->integer = 1; i->min = min; i->max = max; }
    static void hw_params_set(struct snd_pcm_hw_params *p, int parameter, u32 value) { if ((parameter >= 0 && parameter <= 2)) hw_params_set_mask(p,parameter,value); else if ((parameter >= 8 && parameter <= 19)) hw_params_set_interval(p,parameter,value,value); }
    static u32 hw_params_get_mask(struct snd_pcm_hw_params *p, int parameter, u32 value) { struct snd_mask *m=get_mask_struct(p,parameter); return m->bits[((value) / 32)] & (1 << ((value) % 32)); }
    static void hw_params_get_interval(struct snd_pcm_hw_params *p, int parameter, u32 *min, u32 *max) { struct snd_interval *i = get_interval_struct(p,parameter); *min = i->min + i->openmin; *max = i->max - i->openmax; }
    static u32 hw_params_get(struct snd_pcm_hw_params *p, int parameter, u32 value) { u32 r, t; return (parameter >= 0 && parameter <= 2) ? hw_params_get_mask(p,parameter,value) : ((parameter >= 8 && parameter <= 19) ? (hw_params_get_interval(p,parameter,&r,&t),r) : 0); }
    static void hw_params_fill(struct snd_pcm_hw_params *p) { mset(p,0,sizeof(*p)); mset(p->masks,0xff,sizeof(p->masks)); p->rmask = p->info = 0xffffffffU; for (int i=0;i<=11;i++) { p->intervals[i].min = 0; p->intervals[i].max = 0xffffffffU; } }
    unsigned long pcm_gethw(pcm_params_t *p, pcm_param_t param, u32 val) { return hw_params_get(&p->hw_params,param,val); }
    unsigned long pcm_getsw(pcm_params_t *p, pcm_param_t param) { pcm_sw_params_t *sw = &p->sw_params; return ((u64*)&sw->avail_min)[param - 22]; }
    int pcm_params_setup(int fd, pcm_params_t *p) {
        if (ioctl(fd,_IOWR('A',0x11,struct snd_pcm_hw_params),&p->hw_params) == -1) return -1;
        if (!pcm_getsw(p,22)) ((u64*)&p->sw_params.avail_min)[0] = pcm_gethw(p,13,0);
        if (!pcm_getsw(p,24)) ((u64*)&p->sw_params.avail_min)[2] = pcm_gethw(p,17,0);
        if (ioctl(fd,_IOW('A',0x03,int),&p->sw_params.tstamp_type) == -1) return -1; // Support ancient kernels
        if (ioctl(fd,_IOWR('A',0x13,struct snd_pcm_sw_params),&p->sw_params) == -1) return -1;
        return ioctl(fd,_IO('A',0x40));
    }

    int pcm_open(int card, int device, int flags) { char path[4096]; sFormat(path,sizeof(path),"/dev/snd/pcmC%uD%u%c",card,device,(flags & 1) == 0 ? 'c' : 'p'); return OS_Open(path,00000002 | (flags & (1 << 1) ? 00004000 : 0),0); }
#endif
#define MP3_HDR_IS_MONO(h)             (((h[3]) & 0xC0) == 0xC0)
#define MP3_HDR_IS_MS_STEREO(h)        (((h[3]) & 0xE0) == 0x60)
#define MP3_HDR_IS_CRC(h)              (!((h[1]) & 1))
#define MP3_HDR_TEST_PADDING(h)        ((h[2]) & 0x2)
#define MP3_HDR_TEST_MPEG1(h)          ((h[1]) & 0x8)
#define MP3_HDR_TEST_NOT_MPEG25(h)     ((h[1]) & 0x10)
#define MP3_HDR_TEST_I_STEREO(h)       ((h[3]) & 0x10)
#define MP3_HDR_TEST_MS_STEREO(h)      ((h[3]) & 0x20)
#define MP3_HDR_GET_STEREO_MODE(h)     (((h[3]) >> 6) & 3)
#define MP3_HDR_GET_STEREO_MODE_EXT(h) (((h[3]) >> 4) & 3)
#define MP3_HDR_GET_LAYER(h)           (((h[1]) >> 1) & 3)
#define MP3_HDR_GET_BITRATE(h)         ((h[2]) >> 4)
#define MP3_HDR_GET_SAMPLE_RATE(h)     (((h[2]) >> 2) & 3)
#define MP3_HDR_GET_SAMPLE_RATEHDR(h)  (MP3_HDR_GET_SAMPLE_RATE(h) + (((h[1]>>3)&1)+((h[1]>>4)&1))*3)
#define MP3_HDR_IS_FRAME_576(h)        ((h[1] & 14) == 2)
#define MP3_HDR_IS_LAYER_1(h)          ((h[1] & 6) == 6)
#define MP3_OFFSET_PTR(p,offset) ((void*)((u8*)(p)+(offset)))
static u8 g_halfrate[2][3][15]={ {{0,4,8,12,16,20,24,28,32,40,48,56,64,72,80},{0,4,8,12,16,20,24,28,32,40,48,56,64,72,80},{0,16,24,28,32,40,48,56,64,72,80,88,96,112,128}},{{0,16,20,24,28,32,40,48,56,64,80,96,112,128,160},{0,16,24,28,32,40,48,56,64,80,96,112,128,160,192},{0,16,32,48,64,80,96,112,128,144,160,176,192,208,224}} };
static u8 g_scf_long[8][23]={{0},{12,12,12,12,12,12,16,20,24,28,32,40,48,56,64,76,90,2,2,2,2,2,0},{0},{6,6,6,6,6,6,8,10,12,14,16,18,22,26,32,38,46,54,62,70,76,36,0},{0},{4,4,4,4,4,4,6,6,8,8,10,12,16,20,24,28,34,42,50,54,76,158,0},{4,4,4,4,4,4,6,6,6,8,10,12,16,18,22,28,34,40,46,54,54,192,0},{4,4,4,4,4,4,6,6,8,10,12,16,20,24,30,38,46,56,68,84,102,26,0}};
static u8 g_scf_short[8][40]={{4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0},{8,8,8,8,8,8,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0},{4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0 },{4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0 }, { 4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },{4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0},{4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0},{4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0}};
static u8 g_scf_mixed[8][40]={{6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0},{12,12,12,4,4,4,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0},{6,6,6,6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0},{6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0},{6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0},{4,4,4,4,4,4,6,6,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0},{4,4,4,4,4,4,6,6,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0},{4,4,4,4,4,4,6,6,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0}};
static const u8 g_sfc_long_024[23] = { 6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54,0 };
static const u8 g_scf_partitions[3][28]={{6,5,5,5,6,5,5,5,6,5,7,3,11,10,0,0,7,7,7,0,6,6,6,3,8,8,5,0}, {8,9,6,12,6,9,9,9,6,9,12,6,15,18,0,0,6,15,12,0,6,12,9,6,6,18,9,0}, {9,9,6,12,9,9,9,9,9,9,12,6,18,18,0,0,12,12,12,0,12,9,9,6,15,12,9,0}};
static const float g_mp3_pow43[129+16]={ 0,-1,-2.519842f,-4.326749f,-6.349604f,-8.549880f,-10.902724f,-13.390518f,-16.000000f,-18.720754f,-21.544347f,-24.463781f,-27.473142f,-30.567351f,-33.741992f,-36.993181f,0,1,2.519842f,4.326749f,6.349604f,8.549880f,10.902724f,13.390518f,16.000000f,18.720754f,21.544347f,24.463781f,27.473142f,30.567351f,33.741992f,36.993181f,40.317474f,43.711787f,47.173345f,50.699631f,54.288352f,57.937408f,61.644865f,65.408941f,69.227979f,73.100443f,77.024898f,81.000000f,85.024491f,89.097188f,93.216975f,97.382800f,101.593667f,105.848633f,110.146801f,114.487321f,118.869381f,123.292209f,127.755065f,132.257246f,136.798076f,141.376907f,145.993119f,150.646117f,155.335327f,160.060199f,164.820202f,169.614826f,174.443577f,179.305980f,184.201575f,189.129918f,194.090580f,199.083145f,204.107210f,209.162385f,214.248292f,219.364564f,224.510845f,229.686789f,234.892058f,240.126328f,245.389280f,250.680604f,256.000000f,261.347174f,266.721841f,272.123723f,277.552547f,283.008049f,288.489971f,293.998060f,299.532071f,305.091761f,310.676898f,316.287249f,321.922592f,327.582707f,333.267377f,338.976394f,344.709550f,350.466646f,356.247482f,362.051866f,367.879608f,373.730522f,379.604427f,385.501143f,391.420496f,397.362314f,403.326427f,409.312672f,415.320884f,421.350905f,427.402579f,433.475750f,439.570269f,445.685987f,451.822757f,457.980436f,464.158883f,470.357960f,476.577530f,482.817459f,489.077615f,495.357868f,501.658090f,507.978156f,514.317941f,520.677324f,527.056184f,533.454404f,539.871867f,546.308458f,552.764065f,559.238575f,565.731879f,572.243870f,578.774440f,585.323483f,591.890898f,598.476581f,605.080431f,611.702349f,618.342238f,625.000000f,631.675540f,638.368763f,645.079578f };
static const i16 tabs[]={ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,785,785,785,785,784,784,784,784,513,513,513,513,513,513,513,513,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-255,1313,1298,1282,785,785,785,785,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,290,288,-255,1313,1298,1282,769,769,769,769,529,529,529,529,529,529,529,529,528,528,528,528,528,528,528,528,512,512,512,512,512,512,512,512,290,288,-253,-318,-351,-367,785,785,785,785,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,819,818,547,547,275,275,275,275,561,560,515,546,289,274,288,258,
    -254,-287,1329,1299,1314,1312,1057,1057,1042,1042,1026,1026,784,784,784,784,529,529,529,529,529,529,529,529,769,769,769,769,768,768,768,768,563,560,306,306,291,259,-252,-413,-477,-542,1298,-575,1041,1041,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-383,-399,1107,1092,1106,1061,849,849,789,789,1104,1091,773,773,1076,1075,341,340,325,309,834,804,577,577,532,532,516,516,832,818,803,816,561,561,531,531,515,546,289,289,288,258,-252,-429,-493,-559,1057,1057,1042,1042,529,529,529,529,529,529,529,529,784,784,784,784,769,769,769,769,512,512,512,512,512,512,512,512,-382,1077,-415,1106,1061,1104,849,849,789,789,1091,1076,1029,1075,834,834,597,581,340,340,339,324,804,833,532,532,832,772,818,803,817,787,816,771,290,290,290,290,288,258,
    -253,-349,-414,-447,-463,1329,1299,-479,1314,1312,1057,1057,1042,1042,1026,1026,785,785,785,785,784,784,784,784,769,769,769,769,768,768,768,768,-319,851,821,-335,836,850,805,849,341,340,325,336,533,533,579,579,564,564,773,832,578,548,563,516,321,276,306,291,304,259,-251,-572,-733,-830,-863,-879,1041,1041,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-511,-527,-543,1396,1351,1381,1366,1395,1335,1380,-559,1334,1138,1138,1063,1063,1350,1392,1031,1031,1062,1062,1364,1363,1120,1120,1333,1348,881,881,881,881,375,374,359,373,343,358,341,325,791,791,1123,1122,-703,1105,1045,-719,865,865,790,790,774,774,1104,1029,338,293,323,308,-799,-815,833,788,772,818,803,816,322,292,307,320,561,531,515,546,289,274,288,258,
    -251,-525,-605,-685,-765,-831,-846,1298,1057,1057,1312,1282,785,785,785,785,784,784,784,784,769,769,769,769,512,512,512,512,512,512,512,512,1399,1398,1383,1367,1382,1396,1351,-511,1381,1366,1139,1139,1079,1079,1124,1124,1364,1349,1363,1333,882,882,882,882,807,807,807,807,1094,1094,1136,1136,373,341,535,535,881,775,867,822,774,-591,324,338,-671,849,550,550,866,864,609,609,293,336,534,534,789,835,773,-751,834,804,308,307,833,788,832,772,562,562,547,547,305,275,560,515,290,290,-252,-397,-477,-557,-622,-653,-719,-735,-750,1329,1299,1314,1057,1057,1042,1042,1312,1282,1024,1024,785,785,785,785,784,784,784,784,769,769,769,769,-383,1127,1141,1111,1126,1140,1095,1110,869,869,883,883,1079,1109,882,882,375,374,807,868,838,881,791,-463,867,822,368,263,852,837,836,-543,610,610,550,550,352,336,534,534,865,774,851,821,850,805,593,533,579,564,773,832,578,578,548,548,577,577,307,276,306,291,516,560,259,259,
    -250,-2107,-2507,-2764,-2909,-2974,-3007,-3023,1041,1041,1040,1040,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-767,-1052,-1213,-1277,-1358,-1405,-1469,-1535,-1550,-1582,-1614,-1647,-1662,-1694,-1726,-1759,-1774,-1807,-1822,-1854,-1886,1565,-1919,-1935,-1951,-1967,1731,1730,1580,1717,-1983,1729,1564,-1999,1548,-2015,-2031,1715,1595,-2047,1714,-2063,1610,-2079,1609,-2095,1323,1323,1457,1457,1307,1307,1712,1547,1641,1700,1699,1594,1685,1625,1442,1442,1322,1322,-780,-973,-910,1279,1278,1277,1262,1276,1261,1275,1215,1260,1229,-959,974,974,989,989,-943,735,478,478,495,463,506,414,-1039,1003,958,1017,927,942,987,957,431,476,1272,1167,1228,-1183,1256,-1199,895,895,941,941,1242,1227,1212,1135,1014,1014,490,489,503,487,910,1013,985,925,863,894,970,955,1012,847,-1343,831,755,755,984,909,428,366,754,559,-1391,752,486,457,924,997,698,698,983,893,740,740,908,877,739,739,667,667,953,938,497,287,271,271,683,606,590,712,726,574,302,302,738,736,481,286,526,725,605,711,636,724,696,651,589,681,666,710,364,467,573,695,466,466,301,465,379,379,709,604,665,679,316,316,634,633,436,436,464,269,424,394,452,332,438,363,347,408,393,448,331,422,362,407,392,421,346,406,391,376,375,359,1441,1306,-2367,1290,-2383,1337,-2399,-2415,1426,1321,-2431,1411,1336,-2447,-2463,-2479,1169,1169,1049,1049,1424,1289,1412,1352,1319,-2495,1154,1154,1064,1064,1153,1153,416,390,360,404,403,389,344,374,373,343,358,372,327,357,342,311,356,326,1395,1394,1137,1137,1047,1047,1365,1392,1287,1379,1334,1364,1349,1378,1318,1363,792,792,792,792,1152,1152,1032,1032,1121,1121,1046,1046,1120,1120,1030,1030,-2895,1106,1061,1104,849,849,789,789,1091,1076,1029,1090,1060,1075,833,833,309,324,532,532,832,772,818,803,561,561,531,560,515,546,289,274,288,258,
    -250,-1179,-1579,-1836,-1996,-2124,-2253,-2333,-2413,-2477,-2542,-2574,-2607,-2622,-2655,1314,1313,1298,1312,1282,785,785,785,785,1040,1040,1025,1025,768,768,768,768,-766,-798,-830,-862,-895,-911,-927,-943,-959,-975,-991,-1007,-1023,-1039,-1055,-1070,1724,1647,-1103,-1119,1631,1767,1662,1738,1708,1723,-1135,1780,1615,1779,1599,1677,1646,1778,1583,-1151,1777,1567,1737,1692,1765,1722,1707,1630,1751,1661,1764,1614,1736,1676,1763,1750,1645,1598,1721,1691,1762,1706,1582,1761,1566,-1167,1749,1629,767,766,751,765,494,494,735,764,719,749,734,763,447,447,748,718,477,506,431,491,446,476,461,505,415,430,475,445,504,399,460,489,414,503,383,474,429,459,502,502,746,752,488,398,501,473,413,472,486,271,480,270,-1439,-1455,1357,-1471,-1487,-1503,1341,1325,-1519,1489,1463,1403,1309,-1535,1372,1448,1418,1476,1356,1462,1387,-1551,1475,1340,1447,1402,1386,-1567,1068,1068,1474,1461,455,380,468,440,395,425,410,454,364,467,466,464,453,269,409,448,268,432,1371,1473,1432,1417,1308,1460,1355,1446,1459,1431,1083,1083,1401,1416,1458,1445,1067,1067,1370,1457,1051,1051,1291,1430,1385,1444,1354,1415,1400,1443,1082,1082,1173,1113,1186,1066,1185,1050,-1967,1158,1128,1172,1097,1171,1081,-1983,1157,1112,416,266,375,400,1170,1142,1127,1065,793,793,1169,1033,1156,1096,1141,1111,1155,1080,1126,1140,898,898,808,808,897,897,792,792,1095,1152,1032,1125,1110,1139,1079,1124,882,807,838,881,853,791,-2319,867,368,263,822,852,837,866,806,865,-2399,851,352,262,534,534,821,836,594,594,549,549,593,593,533,533,848,773,579,579,564,578,548,563,276,276,577,576,306,291,516,560,305,305,275,259,
    -251,-892,-2058,-2620,-2828,-2957,-3023,-3039,1041,1041,1040,1040,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-511,-527,-543,-559,1530,-575,-591,1528,1527,1407,1526,1391,1023,1023,1023,1023,1525,1375,1268,1268,1103,1103,1087,1087,1039,1039,1523,-604,815,815,815,815,510,495,509,479,508,463,507,447,431,505,415,399,-734,-782,1262,-815,1259,1244,-831,1258,1228,-847,-863,1196,-879,1253,987,987,748,-767,493,493,462,477,414,414,686,669,478,446,461,445,474,429,487,458,412,471,1266,1264,1009,1009,799,799,-1019,-1276,-1452,-1581,-1677,-1757,-1821,-1886,-1933,-1997,1257,1257,1483,1468,1512,1422,1497,1406,1467,1496,1421,1510,1134,1134,1225,1225,1466,1451,1374,1405,1252,1252,1358,1480,1164,1164,1251,1251,1238,1238,1389,1465,-1407,1054,1101,-1423,1207,-1439,830,830,1248,1038,1237,1117,1223,1148,1236,1208,411,426,395,410,379,269,1193,1222,1132,1235,1221,1116,976,976,1192,1162,1177,1220,1131,1191,963,963,-1647,961,780,-1663,558,558,994,993,437,408,393,407,829,978,813,797,947,-1743,721,721,377,392,844,950,828,890,706,706,812,859,796,960,948,843,934,874,571,571,-1919,690,555,689,421,346,539,539,944,779,918,873,932,842,903,888,570,570,931,917,674,674,-2575,1562,-2591,1609,-2607,1654,1322,1322,1441,1441,1696,1546,1683,1593,1669,1624,1426,1426,1321,1321,1639,1680,1425,1425,1305,1305,1545,1668,1608,1623,1667,1592,1638,1666,1320,1320,1652,1607,1409,1409,1304,1304,1288,1288,1664,1637,1395,1395,1335,1335,1622,1636,1394,1394,1319,1319,1606,1621,1392,1392,1137,1137,1137,1137,345,390,360,375,404,373,1047,-2751,-2767,-2783,1062,1121,1046,-2799,1077,-2815,1106,1061,789,789,1105,1104,263,355,310,340,325,354,352,262,339,324,1091,1076,1029,1090,1060,1075,833,833,788,788,1088,1028,818,818,803,803,561,561,531,531,816,771,546,546,289,274,288,258,
    -253,-317,-381,-446,-478,-509,1279,1279,-811,-1179,-1451,-1756,-1900,-2028,-2189,-2253,-2333,-2414,-2445,-2511,-2526,1313,1298,-2559,1041,1041,1040,1040,1025,1025,1024,1024,1022,1007,1021,991,1020,975,1019,959,687,687,1018,1017,671,671,655,655,1016,1015,639,639,758,758,623,623,757,607,756,591,755,575,754,559,543,543,1009,783,-575,-621,-685,-749,496,-590,750,749,734,748,974,989,1003,958,988,973,1002,942,987,957,972,1001,926,986,941,971,956,1000,910,985,925,999,894,970,-1071,-1087,-1102,1390,-1135,1436,1509,1451,1374,-1151,1405,1358,1480,1420,-1167,1507,1494,1389,1342,1465,1435,1450,1326,1505,1310,1493,1373,1479,1404,1492,1464,1419,428,443,472,397,736,526,464,464,486,457,442,471,484,482,1357,1449,1434,1478,1388,1491,1341,1490,1325,1489,1463,1403,1309,1477,1372,1448,1418,1433,1476,1356,1462,1387,-1439,1475,1340,1447,1402,1474,1324,1461,1371,1473,269,448,1432,1417,1308,1460,-1711,1459,-1727,1441,1099,1099,1446,1386,1431,1401,-1743,1289,1083,1083,1160,1160,1458,1445,1067,1067,1370,1457,1307,1430,1129,1129,1098,1098,268,432,267,416,266,400,-1887,1144,1187,1082,1173,1113,1186,1066,1050,1158,1128,1143,1172,1097,1171,1081,420,391,1157,1112,1170,1142,1127,1065,1169,1049,1156,1096,1141,1111,1155,1080,1126,1154,1064,1153,1140,1095,1048,-2159,1125,1110,1137,-2175,823,823,1139,1138,807,807,384,264,368,263,868,838,853,791,867,822,852,837,866,806,865,790,-2319,851,821,836,352,262,850,805,849,-2399,533,533,835,820,336,261,578,548,563,577,532,532,832,772,562,562,547,547,305,275,560,515,290,290,288,258};
static const u8 tab32[]={130,162,193,209,44,28,76,140,9,9,9,9,9,9,9,9,190,254,222,238,126,94,157,157,109,61,173,205}; static const u8 tab33[]={252,236,220,204,188,172,156,140,124,108,92,76,60,44,28,12};
static const i16 tabindex[2*16]={0,32,64,98,0,132,180,218,292,364,426,538,648,746,0,1126,1460,1460,1460,1460,1460,1460,1460,1460,1842,1842,1842,1842,1842,1842,1842,1842}; static const u8 g_linbits[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,6,8,10,13,4,5,6,7,8,9,11,13};
static const float g_sec[24]={10.19000816f,0.50060302f,0.50241929f,3.40760851f,0.50547093f,0.52249861f,2.05778098f,0.51544732f,0.56694406f,1.48416460f,0.53104258f,0.64682180f,1.16943991f,0.55310392f,0.78815460f,0.97256821f,0.58293498f,1.06067765f,0.83934963f,0.62250412f,1.72244716f,0.74453628f,0.67480832f,5.10114861f};
static const float g_win[]={ -1,26,-31,208,218,401,-519,2063,2000,4788,-5517,7134,5959,35640,-39336,74992,-1,24,-35,202,222,347,-581,2080,1952,4425,-5879,7640,5288,33791,-41176,74856,-1,21,-38,196,225,294,-645,2087,1893,4063,-6237,8092,4561,31947,-43006,74630,-1,19,-41,190,227,244,-711,2085,1822,3705,-6589,8492,3776,30112,-44821,74313,-1,17,-45,183,228,197,-779,2075,1739,3351,-6935,8840,2935,28289,-46617,73908,-1,16,-49,176,228,153,-848,2057,1644,3004,-7271,9139,2037,26482,-48390,73415,-2,14,-53,169,227,111,-919,2032,1535,2663,-7597,9389,1082,24694,-50137,72835,-2,13,-58,161,224,72,-991,2001,1414,2330,-7910,9592,70,22929,-51853,72169,-2,11,-63,154,221,36,-1064,1962,1280,2006,-8209,9750,-998,21189,-53534,71420,-2,10,-68,147,215,2,-1137,1919,1131,1692,-8491,9863,-2122,19478,-55178,70590,-3,9,-73,139,208,-29,-1210,1870,970,1388,-8755,9935,-3300,17799,-56778,69679,-3,8,-79,132,200,-57,-1283,1817,794,1095,-8998,9966,-4533,16155,-58333,68692,-4,7,-85,125,189,-83,-1356,1759,605,814,-9219,9959,-5818,14548,-59838,67629,-4,7,-91,117,177,-106,-1428,1698,402,545,-9416,9916,-7154,12980,-61289,66494,-5,6,-97,111,163,-127,-1498,1634,185,288,-9585,9838,-8540,11455,-62684,65290};
typedef struct { int frame_bytes,channels,sample_rate,layer,bitrate_kbps; } mp3dec_frame_info;
typedef struct { const u8 *buf; int pos,limit; } mp3_bs;
typedef struct { const u8 *sfbtab; u16 part_23_length,big_values,scalefac_compress; u8 global_gain,block_type,mixed_block_flag,n_long_sfb,n_short_sfb,table_select[3],region_count[3],subblock_gain[3],preflag,scalefac_scale,count1_table,scfsi; } mp3L3_gr_info;
typedef struct { mp3_bs bs; u8 maindata[511 + 2304]; mp3L3_gr_info gr_info[4]; float grbuf[2][576],scf[40],syn[18+15][2*32]; u8 ist_pos[2][39]; } mp3dec_scratch;
typedef struct { float mdct_overlap[2][9*32], qmf_state[15*2*32]; int reserv; u8 header[4],reserv_buf[511]; mp3dec_scratch scratch; } mp3dec;
typedef struct { mp3dec decoder; u32 channels,sampleRate,mp3FChan,mp3FrameSampleRate,pcmFConsInMP3F,pcmFRemInMP3F,delayInPCMFrames,paddingInPCMFrames; void *pUserData; u8 pcmFrames[sizeof(float) * (1152 * 2)]; u64 currentPCMFrame,streamCursor,streamLength,streamStartOffset,totalPCMFrameCount; bool atEnd; size_t dataSize,dataCapacity,dataConsumed; u8 *pData; } mp3;
static u32 mp3_bs_get_bits(mp3_bs *bs, int n) { u32 next,cache=0, s=bs->pos&7; int shl=n+s; const u8 *p=bs->buf+(bs->pos>>3); if ((bs->pos+=n)>bs->limit) {return 0;} next=*p++&(255>>s); while ((shl-=8)>0) { cache|=next<<shl; next=*p++; } return cache|(next>>-shl); }
static int mp3_hdr_valid(const u8 *h) { int bitrate_idx=MP3_HDR_GET_BITRATE(h); return h[0]==0xff && ((h[1]&0xF0)==0xf0||(h[1]&0xFE)==0xe2) && (MP3_HDR_GET_LAYER(h)!=0) && (bitrate_idx!=0) && (bitrate_idx!=15) && (MP3_HDR_GET_SAMPLE_RATE(h)!=3); }
static int mp3_hdr_compare(const u8 *h1, const u8 *h2) { return mp3_hdr_valid(h2) && ((h1[1]^h2[1])&0xFE)==0 && ((h1[2]^h2[2])&0x0C)==0; }
static unsigned mp3_hdr_bitrate_kbps(const u8 *h) { return 2*g_halfrate[!!MP3_HDR_TEST_MPEG1(h)][(((h[1]) >> 1) & 3)-1/*layer*/][((h[2]) >> 4)/*bitrate*/]; }
static unsigned mp3_hdr_sample_rate_hz(const u8 *h) { static const unsigned g_hz[3]={44100,48000,32000}; return g_hz[MP3_HDR_GET_SAMPLE_RATE(h)]>>(int)!MP3_HDR_TEST_MPEG1(h)>>(int)!MP3_HDR_TEST_NOT_MPEG25(h); }
static unsigned mp3_hdr_frame_samples(const u8 *h) { return ((h[1]&6) == 6) ? 384 : (1152>>(int)MP3_HDR_IS_FRAME_576(h)); }
static int mp3_hdr_frame_bytes(const u8 *h) { int fb=mp3_hdr_frame_samples(h)*mp3_hdr_bitrate_kbps(h)*125/mp3_hdr_sample_rate_hz(h); if (MP3_HDR_IS_LAYER_1(h)) {fb&=~3;} return fb; }
static int mp3_hdr_padding(const u8 *h) { return MP3_HDR_TEST_PADDING(h)?(MP3_HDR_IS_LAYER_1(h)?4:1):0; }
void InitSCFTables() { for (int i=0;i<23;++i) g_scf_long[0][i] = g_scf_long[1][i] = g_scf_long[2][i] = g_sfc_long_024[i]; }
static int mp3L3_read_side_info(mp3_bs *bs, mp3L3_gr_info *gr, const u8 *hdr) {
    unsigned tables,scfsi=0; int main_data_begin,part_23_sum = 0, gr_count=MP3_HDR_IS_MONO(hdr) ? 1 : 2, sr_idx=MP3_HDR_GET_SAMPLE_RATEHDR(hdr); sr_idx-=(sr_idx!=0);
    if (MP3_HDR_TEST_MPEG1(hdr)) { gr_count*=2; main_data_begin=mp3_bs_get_bits(bs,9); scfsi=mp3_bs_get_bits(bs,7+gr_count); }
    else main_data_begin = mp3_bs_get_bits(bs,8+gr_count)>>gr_count;
    do {
        if (MP3_HDR_IS_MONO(hdr)) scfsi<<=4;
        gr->part_23_length=(u16)mp3_bs_get_bits(bs,12); part_23_sum+=gr->part_23_length; gr->big_values=(u16)mp3_bs_get_bits(bs,9); if (gr->big_values>288) return -1;
        gr->global_gain=(u8)mp3_bs_get_bits(bs,8); gr->scalefac_compress=(u16)mp3_bs_get_bits(bs,MP3_HDR_TEST_MPEG1(hdr)?4:9); gr->sfbtab=g_scf_long[sr_idx]; gr->n_long_sfb=22; gr->n_short_sfb=0;
        if (mp3_bs_get_bits(bs,1)) {
            gr->block_type = (u8)mp3_bs_get_bits(bs,2); if (!gr->block_type) return -1;
            gr->mixed_block_flag = (u8)mp3_bs_get_bits(bs,1); gr->region_count[0] = 7; gr->region_count[1] = 255;
            if (gr->block_type==2) {
                scfsi&=0x0F0F;
                if (!gr->mixed_block_flag) { gr->region_count[0] = 8; gr->sfbtab = g_scf_short[sr_idx]; gr->n_long_sfb = 0; gr->n_short_sfb = 39; }
                else                       { gr->sfbtab = g_scf_mixed[sr_idx]; gr->n_long_sfb=MP3_HDR_TEST_MPEG1(hdr) ? 8 : 6; gr->n_short_sfb = 30; }
            }
            tables=mp3_bs_get_bits(bs,10)<<5;
            gr->subblock_gain[0]=(u8)mp3_bs_get_bits(bs,3); gr->subblock_gain[1]=(u8)mp3_bs_get_bits(bs,3); gr->subblock_gain[2]=(u8)mp3_bs_get_bits(bs,3);
        } else { gr->block_type=0; gr->mixed_block_flag=0; tables=mp3_bs_get_bits(bs,15); gr->region_count[0]=(u8)mp3_bs_get_bits(bs,4); gr->region_count[1]=(u8)mp3_bs_get_bits(bs,3); gr->region_count[2]=255; }
        gr->table_select[0]=(u8)(tables>>10); gr->table_select[1]=(u8)((tables>>5)&31); gr->table_select[2]=(u8)((tables)&31);
        gr->preflag=(u8)(MP3_HDR_TEST_MPEG1(hdr) ? mp3_bs_get_bits(bs,1) : (gr->scalefac_compress>=500));
        gr->scalefac_scale=(u8)mp3_bs_get_bits(bs,1); gr->count1_table=(u8)mp3_bs_get_bits(bs,1); gr->scfsi=(u8)((scfsi>>12)&15);
        scfsi <<= 4; gr++;
    } while(--gr_count);
    if (part_23_sum+bs->pos > bs->limit+main_data_begin*8) return -1;
    return main_data_begin;
}

static void mp3L3_read_scalefactors(u8 *scf, u8 *ist_pos, const u8 *scf_size, const u8 *scf_count, mp3_bs *bs, int scfsi) {
    for (int i=0; i<4&&scf_count[i]; i++,scfsi*=2) {
        int cnt = scf_count[i];
        if (scfsi & 8) {mcpy(scf,ist_pos,cnt);} else { int bits=scf_size[i]; if(!bits){mset(scf,0,cnt); mset(ist_pos,0,cnt);} else {int max_scf=(scfsi<0)?((1<<bits)-1):-1; for (int k=0;k<cnt;k++) {int s=mp3_bs_get_bits(bs,bits); ist_pos[k]=(u8)(s==max_scf?-1:s); scf[k]=(u8)s;} } }
        ist_pos+=cnt; scf+=cnt;
    }
    scf[0]=scf[1]=scf[2]=0;
}

static float mp3L3_ldexp_q2(float y, int exp_q2) { static const float g_expfrac[4]={9.31322575e-10f,7.83145814e-10f,6.58544508e-10f,5.53767716e-10f}; int e; do { e=vmin(30*4,exp_q2); y*=g_expfrac[e&3]*(1<<30>>(e>>2)); } while ((exp_q2-=e)>0); return y; }
#define MP3_MAX_SCFI (((255+-1*4-210)+3)&~3)
static void mp3L3_decode_scalefactors(const u8 *hdr, u8 *ist_pos, mp3_bs *bs, const mp3L3_gr_info *gr, float *scf, int ch) {
    const u8 *scf_partition=g_scf_partitions[!!gr->n_short_sfb+!gr->n_long_sfb];
    u8 scf_size[4],iscf[40]; int i,scf_shift=gr->scalefac_scale+1,gain_exp,scfsi=gr->scfsi; float gain;
    if (MP3_HDR_TEST_MPEG1(hdr)) { static const u8 g_scfc_decode[16]={0,1,2,3,12,5,6,7,9,10,11,13,14,15,18,19}; int part=g_scfc_decode[gr->scalefac_compress]; scf_size[1]=scf_size[0]=(u8)(part>>2); scf_size[3]=scf_size[2]=(u8)(part&3); }
    else {
        static const u8 g_mod[6*4]={5,5,4,4,5,5,4,1,4,3,1,1,5,6,6,1,4,4,4,1,4,3,1,1};
        int k,modprod,sfc,ist=MP3_HDR_TEST_I_STEREO(hdr)&&ch;
        sfc=gr->scalefac_compress>>ist;
        for (k=ist*3*4; sfc>=0; sfc-=modprod,k+=4) { for (modprod=1,i=3;i>=0;i--) {scf_size[i]=(u8)(sfc/modprod%g_mod[k+i]); modprod*=g_mod[k+i];} }
        scf_partition+=k; scfsi=-16;
    }
    mp3L3_read_scalefactors(iscf,ist_pos,scf_size,scf_partition,bs,scfsi);
    if (gr->n_short_sfb) {
        int sh=3-scf_shift;
        for (i=0;i<gr->n_short_sfb;i+=3) { iscf[gr->n_long_sfb+i+0]=(u8)(iscf[gr->n_long_sfb+i+0]+(gr->subblock_gain[0]<<sh)); iscf[gr->n_long_sfb+i+1]=(u8)(iscf[gr->n_long_sfb+i+1]+(gr->subblock_gain[1]<<sh)); iscf[gr->n_long_sfb+i+2]=(u8)(iscf[gr->n_long_sfb+i+2]+(gr->subblock_gain[2]<<sh)); }
    } else if (gr->preflag) { static const u8 g_preamp[10]={1,1,1,1,2,2,3,3,3,2}; for (i=0;i<10;i++) {iscf[11+i]=(u8)(iscf[11+i]+g_preamp[i]);} }
    gain_exp=gr->global_gain+-1*4-210-(MP3_HDR_IS_MS_STEREO(hdr)?2:0);
    gain=mp3L3_ldexp_q2(1<<(MP3_MAX_SCFI/4),MP3_MAX_SCFI-gain_exp);
    for (i=0;i<(int)(gr->n_long_sfb+gr->n_short_sfb);i++) scf[i]=mp3L3_ldexp_q2(gain,iscf[i]<<scf_shift);
}

static float mp3L3_pow_43(int x) { if(x<129){return g_mp3_pow43[16+x];} int mult=256; if(x<1024){mult=16; x<<=3;} int sign=2*x&64; float frac=(float)((x&63)-sign)/((x&~63)+sign); return g_mp3_pow43[16+((x+sign)>>6)]*(1.f+frac*((4.f/3)+frac*(2.f/9)))*mult; }
static void mp3L3_huffman(float *dst, mp3_bs *bs, const mp3L3_gr_info *gr_info, const float *scf, int layer3gr_limit) {
    #define MP3_FLUSH_BITS(n) { bs_cache<<=(n); bs_sh+=(n); }
    float one=0.0f; int ireg=0,big_val_cnt=gr_info->big_values; const u8 *sfb=gr_info->sfbtab; const u8 *bs_next_ptr=bs->buf+bs->pos/8;
    u32 bs_cache=(((bs_next_ptr[0]*256u+bs_next_ptr[1])*256u+bs_next_ptr[2])*256u+bs_next_ptr[3])<<(bs->pos&7); int pairs_to_decode,np,bs_sh=(bs->pos&7)-8; bs_next_ptr+=4;
    while (big_val_cnt>0) {
        int tab_num=gr_info->table_select[ireg], sfb_cnt=gr_info->region_count[ireg++]; const i16 *codebook=tabs+tabindex[tab_num]; int linbits=g_linbits[tab_num];
        if (linbits) {
            do {
                np=*sfb++/2; pairs_to_decode=vmin(big_val_cnt,np); one=*scf++;
                do {
                    int j,w=5,leaf=codebook[(bs_cache>>(32-w))];
                    while (leaf<0){MP3_FLUSH_BITS(w);w=leaf&7;leaf=codebook[(bs_cache>>(32-w))-(leaf>>3)];}
                    MP3_FLUSH_BITS(leaf>>8);
                    for (j=0;j<2;j++,dst++,leaf>>=4){
                        int lsb=leaf&0x0F;
                        if (lsb==15) { lsb += (bs_cache>>(32-(linbits))); MP3_FLUSH_BITS(linbits); while(bs_sh>=0){bs_cache|=(u32)*bs_next_ptr++<<bs_sh;bs_sh-=8;}; *dst= one * mp3L3_pow_43(lsb) * ((i32)bs_cache < 0 ? -1 : 1); }
                        else *dst=g_mp3_pow43[16+lsb-16*(bs_cache>>31)]*one;
                        MP3_FLUSH_BITS(lsb?1:0);
                    }
                    while(bs_sh>=0){bs_cache|=(u32)*bs_next_ptr++<<bs_sh;bs_sh-=8;};
                } while(--pairs_to_decode);
            } while((big_val_cnt-=np)>0&&--sfb_cnt>=0);
        } else {
            do {
                np=*sfb++/2; pairs_to_decode=vmin(big_val_cnt,np); one=*scf++;
                do {
                    int j,w=5,leaf=codebook[(bs_cache>>(32-w))];
                    while (leaf<0){MP3_FLUSH_BITS(w);w=leaf&7;leaf=codebook[(bs_cache>>(32-w))-(leaf>>3)];}
                    MP3_FLUSH_BITS(leaf>>8);
                    for (j=0;j<2;j++,dst++,leaf>>=4) { int lsb=leaf&0x0F; *dst=g_mp3_pow43[16+lsb-16*(bs_cache>>31)]*one; MP3_FLUSH_BITS(lsb?1:0); }
                    while(bs_sh>=0){bs_cache|=(u32)*bs_next_ptr++<<bs_sh;bs_sh-=8;};
                } while(--pairs_to_decode);
            } while((big_val_cnt-=np)>0&&--sfb_cnt>=0);
        }
    }
    for (np=1-big_val_cnt;;dst+=4) {
        const u8 *codebook_count1=(gr_info->count1_table)?tab33:tab32;
        int leaf=codebook_count1[(bs_cache>>28)];
        if (!(leaf&8)) leaf=codebook_count1[(leaf>>3)+(bs_cache<<4>>(32-(leaf&3)))];
        MP3_FLUSH_BITS(leaf&7);
        if (((bs_next_ptr-bs->buf)*8-24+bs_sh)>layer3gr_limit) break;
        if(!--np) { np=*sfb++/2; if(!np) {break;} one=*scf++; }; if(leaf&(128>>0)){dst[0]=((i32)bs_cache<0)?-one:one;MP3_FLUSH_BITS(1)} if(leaf&(128>>1)){dst[1]=((i32)bs_cache<0)?-one:one;MP3_FLUSH_BITS(1)}
        if(!--np) { np=*sfb++/2; if(!np) {break;} one=*scf++; }; if(leaf&(128>>2)){dst[2]=((i32)bs_cache<0)?-one:one;MP3_FLUSH_BITS(1)} if(leaf&(128>>3)){dst[3]=((i32)bs_cache<0)?-one:one;MP3_FLUSH_BITS(1)}
        while(bs_sh>=0){bs_cache|=(u32)*bs_next_ptr++<<bs_sh;bs_sh-=8;};
    }
    bs->pos=layer3gr_limit;
}

static void mp3L3_midside_stereo(float *l, int n) { int i=0; float *r=l+576; for (; i<n; i++) { float a=l[i],b=r[i]; l[i]=a+b; r[i]=a-b; } }
static void mp3L3_intensity_stereo_band(float *l, int n, float kl, float kr) { int i; for(i=0;i<n;i++){l[i+576]=l[i]*kr; l[i]=l[i]*kl;} }
static void mp3L3_stereo_top_band(const float *r, const u8 *sfb, int nbands, int max_band[3]) { int i,k; max_band[0]=max_band[1]=max_band[2]=-1; for (i=0;i<nbands;i++){for(k=0;k<sfb[i];k+=2){if(r[k]!=0||r[k+1]!=0){max_band[i%3]=i;break;}}r+=sfb[i];} }
static void mp3L3_stereo_process(float *left, const u8 *ist_pos, const u8 *sfb, const u8 *hdr, int max_band[3], int mpeg2_sh) {
    static const float g_pan[7*2]={0,1,0.21132487f,0.78867513f,0.36602540f,0.63397460f,0.5f,0.5f,0.63397460f,0.36602540f,0.78867513f,0.21132487f,1,0};
    unsigned i,max_pos=MP3_HDR_TEST_MPEG1(hdr)?7:64;
    for (i=0;sfb[i];i++){
        unsigned ipos=ist_pos[i];
        if ((int)i>max_band[i%3]&&ipos<max_pos){
            float kl,kr,s=MP3_HDR_TEST_MS_STEREO(hdr)?1.41421356f:1;
            if(MP3_HDR_TEST_MPEG1(hdr)){kl=g_pan[2*ipos];kr=g_pan[2*ipos+1];} else {kl=1;kr=mp3L3_ldexp_q2(1,(ipos+1)>>1<<mpeg2_sh);if(ipos&1){kl=kr;kr=1;}}
            mp3L3_intensity_stereo_band(left,sfb[i],kl*s,kr*s);
        } else if (MP3_HDR_TEST_MS_STEREO(hdr)) mp3L3_midside_stereo(left,sfb[i]);
        left+=sfb[i];
    }
}
static void mp3L3_intensity_stereo(float *left, u8 *ist_pos, const mp3L3_gr_info *gr, const u8 *hdr) {
    int max_band[3],n_sfb=gr->n_long_sfb+gr->n_short_sfb,i,max_blocks=gr->n_short_sfb?3:1;
    mp3L3_stereo_top_band(left+576,gr->sfbtab,n_sfb,max_band);
    if (gr->n_long_sfb) max_band[0]=max_band[1]=max_band[2]=vmax(vmax(max_band[0],max_band[1]),max_band[2]);
    for (i=0;i<max_blocks;i++){ int default_pos = MP3_HDR_TEST_MPEG1(hdr) ? 3 : 0, itop = n_sfb-max_blocks + i, prev = itop - max_blocks; ist_pos[itop] = (u8)(max_band[i] >= prev ? default_pos : ist_pos[prev]); }
    mp3L3_stereo_process(left,ist_pos,gr->sfbtab,hdr,max_band,gr[1].scalefac_compress&1);
}

static void mp3L3_reorder(float *grbuf, float *scratch, const u8 *sfb) { int i,len; float *src=grbuf,*dst=scratch; for(;0!=(len=*sfb);sfb+=3,src+=2*len){for(i=0;i<len;i++,src++){*dst++=src[0*len];*dst++=src[1*len];*dst++=src[2*len];}} mcpy(grbuf,scratch,(dst-scratch)*sizeof(float)); }
static void mp3L3_antialias(float *grbuf, int nbands) {
    static const float g_aa[2][8]={{0.85749293f,0.88174200f,0.94962865f,0.98331459f,0.99551782f,0.99916056f,0.99989920f,0.99999316f},{0.51449576f,0.47173197f,0.31337745f,0.18191320f,0.09457419f,0.04096558f,0.01419856f,0.00369997f}};
    for(;nbands>0;nbands--,grbuf+=18){ int i=0; for(;i<8;i++){float u=grbuf[18+i],d=grbuf[17-i];grbuf[18+i]=u*g_aa[0][i]-d*g_aa[1][i];grbuf[17-i]=u*g_aa[1][i]+d*g_aa[0][i];} }
}

static void mp3L3_dct3_9(float *y) { float s1,s3,s5,s7,t0,t2,t4,s0=y[0],s2=y[2],s4=y[4],s6=y[6],s8=y[8]; t0=s0+s6*0.5f; s0-=s6; t4=(s4+s2)*0.93969262f; t2=(s8+s2)*0.76604444f; s6=(s4-s8)*0.17364818f; s4+=s8-s2; s2=s0-s4*0.5f; y[4]=s4+s0; s8=t0-t2+s6; s0=t0-t4+t2; s4=t0+t4-s6; s1=y[1]; s3=y[3]; s5=y[5]; s7=y[7]; s3*=0.86602540f; t0=(s5+s1)*0.98480775f; t4=(s5-s7)*0.34202014f; t2=(s1+s7)*0.64278761f; s1=(s1-s5-s7)*0.86602540f; s5=t0-s3-t2; s7=t4-s3-t0; s3=t4+s3-t2; y[0]=s4-s7; y[1]=s2+s1; y[2]=s0-s3; y[3]=s8+s5; y[5]=s8-s5; y[6]=s0+s3; y[7]=s2-s1; y[8]=s4+s7; }
static void mp3L3_imdct36(float *grbuf, float *overlap, const float *win, int nbands) {
    int i,j;
    static const float g_twid9[18]={0.73727734f,0.79335334f,0.84339145f,0.88701083f,0.92387953f,0.95371695f,0.97629601f,0.99144486f,0.99904822f,0.67559021f,0.60876143f,0.53729961f,0.46174861f,0.38268343f,0.30070580f,0.21643961f,0.13052619f,0.04361938f};
    for (j=0;j<nbands;j++,grbuf+=18,overlap+=9){
        float co[9],si[9];
        co[0]=-grbuf[0]; si[0]=grbuf[17];
        for(i=0;i<4;i++){si[8-2*i]=grbuf[4*i+1]-grbuf[4*i+2];co[1+2*i]=grbuf[4*i+1]+grbuf[4*i+2];si[7-2*i]=grbuf[4*i+4]-grbuf[4*i+3];co[2+2*i]=-(grbuf[4*i+3]+grbuf[4*i+4]);}
        mp3L3_dct3_9(co); mp3L3_dct3_9(si);
        si[1]=-si[1];si[3]=-si[3];si[5]=-si[5];si[7]=-si[7];
        i=0;
        for(;i<9;i++){float ovl=overlap[i],sum=co[i]*g_twid9[9+i]+si[i]*g_twid9[i];overlap[i]=co[i]*g_twid9[i]-si[i]*g_twid9[9+i];grbuf[i]=ovl*win[i]-sum*win[9+i];grbuf[17-i]=ovl*win[9+i]+sum*win[i];}
    }
}

static void mp3L3_idct3(float x0,float x1,float x2,float *dst){float m1=x1*0.86602540f,a1=x0-x2*0.5f;dst[1]=x0+x2;dst[0]=a1+m1;dst[2]=a1-m1;}
static void imdct12(float *x,float *dst,float *overlap){
    static const float g_twid3[6]={0.79335334f,0.92387953f,0.99144486f,0.60876143f,0.38268343f,0.13052619f};
    float co[3],si[3]; int i; mp3L3_idct3(-x[0],x[6]+x[3],x[12]+x[9],co); mp3L3_idct3(x[15],x[12]-x[9],x[6]-x[3],si);
    si[1]=-si[1];
    for(i=0;i<3;i++){float ovl=overlap[i],sum=co[i]*g_twid3[3+i]+si[i]*g_twid3[i];overlap[i]=co[i]*g_twid3[i]-si[i]*g_twid3[3+i];dst[i]=ovl*g_twid3[2-i]-sum*g_twid3[5-i];dst[5-i]=ovl*g_twid3[5-i]+sum*g_twid3[2-i];}
}

static void mp3L3_imdct_short(float *grbuf,float *overlap,int nbands){ for(;nbands>0;nbands--,overlap+=9,grbuf+=18){float tmp[18]; mcpy(tmp,grbuf,sizeof(tmp)); mcpy(grbuf,overlap,6*sizeof(float)); imdct12(tmp,grbuf+6,overlap+6); imdct12(tmp+1,grbuf+12,overlap+6); imdct12(tmp+2,overlap,overlap+6);} }
static void mp3L3_change_sign(float *grbuf){int b,i;for(b=0,grbuf+=18;b<32;b+=2,grbuf+=36)for(i=1;i<18;i+=2)grbuf[i]=-grbuf[i];}
static const float g_mdct_window[2][18]={{0.99904822f,0.99144486f,0.97629601f,0.95371695f,0.92387953f,0.88701083f,0.84339145f,0.79335334f,0.73727734f,0.04361938f,0.13052619f,0.21643961f,0.30070580f,0.38268343f,0.46174861f,0.53729961f,0.60876143f,0.67559021f},{1,1,1,1,1,1,0.99144486f,0.92387953f,0.79335334f,0,0,0,0,0,0,0.13052619f,0.38268343f,0.60876143f}};
static void mp3L3_imdct_gr(float *grbuf,float *overlap,unsigned block_type,unsigned n_long_bands){ if (n_long_bands){mp3L3_imdct36(grbuf,overlap,g_mdct_window[0],n_long_bands);grbuf+=18*n_long_bands;overlap+=9*n_long_bands;} if (block_type==2) {mp3L3_imdct_short(grbuf,overlap,32-n_long_bands);} else {mp3L3_imdct36(grbuf,overlap,g_mdct_window[block_type==3],32-n_long_bands);} }
static void mp3L3_save_reservoir(mp3dec *h, mp3dec_scratch *s) { int pos=(s->bs.pos+7)/8u,remains=s->bs.limit/8u-pos; if (remains>511){pos+=remains-511;remains=511;} if (remains>0) {mmov(h->reserv_buf,s->maindata+pos,remains);} h->reserv=remains; }
static int mp3L3_restore_reservoir(mp3dec *h, mp3_bs *bs, mp3dec_scratch *s, int main_data_begin) { int frame_bytes=(bs->limit-bs->pos)/8,bytes_have=vmin(h->reserv,main_data_begin); mcpy(s->maindata,h->reserv_buf+vmax(0,h->reserv-main_data_begin),vmin(h->reserv,main_data_begin)); mcpy(s->maindata+bytes_have,bs->buf+bs->pos/8,frame_bytes); s->bs.buf=s->maindata; s->bs.pos=0; s->bs.limit=(bytes_have+frame_bytes) * 8; return h->reserv>=main_data_begin; }
static void mp3L3_decode(mp3dec *h, mp3dec_scratch *s, mp3L3_gr_info *gr_info, int nch){
    int ch; for(ch=0;ch<nch;ch++) { int limit=s->bs.pos+gr_info[ch].part_23_length; mp3L3_decode_scalefactors(h->header,s->ist_pos[ch],&s->bs,gr_info+ch,s->scf,ch); mp3L3_huffman(s->grbuf[ch],&s->bs,gr_info+ch,s->scf,limit); }
    if (MP3_HDR_TEST_I_STEREO(h->header)) mp3L3_intensity_stereo(s->grbuf[0],s->ist_pos[1],gr_info,h->header);
    else if (MP3_HDR_IS_MS_STEREO(h->header)) mp3L3_midside_stereo(s->grbuf[0],576);
    for(ch=0;ch<nch;ch++,gr_info++){
        int aa_bands=31,n_long_bands=(gr_info->mixed_block_flag?2:0)<<(int)(MP3_HDR_GET_SAMPLE_RATEHDR(h->header)==2);
        if (gr_info->n_short_sfb){aa_bands=n_long_bands-1;mp3L3_reorder(s->grbuf[ch]+n_long_bands*18,s->syn[0],gr_info->sfbtab+gr_info->n_long_sfb);}
        mp3L3_antialias(s->grbuf[ch],aa_bands); mp3L3_imdct_gr(s->grbuf[ch],h->mdct_overlap[ch],gr_info->block_type,n_long_bands); mp3L3_change_sign(s->grbuf[ch]);
    }
}

static void mp3d_DCT_II(float *grbuf, int n){
    int i;
    for(int k=0;k<n;k++){
        float t[4][8],*x,*y=grbuf+k;
        for(x=t[0],i=0;i<8;i++,x++) { float x0=y[i*18], x1=y[(15-i)*18], x2=y[(16+i)*18],x3=y[(31-i)*18], t0=x0+x3, t1=x1+x2, t2=(x1-x2)*g_sec[3*i], t3=(x0-x3)*g_sec[3*i+1]; x[0]=t0+t1; x[8]=(t0-t1)*g_sec[3*i+2]; x[16]=t3+t2; x[24]=(t3-t2)*g_sec[3*i+2]; }
        for(x=t[0],i=0;i<4;i++,x+=8) {
            float x0=x[0], x1=x[1], x2=x[2], x3=x[3], x4=x[4], x5=x[5], x6=x[6], x7=x[7], xt;
            xt=x0-x7; x0+=x7; x7=x1-x6; x1+=x6; x6=x2-x5; x2+=x5; x5=x3-x4; x3+=x4; x4=x0-x3; x0+=x3; x3=x1-x2; x1+=x2; x[0]=x0+x1;
            x[4]=(x0-x1)*0.70710677f; x5+=x6; x6=(x6+x7)*0.70710677f; x7+=xt; x3=(x3+x4)*0.70710677f; x5-=x7*0.198912367f; x7+=x5*0.382683432f; x5-=x7*0.198912367f;
            x0=xt-x6; xt+=x6; x[1]=(xt+x7)*0.50979561f; x[2]=(x4+x3)*0.54119611f; x[3]=(x0-x5)*0.60134488f; x[5]=(x0+x5)*0.89997619f; x[6]=(x4-x3)*1.30656302f; x[7]=(xt-x7)*2.56291556f;
        }
        for(i=0;i<7;i++,y+=4*18){y[0]=t[0][i];y[18]=t[2][i]+t[3][i]+t[3][i+1];y[36]=t[1][i]+t[1][i+1];y[54]=t[2][i+1]+t[3][i]+t[3][i+1];}
        y[0]=t[0][7];y[18]=t[2][7]+t[3][7];y[36]=t[1][7];y[54]=t[3][7];
    }
}

typedef float mp3_sample_t;
static float mp3d_scale_pcm(float sample) { return sample*(1.f/32768.f); }
static void mp3d_synth_pair(mp3_sample_t *pcm, int nch, const float *z){
    float a; a =(z[14*64]-z[0])*29; a+=(z[1*64]+z[13*64])*213; a+=(z[12*64]-z[2*64])*459; a+=(z[ 3*64]+z[11*64])*2037; a+=(z[10*64]-z[4*64])*5153; a+=(z[5*64]+z[9*64])*6574; a+=(z[8*64]-z[6*64])*37489; a+=z[7*64]*75038;
    pcm[0]=mp3d_scale_pcm(a); z+=2; a =z[14*64]*104; a+=z[12*64]*1567; a+=z[10*64]*9727; a+=z[8*64]*64019; a+=z[6*64]*-9975; a+=z[4*64]*-45; a+=z[2*64]*146; a+=z[0*64]*-5; pcm[16*nch]=mp3d_scale_pcm(a);
}

static void mp3d_synth(float *xl, mp3_sample_t *dstl, int nch, float *lins){
    int i; float *xr=xl+576*(nch-1); mp3_sample_t *dstr=dstl+(nch-1); float *zlin=lins+15*64; const float *w=g_win;
    zlin[4*15]=xl[18*16];zlin[4*15+1]=xr[18*16];zlin[4*15+2]=xl[0];zlin[4*15+3]=xr[0]; zlin[4*31]=xl[1+18*16];zlin[4*31+1]=xr[1+18*16];zlin[4*31+2]=xl[1];zlin[4*31+3]=xr[1];
    mp3d_synth_pair(dstr,nch,lins+4*15+1); mp3d_synth_pair(dstr+32*nch,nch,lins+4*15+64+1); mp3d_synth_pair(dstl,nch,lins+4*15); mp3d_synth_pair(dstl+32*nch,nch,lins+4*15+64);
    for(i=14;i>=0;i--){
        #define MP3_LOAD(k) float w0=*w++;float w1=*w++;float *vz=&zlin[4*i-k*64];float *vy=&zlin[4*i-(15-k)*64];
        #define MP3_S0(k) {int j;MP3_LOAD(k) for(j=0;j<4;j++)b[j]=vz[j]*w1+vy[j]*w0,a[j]=vz[j]*w0-vy[j]*w1;}
        #define MP3_S1(k) {int j;MP3_LOAD(k) for(j=0;j<4;j++)b[j]+=vz[j]*w1+vy[j]*w0,a[j]+=vz[j]*w0-vy[j]*w1;}
        #define MP3_S2(k) {int j;MP3_LOAD(k) for(j=0;j<4;j++)b[j]+=vz[j]*w1+vy[j]*w0,a[j]+=vy[j]*w1-vz[j]*w0;}
        float a[4],b[4];
        zlin[4*i]=xl[18*(31-i)]; zlin[4*i+1]=xr[18*(31-i)]; zlin[4*i+2]=xl[1+18*(31-i)]; zlin[4*i+3]=xr[1+18*(31-i)]; zlin[4*(i+16)]=xl[1+18*(1+i)];zlin[4*(i+16)+1]=xr[1+18*(1+i)];zlin[4*(i-16)+2]=xl[18*(1+i)];zlin[4*(i-16)+3]=xr[18*(1+i)];
        MP3_S0(0) MP3_S2(1) MP3_S1(2) MP3_S2(3) MP3_S1(4) MP3_S2(5) MP3_S1(6) MP3_S2(7)
        dstr[(15-i)*nch]=mp3d_scale_pcm(a[1]); dstr[(17+i)*nch]=mp3d_scale_pcm(b[1]); dstl[(15-i)*nch]=mp3d_scale_pcm(a[0]); dstl[(17+i)*nch]=mp3d_scale_pcm(b[0]); dstr[(47-i)*nch]=mp3d_scale_pcm(a[3]); dstr[(49+i)*nch]=mp3d_scale_pcm(b[3]); dstl[(47-i)*nch]=mp3d_scale_pcm(a[2]); dstl[(49+i)*nch]=mp3d_scale_pcm(b[2]);
    }
}

static void mp3d_synth_granule(float *qmf_state, float *grbuf, int nbands, int nch, mp3_sample_t *pcm, float *lins){ for(int i=0;i<nch;i++) {mp3d_DCT_II(grbuf+576*i,nbands);} mcpy(lins,qmf_state,sizeof(float)*15*64); for(int i=0;i<nbands;i+=2) {mp3d_synth(grbuf+i,pcm+32*nch*i,nch,lins+i*64);} mcpy(qmf_state,lins+nbands*64,sizeof(float)*15*64); }
static int mp3d_match_frame(const u8 *hdr, int mp3_bytes){ for(int i=0,nmatch=0;nmatch<10;nmatch++){ i+=mp3_hdr_frame_bytes(hdr+i)+mp3_hdr_padding(hdr+i); if (i + 4 > mp3_bytes) {return nmatch>0;} if (!mp3_hdr_compare(hdr,hdr+i)) {return 0;} } return 1; }
static int mp3d_find_frame(const u8 *mp3, int mp3_bytes, int *ptr_frame_bytes){
    int i;
    for(i=0;i<mp3_bytes-4;i++,mp3++){
        if (mp3_hdr_valid(mp3)){
            int frame_bytes=mp3_hdr_frame_bytes(mp3); int fp=frame_bytes+mp3_hdr_padding(mp3);
            if((frame_bytes&&i+fp<=mp3_bytes&&mp3d_match_frame(mp3,mp3_bytes-i))||(!i&&fp==mp3_bytes)){*ptr_frame_bytes=fp;return i;}
        }
    }
    *ptr_frame_bytes=0; return mp3_bytes;
}

static void mp3dec_init(mp3dec *dec) { dec->header[0]=0; }
static int mp3dec_decode_frame(mp3dec *dec, const u8 *mp3, int mp3_bytes, void *pcm, mp3dec_frame_info *info){
    int i=0,igr,frame_size=0,success=1;
    const u8 *hdr; mp3_bs bs_frame[1];
    if (mp3_bytes>4&&dec->header[0]==0xff&&mp3_hdr_compare(dec->header,mp3)){ frame_size=mp3_hdr_frame_bytes(mp3)+mp3_hdr_padding(mp3); if(frame_size!=mp3_bytes&&(frame_size+4>mp3_bytes||!mp3_hdr_compare(mp3,mp3+frame_size))){frame_size=0;} }
    if (!frame_size){ mset(dec,0,sizeof(mp3dec)); i=mp3d_find_frame(mp3,mp3_bytes,&frame_size); if (!frame_size || i + frame_size > mp3_bytes) {info->frame_bytes=i;return 0;} }
    hdr=mp3+i; mcpy(dec->header,hdr,4); info->frame_bytes=i+frame_size; info->channels=MP3_HDR_IS_MONO(hdr) ? 1 : 2; info->sample_rate=mp3_hdr_sample_rate_hz(hdr); info->layer = 4 - MP3_HDR_GET_LAYER(hdr); info->bitrate_kbps=mp3_hdr_bitrate_kbps(hdr);
    bs_frame[0].buf=hdr + 4; bs_frame[0].pos=0; bs_frame[0].limit=(frame_size - 4) * 8; if(MP3_HDR_IS_CRC(hdr)){mp3_bs_get_bits(bs_frame,16);} if(info->layer!=3){return 0;}  /* Layer 1/2 not supported */
    int main_data_begin=mp3L3_read_side_info(bs_frame,dec->scratch.gr_info,hdr); if(main_data_begin<0||bs_frame->pos>bs_frame->limit){mp3dec_init(dec); return 0;}
    success=mp3L3_restore_reservoir(dec,bs_frame,&dec->scratch,main_data_begin);
    if(success&&pcm!=NULL){ for(igr=0;igr<(MP3_HDR_TEST_MPEG1(hdr)?2:1);igr++,pcm=MP3_OFFSET_PTR(pcm,sizeof(mp3_sample_t)*576*info->channels)){ mset(dec->scratch.grbuf[0],0,576 * 2 * sizeof(float)); mp3L3_decode(dec,&dec->scratch,dec->scratch.gr_info+igr*info->channels,info->channels); mp3d_synth_granule(dec->qmf_state,dec->scratch.grbuf[0],18,info->channels,(mp3_sample_t*)pcm,dec->scratch.syn[0]); } }
    mp3L3_save_reservoir(dec,&dec->scratch); return success*mp3_hdr_frame_samples(dec->header);
}

static size_t mp3_on_read_os(void *ud, void *buf, size_t n) { FHandle f = (FHandle)(uintptr_t)ud; if (f == INVALID_FHANDLE) {return 0;} long result = OS_Read(f,buf,n); return (result > 0) ? (size_t)result : 0; }
static bool mp3_on_seek_os(void *ud, int offset, u8 origin) { FHandle f = (FHandle)(uintptr_t)ud; if (f == INVALID_FHANDLE) {return false;} int whence = origin; return OS_Seek(f,(i64)offset,whence) >= 0; }
static size_t mp3_on_read(mp3 *p, void *buf, size_t n) { size_t r = mp3_on_read_os(p->pUserData,buf,n); p->streamCursor += r; return r; }
static size_t mp3_on_read_clamped(mp3 *p, void *buf, size_t n) { if (p->streamLength == (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF)) {return mp3_on_read(p,buf,n);} u64 rem = p->streamLength - p->streamCursor; if (n > rem) n = (size_t)rem; return mp3_on_read(p,buf,n); }
static bool mp3_on_seek(mp3 *p, int offset, u8 origin) { if (!mp3_on_seek_os(p->pUserData,offset,origin)) {return false;} if (origin == 0) {p->streamCursor = (u64)offset;}else{p->streamCursor += (u64)offset;} return true; }
static u32 mp3_decode_next_frame_ex(mp3 *p, mp3_sample_t *pPCMFrames, mp3dec_frame_info *pInfo) {
    u32 pcmFramesRead = 0; if (p->atEnd) return 0;
    for (;;) {
        mp3dec_frame_info info;
        if (p->dataSize < 16384) {
            if (p->pData) mmov(p->pData, p->pData + p->dataConsumed, p->dataSize);
            p->dataConsumed = 0;
            if (p->dataCapacity < (16384 * 4)) { u8 *nd = (u8*)OS_Realloc(p->pData,p->dataCapacity,16384 * 4); p->pData = nd; p->dataCapacity = 16384 * 4; }
            size_t bytesRead = mp3_on_read_clamped(p, p->pData + p->dataSize, p->dataCapacity - p->dataSize);
            if (!bytesRead && p->dataSize == 0) { p->atEnd = 1; return 0; }
            p->dataSize += bytesRead;
        }
        if (p->dataSize > 2147483647) { p->atEnd = 1; return 0; }
        if (!p->pData) return 0;
        pcmFramesRead = mp3dec_decode_frame(&p->decoder,p->pData + p->dataConsumed,(int)p->dataSize,pPCMFrames,&info);
        p->dataConsumed += (size_t)info.frame_bytes; p->dataSize -= (size_t)info.frame_bytes;
        if (pcmFramesRead > 0) {
            pcmFramesRead = mp3_hdr_frame_samples(p->decoder.header);
            p->pcmFConsInMP3F = 0; p->pcmFRemInMP3F = pcmFramesRead; p->mp3FChan = info.channels; p->mp3FrameSampleRate = info.sample_rate;
            if (pInfo) *pInfo = info;
            break;
        } else if (info.frame_bytes == 0) {
            mmov(p->pData, p->pData + p->dataConsumed, p->dataSize);
            p->dataConsumed = 0;
            if (p->dataCapacity == p->dataSize) { size_t needed=p->dataCapacity + 16384*4; u8 *nd=(u8*)OS_Realloc(p->pData,p->dataCapacity,needed); p->pData=nd; p->dataCapacity=needed; }
            size_t bytesRead = mp3_on_read_clamped(p,p->pData + p->dataSize,p->dataCapacity - p->dataSize);
            if (!bytesRead) { p->atEnd = 1; return 0; }
            p->dataSize += bytesRead;
        }
    }
    return pcmFramesRead;
}

static u32 mp3_decode_next_frame(mp3 *p) { return mp3_decode_next_frame_ex(p,(mp3_sample_t*)p->pcmFrames,NULL); }
static void mp3_skip_id3v2(mp3 *p) {
    char h[10]; if (mp3_on_read_os(p->pUserData, h, 10) != 10) return;
    if (h[0] == 'I' && h[1] == 'D' && h[2] == '3') {
        u32 sz = (((u32)h[6] & 0x7F) << 21) | (((u32)h[7] & 0x7F) << 14) | (((u32)h[8] & 0x7F) << 7) | ((u32)h[9] & 0x7F);
        if (h[5] & 0x10) sz += 10;
        mp3_on_seek_os(p->pUserData,(int)sz,1); // SEEK_CUR
        p->streamStartOffset += 10 + sz; p->streamCursor=p->streamStartOffset;
    } else mp3_on_seek_os(p->pUserData,0,0); // SEEK_SET
}

static bool mp3_init_internal(mp3 *p) {
    mp3dec_init(&p->decoder);
    p->streamCursor = p->streamStartOffset=0; p->streamLength = (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF); p->delayInPCMFrames = p->paddingInPCMFrames=0; p->totalPCMFrameCount = (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF);
    if (mp3_on_seek_os(p->pUserData, 0, 2)) { // SEEK_END
        i64 slen = OS_Tell((FHandle)(uintptr_t)p->pUserData);
        if (slen > 0) { if (slen > 128) { char tag[3]; mp3_on_seek_os(p->pUserData,-128,2); if (mp3_on_read(p, tag, 3) == 3 && tag[0]=='T' && tag[1]=='A' && tag[2]=='G') slen -= 128; } p->streamLength = (u64)slen; }
        mp3_on_seek_os(p->pUserData,0,0); p->streamCursor = 0;
    }
    mp3_skip_id3v2(p); mp3dec_frame_info firstFrameInfo; u32 firstFramePCMFrameCount = mp3_decode_next_frame_ex(p,(mp3_sample_t*)p->pcmFrames,&firstFrameInfo);
    if (firstFramePCMFrameCount == 0) { OS_Free(p->pData,p->dataCapacity); p->pData = NULL; p->dataCapacity = 0; return false; }
    p->channels=p->mp3FChan; p->sampleRate=p->mp3FrameSampleRate; return true;
}

static bool mp3_init_file(mp3 *pMP3, const char *path) { if(!pMP3 || !path){return false;} mset(pMP3,0,sizeof(mp3)); FHandle f=OS_OpenReadonly(path); if(f == INVALID_FHANDLE){return false;} pMP3->pUserData=(void*)(uintptr_t)f; bool r=mp3_init_internal(pMP3); if(!r){OS_Close(f); return false;} return true; }
static void mp3_uninit(mp3 *pMP3) { if (!pMP3) {return;} if (pMP3->pUserData) {OS_Close((FHandle)(uintptr_t)pMP3->pUserData); pMP3->pUserData=NULL;} OS_Free(pMP3->pData, pMP3->dataCapacity); pMP3->pData = NULL; pMP3->dataCapacity = 0; }
static void mp3_reset(mp3 *p) { p->pcmFConsInMP3F=0; p->pcmFRemInMP3F=0; p->currentPCMFrame=0; p->dataSize=0; p->atEnd=0; mp3dec_init(&p->decoder); }
static bool mp3_seek_to_start_of_stream(mp3 *p){u64 o=p->streamStartOffset;if(!mp3_on_seek(p,o<=0x7FFFFFFF?(int)o:0x7FFFFFFF,0))return 0;if(o>0x7FFFFFFF){o-=0x7FFFFFFF;while(o>0){int c=(o<=0x7FFFFFFF)?(int)o:0x7FFFFFFF;if(!mp3_on_seek(p,c,1))return 0;o-=c;}}mp3_reset(p);return 1;}
static u64 mp3_read_pcm_frames_raw(mp3 *p, u64 framesToRead, void *pBufferOut){
    u64 totalFramesRead=0;
    while(framesToRead>0){
        u32 framesToConsume;
        if(p->currentPCMFrame<p->delayInPCMFrames){ u32 skip=(u32)vmin(p->pcmFRemInMP3F,p->delayInPCMFrames-p->currentPCMFrame); p->currentPCMFrame+=skip; p->pcmFConsInMP3F+=skip; p->pcmFRemInMP3F-=skip; }
        framesToConsume=(u32)vmin(p->pcmFRemInMP3F,framesToRead);
        if(p->totalPCMFrameCount != (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF) && p->totalPCMFrameCount > p->paddingInPCMFrames){
            if(p->currentPCMFrame<(p->totalPCMFrameCount-p->paddingInPCMFrames)){ u64 rem=(p->totalPCMFrameCount-p->paddingInPCMFrames)-p->currentPCMFrame; if(framesToConsume>rem) framesToConsume=(u32)rem; } else break;
        }
        if(pBufferOut){ float *out=(float*)MP3_OFFSET_PTR(pBufferOut,sizeof(float)*totalFramesRead*p->channels); float *in =(float*)MP3_OFFSET_PTR(&p->pcmFrames[0],sizeof(float)*p->pcmFConsInMP3F*p->mp3FChan); mcpy(out,in,sizeof(float)*framesToConsume*p->channels); }
        p->currentPCMFrame+=framesToConsume; p->pcmFConsInMP3F+=framesToConsume; p->pcmFRemInMP3F-=framesToConsume; totalFramesRead+=framesToConsume; framesToRead-=framesToConsume;
        if(framesToRead==0) break;
        if(p->totalPCMFrameCount != (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF) && p->totalPCMFrameCount > p->paddingInPCMFrames && p->currentPCMFrame >= (p->totalPCMFrameCount - p->paddingInPCMFrames)) break;
        if(mp3_decode_next_frame(p)==0) break;
    }
    return totalFramesRead;
}

static u64 mp3_read_pcm_frames_f32(mp3* m, u64 framesToRead, float *pBufferOut){ if(!m) {return 0;} return mp3_read_pcm_frames_raw(m,framesToRead,pBufferOut); }
static bool mp3_seek_to_pcm_frame(mp3* m, u64 fidx){ if(!m) {return 0;} if(fidx==0){return mp3_seek_to_start_of_stream(m);} if(fidx<m->currentPCMFrame){ if(!mp3_seek_to_start_of_stream(m)) {return 0;} } u64 toSkip=fidx-m->currentPCMFrame; u64 skipped=mp3_read_pcm_frames_f32(m,toSkip,NULL); return skipped==toSkip; }
static u64 mp3_get_pcm_frame_count(mp3* pMP3){
    u64 total; if(pMP3->totalPCMFrameCount != (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF)){ total=pMP3->totalPCMFrameCount; if(total>=pMP3->delayInPCMFrames){total-=pMP3->delayInPCMFrames;} if(total>=pMP3->paddingInPCMFrames){total-=pMP3->paddingInPCMFrames;} return total; }
    u64 savedFrame=pMP3->currentPCMFrame; if(!mp3_seek_to_start_of_stream(pMP3)) return 0;
    total=0; for(;;){ u32 n=mp3_decode_next_frame_ex(pMP3,NULL,NULL); if(!n){break;}total+=n; } mp3_seek_to_start_of_stream(pMP3); mp3_seek_to_pcm_frame(pMP3,savedFrame); return total;
}

typedef struct { FHandle fp; u16 channels,bitsPerSample,fmtTag; u32 sampleRate; u64 totalPCMFrameCount,dataChunkDataPos,bytesRemaining; } WaveFile;
static u16 WavU16LE(const u8 *d) { return (u16)(d[0]|(d[1]<<8)); }
static u32 WavU32LE(const u8 *d) { return (u32)(d[0]|(d[1]<<8)|(d[2]<<16)|(d[3]<<24)); }
static bool WavInit(WaveFile *w, const char *path) {
    u8 buf[36]; mset(w,0,sizeof(*w)); w->fp = OS_OpenReadonly(path); if (w->fp == INVALID_FHANDLE) return false;
    if (OS_Read(w->fp, buf, 12) != 12) goto fail;
    if (mcmp(buf,"RIFF",4) != 0) goto fail;
    if (mcmp(buf+8,"WAVE",4) != 0) goto fail;
    bool got_fmt=false,got_data=false;
    for (;;) {
        u8 chunkId[4],szBuf[4]; if ((OS_Read(w->fp,chunkId,4) != 4) || (OS_Read(w->fp,szBuf,4) != 4)) break;
        u32 chunkSize = WavU32LE(szBuf);
        if (mcmp(chunkId, "fmt ", 4) == 0) {
            if (chunkSize < 16) goto fail;
            u8 fmt[18]; u32 toRead = chunkSize < 18 ? chunkSize : 18; if (OS_Read(w->fp,fmt,toRead) != (long)toRead) goto fail;
            if (chunkSize > toRead) OS_Seek(w->fp, (i64)(chunkSize - toRead),1);
            w->fmtTag = WavU16LE(fmt+0); w->channels = WavU16LE(fmt+2); w->sampleRate = WavU32LE(fmt+4); w->bitsPerSample = WavU16LE(fmt+14);
            if (w->fmtTag == 0xFFFE && toRead >= 18) { u16 cbSize = WavU16LE(fmt + 16); if (cbSize >= 22) { u8 ext[22]; if(OS_Read(w->fp,ext,22) == 22){w->fmtTag=WavU16LE(ext + 6);} } }
            if (w->fmtTag != 0x1) goto fail; // PCM format
            if (w->bitsPerSample != 8 && w->bitsPerSample != 16) goto fail;
            got_fmt = true;
        } else if (mcmp(chunkId,"data",4) == 0) {
            w->dataChunkDataPos = (u64)OS_Tell(w->fp);
            u32 bpf = (u32)w->channels * (w->bitsPerSample / 8);
            if (bpf == 0) goto fail;
            w->bytesRemaining = chunkSize - (chunkSize % bpf);
            w->totalPCMFrameCount = w->bytesRemaining / bpf;
            got_data = true;
            break; /* data chunk is last thing we need */
        } else OS_Seek(w->fp,(i64)(chunkSize + (chunkSize & 1)),1);
    }

    if (got_fmt && got_data) return true;
    fail:
    if (w->fp != INVALID_FHANDLE) { OS_Close(w->fp); w->fp = INVALID_FHANDLE; }
    return false;
}

static u64 WavReadPCMFrames(WaveFile *w, u64 framesToRead, float *out) {
    if (!w || !out || framesToRead == 0) return 0;
    u32 bps = w->bitsPerSample; u32 bpf = (u32)w->channels * (bps / 8); if (bpf == 0) return 0;
    u64 framesLeft=w->bytesRemaining / bpf; if(framesToRead > framesLeft){framesToRead=framesLeft;}
    u64 totalRead=0; u8  tmp[4096];
    while (framesToRead > 0) {
        u64 batchFrames=framesToRead; u64 batchBytes=batchFrames * bpf;
        if (batchBytes > sizeof(tmp)) { batchFrames = sizeof(tmp) / bpf; batchBytes  = batchFrames * bpf; }
        size_t got=OS_Read(w->fp,tmp,(size_t)batchBytes);
        u64 gotFrames=got / bpf; u64 samples=gotFrames * w->channels;
        if (bps == 8) { for (u64 i = 0; i < samples; i++) {*out++ = (tmp[i] / 255.0f) * 2.0f - 1.0f;} }
        else { for (u64 i = 0; i < samples; i++) {i16 s; mcpy(&s,tmp + i*2,2); *out++ = s * (1.0f / 32768.0f);} } // 16bit LE

        w->bytesRemaining -= gotFrames * bpf; framesToRead -= gotFrames; totalRead += gotFrames; if (gotFrames < batchFrames) break;
    }
    return totalRead;
}

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
    float vol = w->volume * (Sys_Settings.VolumeMaster/100.0f)*(Sys_Settings.VolumeEffects/100.0f); V3 pos = w->pos; float dist = V3_Dist(pos,World.position[PLAYER1]); float spatial_atten = (dist >= 64.0f) ? 0.0f : ((dist <= 1.0f) ? 1.0f : 1.0f-(dist-1.0f)/63.0f);
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

void play_wav(const char *path,float volume,V3 pos,bool positional) {
    if (sEqual(path,"./Audio/misc/null.wav")) return;
    i32 slot = GetFreeWavSlot();
    if (slot==-1 && wav_count<MAX_CHANNELS) slot=wav_count++;
    if (slot==-1) { DualLog("WARNING: Max WAV channels (%d) reached\n",MAX_CHANNELS); return; }
    u32 frames; size_t sz=0; float *buf = load_wav(path,&frames,&sz);
    if (!buf) { DualLog("ERROR: Failed to load WAV %s\n",path); return; }
    wav_ch[slot] = (wav_channel_t){ .samples = buf, .allocSize = sz, .frame_count = frames, .frame_pos = 0, .volume = volume, .looping = false, .positional = positional, .pos=pos, .playing = true };
}

void play_message(const char *path) {
    if (log_playing && log_samples && log_allocSize > 0) { log_playing=false; OS_Free(log_samples,log_allocSize); log_samples=NULL; log_allocSize=0; }
    u32 frames; float *buf = load_wav(path,&frames,&log_allocSize); if (!buf) { DualLogError("Failed to load %s\n",path); return; }
    log_samples=buf; log_frame_count=frames; log_frame_pos=0; log_playing=true;
}

i32 SndInit(const char *path, wav_channel_t *w) { u32 frames; size_t sz=0; float *buf=load_wav(path,&frames,&sz); if(!buf){return -1;} w->samples=buf; w->allocSize=sz; w->frame_count=frames; w->frame_pos=0; w->volume=1.0f; w->looping=w->positional=w->playing=false; return 0; }
i32 SndStart(wav_channel_t* w) { w->frame_pos = 0; w->playing = true; for (u32 i=0;i<ext_count;++i) if (ext_ch[i] == w) return 0; if (ext_count < MAX_CHANNELS) ext_ch[ext_count++] = w; return 0; }
void SndStop(wav_channel_t* w) { w->playing=false; }
void SndUninit(wav_channel_t* w) { if (w->samples) { OS_Free(w->samples,w->allocSize); w->samples = NULL; w->allocSize = 0; } w->playing = false; for (u32 i=0;i<ext_count;++i) if (ext_ch[i] == w) { ext_ch[i] = ext_ch[--ext_count]; break; } }
void SndSetVolume(wav_channel_t* w, float volume) { w->volume=volume; }
void SoundSetLooping(wav_channel_t* w, bool loop) { w->looping=loop; }
bool SndPlaying(wav_channel_t* w)  { return w->playing; }
i32 SndFrmCurpos(wav_channel_t* w,u64 *pCursor) { *pCursor=w->frame_pos; return 0; }
float SndLen(wav_channel_t* w) {  return (float)w->frame_count / (float)AUDIO_RATE; }
static void mp3_open_slot(i32 s, const char *path, float fade_from, float fade_to, i32 fade_ms) {
    mp3_channel_t *m = &mp3_ch[s]; if (m->open) { mp3_uninit(&m->dec); m->open=false; } if (!mp3_init_file(&m->dec,path)) { DualLog("ERROR: Failed to load MP3 %s\n",path); return; }
    m->src_rate = m->dec.sampleRate; m->total_frames = mp3_get_pcm_frame_count(&m->dec); mp3_seek_to_pcm_frame(&m->dec,0); m->frames_decoded = 0; m->open = true; m->fade_target = fade_to;
    m->fade_vol = (m->fade_step = (fade_ms > 0) ? (fade_to - fade_from) / (AUDIO_RATE * fade_ms / 1000.0f) : 0.0f) == 0.0f ? fade_to : fade_from;
}

void play_mp3(const char *path, i32 fade_ms) { i32 old = mp3_slot, next = mp3_slot ? 0 : 1; if (mp3_ch[old].open) { mp3_ch[old].fade_target = 0.0f; mp3_ch[old].fade_step = (fade_ms > 0) ? -mp3_ch[old].fade_vol / (AUDIO_RATE * fade_ms / 1000.0f) : -1.0f; } mp3_open_slot(mp3_slot = next,path,0.0f,1.0f,fade_ms); }
void mp3_clear() { for (i32 i=0;i<2;i++) if (mp3_ch[i].open) { mp3_uninit(&mp3_ch[i].dec); mp3_ch[i].open=false; } mp3_slot=0; }
void MP3Pause() { mp3_paused = true; }
void MP3Resume() { mp3_paused = false; }
float GetMP3RemainingTime() { mp3_channel_t *m = &mp3_ch[mp3_slot]; return (!m->open || m->frames_decoded >= m->total_frames) ? 0.0f : (!m->total_frames ? 1.0f : (float)(m->total_frames - m->frames_decoded) / (m->src_rate ? m->src_rate : AUDIO_RATE)); }
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
// Looping Ambients SFX System
#define MAXAMB 256
static u16 ambs = 0;
typedef struct { wav_channel_t sound; u32 loaded; float length_sec; } AmbientSlot;
typedef struct { u16 index; const char* filename; } AmbientDef;
u16 ambReg[MAXAMB]; // For ambient_ type entities that play looped sound
static AmbientSlot ambientSlots[MAXAMB] = {0};
static const AmbientDef ambientSounds[MAXAMB] = {
    {621,"airhiss.wav"},        {622,"clicker.wav"},  {623,"compressor.wav"},    {624,"dishwasher.wav"},{625,"drip_amb.wav"},{626,"fan1.wav"},         {627,"generator_gas.wav"},   {628,"gurgle.wav"},    {629,"icemaker.wav"},       {630,"intake.wav"},            {631,"lathe.wav"},        {632,"lev3loop1.wav"},    {633,"lev3loop2.wav"},
    {634,"lev3loop3.wav"},      {635,"lev3loop4.wav"},{636,"liquid_bubble.wav"}, {637,"lava2.wav"},     {638,"rain.wav"},    {639,"machgear_loop.wav"},{640,"machine_ambience.wav"},{641,"machine_go.wav"},{642,"machine_humamb7.wav"},{643,"machine_humlonoise.wav"},{644,"machine_loop1.wav"},{645,"machine_loop2.wav"},{646,"machinea1.wav"},
    {647,"machinevat_loop.wav"},{648,"mist.wav"},     {649,"pipewater_loop.wav"},{650,"powerloom.wav"}, {651,"pump.wav"},    {652,"pump2.wav"},        {653,"rain.wav"},            {654,"steam_loop.wav"},{655,"washing_machine.wav"}};
void MixAmbs(void) {    
    for (u16 i=0;i<ambs;++i) {
        u16 a = ambReg[i];
        const AmbientDef* def = NULL; for (size_t j=0;j<MAXAMB;++j) { if (ambientSounds[j].index==World.instances[a].index) {def = &ambientSounds[j]; break; } }
        float d = V3_Dist(World.position[PLAYER1],World.position[ambReg[i]]);
        AmbientSlot* slot = &ambientSlots[(size_t)(def - ambientSounds)];
        if (d < 7.68f && PositionVisibleFromPlayerCell(World.position[ambReg[i]].x,World.position[ambReg[i]].z)) {
            if (!slot->loaded) {
                SndUninit(&slot->sound);
                char path[512]; sFormat(path,sizeof(path),"./Audio/ambient/%s",def->filename);
                if (SndInit(path,&slot->sound) != 0) continue;
                slot->length_sec = SndLen(&slot->sound); if(slot->length_sec <= 0.0f) {SndUninit(&slot->sound); continue;}
                SoundSetLooping(&slot->sound,true);
                slot->loaded = 1;
            }
            if (!SndPlaying(&slot->sound)) SndStart(&slot->sound);
            if (slot->length_sec > 0.0f) { u64 cur; SndFrmCurpos(&slot->sound,&cur); } // Time sync
            float final_vol = World.instances[a].volume * ((d <= 1.0f) ? 1.0f : (d >= 7.68f) ? 0.0f : (7.68f - d) / (7.68f - 1.0f));
            SndSetVolume(&slot->sound,final_vol);
        } else if (SndPlaying(&slot->sound)) SndStop(&slot->sound);
    }
}
 
void ResetLevelAudio(void) { ambs=0; mset(ambReg,0,ambs * sizeof(u16)); for (u16 i = INSTS_1ST_IDX; i<World.instCount;++i) { if(IdxIsAmbient(World.instances[i].index)){ambReg[ambs]=i; ambs++; if(ambs >= MAXAMB){DualLogError("Ambient noises %u > %u!\n",ambs,MAXAMB); break;} World.instances[i].volume=EDefs[World.instances[i].index].volume * 0.5f;} } }
// Music System
#define BUFFER_MS 50
#define AUD_BUFFER_T 0.05f
const char* levelMusicLooped[14] = {"./Audio/music/looped/track0.mp3","./Audio/music/looped/track1.mp3","./Audio/music/looped/track2.mp3","./Audio/music/looped/track3.mp3","./Audio/music/looped/track4.mp3",
                                    "./Audio/music/looped/track5.mp3","./Audio/music/looped/track6.mp3","./Audio/music/looped/track7.mp3","./Audio/music/looped/track8.mp3","./Audio/music/looped/track9.mp3",
                                    "./Audio/music/looped/track10.mp3","./Audio/music/looped/track11.mp3","./Audio/music/looped/track12.mp3","./Audio/music/looped/track13.mp3"};
const char* reactorMusic[13] = {"./Audio/music/THM4-01_reactorcombat1.mp3","./Audio/music/THM4-02_reactorcombat2.mp3","./Audio/music/THM4-03_reactorcombat3.mp3",
                                "./Audio/music/THM4-04_reactorcombat4.mp3","./Audio/music/THM4-05_reactorwalkingatocombat.mp3","./Audio/music/THM4-06_reactorwalkingbtocombat.mp3",
                                "./Audio/music/THM4-09_reactorwalkinga1.mp3","./Audio/music/THM4-10_reactorwalkinga2.mp3","./Audio/music/THM4-11_reactorwalkingb1.mp3",
                                "./Audio/music/THM4-12_reactorwalkingb2.mp3","./Audio/music/THM4-13_reactorwalkingb3.mp3","./Audio/music/THM4-14_reactorwalkingc1.mp3",
                                "./Audio/music/THM4-15_reactorwalkingc2.mp3"};
const char* medicalMusic[11] = {"./Audio/music/THM1-19_medicalstart.mp3","./Audio/music/THM1-01_medicalwalking1.mp3","./Audio/music/THM1-02_medicalwalking2.mp3","./Audio/music/THM1-03_medicalwalking3.mp3",
                                "./Audio/music/THM1-04_medicalwalking4.mp3","./Audio/music/THM1-05_medicalcombat1.mp3","./Audio/music/THM1-06_medicalcombat2.mp3","./Audio/music/THM1-07_medicalcombat3.mp3",
                                "./Audio/music/THM1-08_medicalcombat4.mp3","./Audio/music/THM1-09_medicalcombat5.mp3","./Audio/music/THM1-10_medicalcombat6.mp3"};
const char* scienceMusic[10] = {"./Audio/music/THM3-17_sciencestart.mp3","./Audio/music/THM3-03_science1.mp3","./Audio/music/THM3-04_science2.mp3","./Audio/music/THM3-05_science3.mp3",
                                "./Audio/music/THM3-06_science4.mp3","./Audio/music/THM3-07_science5.mp3","./Audio/music/THM3-08_science6.mp3","./Audio/music/THM3-09_science7.mp3",
                                "./Audio/music/THM3-01_scienceaction1.mp3","./Audio/music/THM3-02_scienceaction2.mp3"};
const char* executiveMusic[13] = {"./Audio/music/THM2-11_executive1.mp3","./Audio/music/THM2-12_executive2.mp3","./Audio/music/THM2-13_executive3.mp3",
                                  "./Audio/music/THM2-08_executive4.mp3","./Audio/music/THM2-09_executive5.mp3","./Audio/music/THM2-10_executive6.mp3",
                                  "./Audio/music/THM2-04_executive2.mp3","./Audio/music/THM2-05_executive3.mp3","./Audio/music/THM2-06_executivefluterlude.mp3",
                                  "./Audio/music/THM2-07_executivefluterludewithguitar.mp3","./Audio/music/THM2-01_executiveaction3.mp3","./Audio/music/THM2-02_executiveaction4.mp3",
                                  "./Audio/music/THM2-03_executiveaction5.mp3"};
const char* groveMusic[24] = {"./Audio/music/THM5-07_groveaction1.mp3","./Audio/music/THM5-08_groveaction1.mp3","./Audio/music/THM5-09_groveaction2.mp3","./Audio/music/THM5-10_groveaction3.mp3",
                              "./Audio/music/THM5-11_groveaction4.mp3","./Audio/music/THM5-12_groveaction5.mp3","./Audio/music/THM5-13_groveaction6.mp3","./Audio/music/THM5-14_groveaction7.mp3",
                              "./Audio/music/THM5-15_groveaction8.mp3","./Audio/music/THM5-33_grove1.mp3","./Audio/music/THM5-34_grove2.mp3","./Audio/music/THM5-38_grove3.mp3",
                              "./Audio/music/THM5-39_grove4.mp3","./Audio/music/THM5-40_grove5.mp3","./Audio/music/THM5-35_grove99.mp3","./Audio/music/THM5-36_grove100.mp3",
                              "./Audio/music/THM5-37_grove101.mp3","./Audio/music/THM5-41_grove102.mp3","./Audio/music/THM5-42_grove103.mp3","./Audio/music/THM5-01_grove105.mp3",
                              "./Audio/music/THM5-02_grove106.mp3","./Audio/music/THM5-03_grove107.mp3","./Audio/music/THM5-04_grove108.mp3","./Audio/music/THM5-05_grove109.mp3"};
const char* securityMusic[19] = {"./Audio/music/THM6-05_securityaction1.mp3","./Audio/music/THM6-06_securityaction2.mp3","./Audio/music/THM6-07_securityaction3.mp3",
                                    "./Audio/music/THM6-08_securityaction4.mp3","./Audio/music/THM6-09_securityaction5.mp3","./Audio/music/THM6-10_securityaction6.mp3",
                                    "./Audio/music/THM6-01_security1.mp3","./Audio/music/THM6-02_security2.mp3","./Audio/music/THM6-03_security3.mp3",
                                    "./Audio/music/THM6-04_security4.mp3","./Audio/music/THM6-11_security100.mp3","./Audio/music/THM6-12_security101.mp3",
                                    "./Audio/music/THM6-13_security1.mp3","./Audio/music/THM6-14_security2.mp3","./Audio/music/THM6-15_security3.mp3",
                                    "./Audio/music/THM6-17_security4.mp3","./Audio/music/THM6-18_security5.mp3","./Audio/music/THM6-19_security6.mp3",
                                    "./Audio/music/THM6-20_security7.mp3"};
const char* cyberMusic[13] = {"./Audio/music/THM10-02_cyberstart.mp3","./Audio/music/THM10-01_cyber1.mp3","./Audio/music/THM10-03_cyber2.mp3","./Audio/music/THM10-04_cyber3.mp3",
                              "./Audio/music/THM10-05_cyber4.mp3","./Audio/music/THM10-06_cyber5.mp3","./Audio/music/THM10-07_cyber6.mp3",
                              "./Audio/music/THM10-08_cyber7.mp3","./Audio/music/THM10-09_cyber8.mp3"};
const char* levelMusicElevator[13] = {"./Audio/music/THM7-01_elevator1.mp3","./Audio/music/THM7-01_elevator1.mp3","./Audio/music/THM7-02_elevator2.mp3",
                                      "./Audio/music/THM7-03_elevator3.mp3","./Audio/music/THM7-04_elevator4.mp3","./Audio/music/THM7-05_elevator5.mp3",
                                      "./Audio/music/THM7-06_elevator6.mp3","./Audio/music/THM7-07_elevator7.mp3","./Audio/music/THM7-08_elevator8.mp3",
                                      "./Audio/music/THM7-01_elevator1.mp3","./Audio/music/THM7-01_elevator1.mp3","./Audio/music/THM7-01_elevator1.mp3",
                                      "./Audio/music/THM7-01_elevator1.mp3"};
const char* levelMusicRevive[14] = {"./Audio/music/THM4-18_reactorrevive.mp3","./Audio/music/THM1-18_medicalrevive.mp3","./Audio/music/THM3-19_sciencerevive.mp3","./Audio/music/THM3-19_sciencerevive.mp3",
                                    "./Audio/music/THM3-19_sciencerevive.mp3","./Audio/music/THM1-18_medicalrevive.mp3","./Audio/music/THM2-18_executiverevive.mp3","./Audio/music/THM4-18_reactorrevive.mp3",
                                    "./Audio/music/THM6-22_securityrevive.mp3","./Audio/music/THM1-18_medicalrevive.mp3","./Audio/music/THM2-18_executiverevive.mp3","./Audio/music/THM2-18_executiverevive.mp3",
                                    "./Audio/music/THM2-18_executiverevive.mp3","./Audio/music/THM1-18_medicalrevive.mp3"};
const char* levelMusicDistortion[14] = {"./Audio/music/THM6-49_securitydistorted.mp3","./Audio/music/THM1-48_medicaldistorted.mp3","./Audio/music/THM3-49_sciencedistorted.mp3",
                                        "./Audio/music/THM3-49_sciencedistorted.mp3","./Audio/music/THM1-48_medicaldistorted.mp3","./Audio/music/THM1-48_medicaldistorted.mp3",
                                        "./Audio/music/THM2-46_executivedistorted.mp3","./Audio/music/THM1-48_medicaldistorted.mp3","./Audio/music/THM6-49_securitydistorted.mp3",
                                        "./Audio/music/THM1-48_medicaldistorted.mp3","./Audio/music/THM1-48_medicaldistorted.mp3","./Audio/music/THM1-48_medicaldistorted.mp3",
                                        "./Audio/music/THM1-48_medicaldistorted.mp3","./Audio/music/THM10-41_cyberdistorted.mp3"};
const char* levelMusicDeath[14] = {"./Audio/music/THM0-17_death.mp3","./Audio/music/THM1-17_death.mp3","./Audio/music/THM3-18_death.mp3","./Audio/music/THM0-17_death.mp3",
                                   "./Audio/music/THM3-18_death.mp3","./Audio/music/THM0-17_death.mp3","./Audio/music/THM2-17_death.mp3","./Audio/music/THM0-17_death.mp3",
                                   "./Audio/music/THM6-21_death.mp3","./Audio/music/THM0-17_death.mp3","./Audio/music/THM5-17_death.mp3","./Audio/music/THM5-17_death.mp3",
                                   "./Audio/music/THM5-17_death.mp3","./Audio/music/THM10-16_death.mp3"};
void PlayMenuMusic(void) { mp3_clear(); play_mp3("./Audio/music/TITLOOP-00_menu.mp3",1500); }
void PlayGameMusic(void) { mp3_clear(); /*play_mp3("./Audio/music/THM1-19_medicalstart.mp3",100);*/ }
const char* GetCorrespondingLevelClip(TrackType ttype) {
    switch(ttype) { // Override types, return from these first before special level handling
        case TrackType_Revive:     return levelMusicRevive[World.curLev];
        case TrackType_Death:      return levelMusicDeath[World.curLev];
        case TrackType_Elevator:   return levelMusicElevator[World.curLev];
        case TrackType_Distortion: return levelMusicDistortion[World.curLev];
    }

    if (World.curLev == 0 || World.curLev == 5 || World.curLev == 7) { // 0  REACTOR, 5 FLIGHT, 7 ENGINEERING
        if (World.Sys_Music.levelEntry) return reactorMusic[6];
        if (ttype == TrackType_Combat)  return reactorMusic[random_range_u8(0,6)];
        return reactorMusic[random_range_u8(6,13)];
    } else if (World.curLev == 1) { // 1  MEDICAL
        if (World.Sys_Music.levelEntry) return medicalMusic[0];
        if (ttype == TrackType_Combat)  return medicalMusic[random_range_u8(5,11)];
        return medicalMusic[random_range_u8(1,5)];
    } else if (World.curLev == 2 || World.curLev == 4) { // 2  SCIENCE, 4 STORAGE
        if (World.Sys_Music.levelEntry) return scienceMusic[0];
        if (ttype == TrackType_Combat)  return scienceMusic[random_range_u8(8,10)];
        return scienceMusic[random_range_u8(1,8)];
    } else if (World.curLev == 8) { // 8 SECURITY
        if (World.Sys_Music.levelEntry) return securityMusic[9];
        if (ttype == TrackType_Combat)  return securityMusic[random_range_u8(0,6)];
        return securityMusic[random_range_u8(6,19)];
    } else if (World.curLev == 6) { // 6 EXECUTIVE
        if (World.Sys_Music.levelEntry) return executiveMusic[0];
        if (ttype == TrackType_Combat)  return executiveMusic[random_range_u8(9,13)];
        return executiveMusic[random_range_u8(0,10)];
    } else if (World.curLev == 10 || World.curLev == 11 || World.curLev == 12) { // 10, 12 GROVES
        if (World.Sys_Music.levelEntry) return groveMusic[19];
        if (ttype == TrackType_Combat)  return groveMusic[random_range_u8(0,9)];
        return executiveMusic[random_range_u8(9,24)];
    } else if (World.curLev == 13) { // 13 CYBERSPACE
        if (World.Sys_Music.levelEntry)     return cyberMusic[0];
        if (World.Sys_Music.cyberTube)      return cyberMusic[random_range_u8(4,8)];
        if (random_range(0.0f,1.0f) < 0.5f) return cyberMusic[random_range_u8(1,5)];
        else                                return cyberMusic[8];
    }

    return levelMusicLooped[0];
}

void PlayTrack(TrackType ttype, MusicType mtype) {
    if (!Sys_Settings.DynamicMusic) { // Looped Music (Dynamic Music off)
        if (mtype == MusicType_Override) {
                 if (ttype == TrackType_Revive)     play_mp3(levelMusicRevive[World.curLev],0);
            else if (ttype == TrackType_Death)      play_mp3(levelMusicDeath[World.curLev],0);
            else if (ttype == TrackType_Elevator)   play_mp3(levelMusicElevator[World.curLev],0);
            else if (ttype == TrackType_Distortion) play_mp3(levelMusicDistortion[World.curLev],0);
        } else play_mp3(levelMusicLooped[World.curLev],0);
        
        return;
    }
    

    // Normal Dynamic Music System
    if (mtype == MusicType_Override) mp3_clear();
    play_mp3(GetCorrespondingLevelClip(ttype),BUFFER_MS);
    if (!World.Sys_Music.elevator) World.Sys_Music.levelEntry = false; // already used by GetCorresponding... just now
}

void MusicNotifyZone(TrackType tt) {
    World.Sys_Music.inZone = true;
    switch(tt) {
        case TrackType_Elevator: World.Sys_Music.elevator = true; break;
        case TrackType_Distortion: World.Sys_Music.distortion = true; break;
    }
}

void MusicTriggerEnter(u16 self, u16 other) {
    if (World.instances[self].tickFinished < World.pauseRelativeTime) { // Prevent flickering retrigger when player slides along glancing angle of trigger volume.
        if (other == PLAYER1) { PlayTrack(World.instances[self].trackType,World.instances[self].musicType); MusicNotifyZone(World.instances[self].trackType); }        
        World.instances[self].tickFinished = World.pauseRelativeTime + 0.1;
    }
}

void MusicTriggerExit(u16 other) { if (other == PLAYER1) { mp3_clear(); World.Sys_Music.inZone = World.Sys_Music.elevator = World.Sys_Music.distortion = false; } }// return to normal upon leaving the trigger
void UpdateMusic(void) {
    if (World.paused && !World.menuActive) { MP3Pause(); return; }
    MP3Resume();
    float remaining = GetMP3RemainingTime(); if (remaining > AUD_BUFFER_T) return;
    if (World.menuActive) { play_mp3("./Audio/music/TITLOOP-00_menu.mp3",1500); return; }
    if (World.Sys_Music.inCombat && !World.Sys_Music.inZone && World.Sys_Music.combatImpulseFinished < World.pauseRelativeTime) {
        World.Sys_Music.inCombat = false;
        PlayTrack(TrackType_Combat, MusicType_Override);
        World.Sys_Music.combatImpulseFinished = World.pauseRelativeTime + 20.0;
        return;
    }
    if (World.Sys_Music.inZone) {
        if (World.Sys_Music.distortion) { PlayTrack(TrackType_Distortion, MusicType_Override); return; }
        if (World.Sys_Music.elevator) { PlayTrack(TrackType_Elevator, MusicType_Override); return; }
    }
    if (Sys_Settings.DynamicMusic || remaining <= AUD_BUFFER_T) PlayTrack(TrackType_Walking, MusicType_Walking);
}

void ResetLevelMusic(void) { mp3_clear(); World.Sys_Music.levelEntry = true; World.Sys_Music.inZone = World.Sys_Music.cyberTube = false; World.Sys_Music.clipFinished = World.Sys_Music.combatImpulseFinished = get_time(); World.Sys_Music.combatImpulseFinished += 5.0; }
