// audio.c - Audio System
#include "os.h"
#include <pthread.h>
#include "common.h"
#include "interop.h"
#define AUDIO_RATE      48000
#define AUDIO_CHANNELS  2
#define AUDIO_PERIOD_MS 10
#define AUDIO_PERIODS   4
#define AUDIO_FRAMES    ((AUDIO_RATE * AUDIO_PERIOD_MS) / 1000)
void* CopyMemoryFromBtoAForNBytes(void *dst, const void *src, size_t n);
extern SettingsSystem Sys_Settings; extern GlobalContext Sys_Global;
#ifdef WINDOWS
    typedef u32 snd_pcm_uframes_t; // match nanoalsa
    #define PCM_NONBLOCK (1<<1)
    #define PCM_FORMAT_S16_LE 2
    #define PCM_ACCESS_RW     3
    typedef struct { int format,access,rate,channels,period_frames,periods; } pcm_params_t;
    typedef struct { snd_pcm_uframes_t hw_ptr; }  pcm_status_t;
    typedef struct { snd_pcm_uframes_t appl_ptr; } pcm_control_t;
    typedef struct { pcm_status_t status; pcm_control_t control; } pcm_sync_t;
    typedef enum { PCM_FORMAT=0,PCM_ACCESS,PCM_RATE,PCM_CHANNELS,PCM_PERIOD_SIZE,PCM_PERIODS,PCM_INTERRUPT } pcm_param_t;
    typedef struct {IAudioClient *client; IAudioRenderClient *render; u32 buffer_frames; i32 rate,channels,period_frames; bool open; } wasapi_dev_t;
    static wasapi_dev_t wasapi_devs[8/*MAX_WASAPI_DEVICES*/];
    static int wasapi_dev_count = 0;
    #define FD_TO_IDX(fd) ((int)(intptr_t)(fd)-100)
    #define IDX_TO_FD(i)  ((OsFileHandle)(intptr_t)((i)+100))
    static const IID IID_IAudioClient_         = {0x1CB9AD4C,0xDBFA,0x4C32,{0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2}};
    static const IID IID_IAudioRenderClient_   = {0xF294ACFC,0x3146,0x4483,{0xA7,0xBF,0xAD,0xDC,0xA7,0xC2,0x60,0xE2}};
    static int wasapi_init_device(IMMDevice *dev,int rate,int channels,int period_frames,int periods) {
        if (wasapi_dev_count>=8/*MAX_WASAPI_DEVICES*/) return -1;
        wasapi_dev_t *w = &wasapi_devs[wasapi_dev_count];
        HRESULT hr = dev->lpVtbl->Activate(dev,&IID_IAudioClient_,CLSCTX_ALL,NULL,(void**)&w->client);
        if (FAILED(hr)) return -1;
        WAVEFORMATEX fmt = {WAVE_FORMAT_PCM,(WORD)channels,(DWORD)rate,(DWORD)(rate*channels*2),(WORD)(channels*2),16,0};
        REFERENCE_TIME buf_dur = (REFERENCE_TIME)(period_frames*periods)*10000000LL/rate;
        hr = w->client->lpVtbl->Initialize(w->client,AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_NOPERSIST,buf_dur,0,&fmt,NULL);
        if (FAILED(hr)) { w->client->lpVtbl->Release(w->client); return -1; }
        w->client->lpVtbl->GetBufferSize(w->client,&w->buffer_frames);
        hr = w->client->lpVtbl->GetService(w->client,&IID_IAudioRenderClient_,(void**)&w->render);
        if (FAILED(hr)) { w->client->lpVtbl->Release(w->client); return -1; }
        w->client->lpVtbl->Start(w->client);
        w->rate=rate; w->channels=channels; w->period_frames=period_frames; w->open=true;
        return wasapi_dev_count++;
    }
    
    static const CLSID CLSID_MMDeviceEnumerator_ = {0xBCDE0395,0xE52F,0x467C,{0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E}};
    static const IID IID_IMMDeviceEnumerator_  = {0xA95664D2,0x9614,0x4F35,{0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6}};
    OsFileHandle pcm_open_all(int rate,int channels,int period_frames,int periods) {
        CoInitializeEx(NULL,COINIT_MULTITHREADED);
        IMMDeviceEnumerator *en = NULL;
        if (FAILED(CoCreateInstance(&CLSID_MMDeviceEnumerator_,NULL,CLSCTX_ALL,&IID_IMMDeviceEnumerator_,(void**)&en))) return OS_INVALID_HANDLE;
        IMMDevice *dev = NULL;
        HRESULT hr = en->lpVtbl->GetDefaultAudioEndpoint(en,eRender,eConsole,&dev);
        en->lpVtbl->Release(en);
        if (FAILED(hr)||!dev) return OS_INVALID_HANDLE;
        int idx = wasapi_init_device(dev,rate,channels,period_frames,periods);
        dev->lpVtbl->Release(dev);
        if (idx<0) return OS_INVALID_HANDLE;
        DualLog("Audio: WASAPI default device opened\n");
        return IDX_TO_FD(0);
    }

    void pcm_params_init(pcm_params_t *p) { MemSetToValueForNBytes(p,0,sizeof(*p)); }
    void pcm_set(pcm_params_t *p,pcm_param_t param,unsigned long v) {
        switch(param) {
        case PCM_FORMAT:      p->format=v; break;       case PCM_ACCESS:   p->access=v; break;
        case PCM_RATE:        p->rate=v; break;          case PCM_CHANNELS: p->channels=v; break;
        case PCM_PERIOD_SIZE: p->period_frames=v; break; case PCM_PERIODS:  p->periods=v; break;
        default: break;
        }
    }

    OsFileHandle pcm_open(int card,int dev,int flags) { (void)card;(void)dev;(void)flags; return OS_INVALID_HANDLE; }
    int pcm_params_setup(OsFileHandle fd,pcm_params_t *p) { (void)fd;(void)p; return -1; }
    int pcm_params_refine(OsFileHandle fd,pcm_params_t *p) { (void)fd;(void)p; return 0; }

    int pcm_sync(OsFileHandle fd,pcm_sync_t *sync,unsigned int flags) {
        (void)flags;
        int idx=FD_TO_IDX(fd);
        if (idx<0||idx>=wasapi_dev_count||!wasapi_devs[idx].open) return -1;
        wasapi_dev_t *w=&wasapi_devs[idx];
        UINT32 padding=0; w->client->lpVtbl->GetCurrentPadding(w->client,&padding);
        snd_pcm_uframes_t base = (w->buffer_frames>(UINT32)(w->period_frames*4)) ? w->buffer_frames-(UINT32)(w->period_frames*4) : 0;
        sync->status.hw_ptr=base; sync->control.appl_ptr=base+padding;
        return 0;
    }

    int pcm_prepare(OsFileHandle fd) {
        int idx=FD_TO_IDX(fd); if (idx<0||idx>=wasapi_dev_count) return -1;
        wasapi_dev_t *w=&wasapi_devs[idx];
        w->client->lpVtbl->Stop(w->client); w->client->lpVtbl->Reset(w->client); w->client->lpVtbl->Start(w->client); return 0;
    }

    int pcm_write(OsFileHandle fd,void *buf,int frames) {
        (void)fd;
        for (int i=0;i<wasapi_dev_count;i++) {
            wasapi_dev_t *w=&wasapi_devs[i]; if (!w->open) continue;
            BYTE *data=NULL;
            if (FAILED(w->render->lpVtbl->GetBuffer(w->render,(UINT32)frames,&data))) { pcm_prepare(IDX_TO_FD(i)); continue; }
            CopyMemoryFromBtoAForNBytes(data,buf,frames*w->channels*2);
            w->render->lpVtbl->ReleaseBuffer(w->render,(UINT32)frames,0);
        }
        return frames;
    }

    int pcm_start(OsFileHandle fd)   { (void)fd; return 0; }
    int pcm_stop(OsFileHandle fd)    { int i=FD_TO_IDX(fd); if(i>=0&&i<wasapi_dev_count) wasapi_devs[i].client->lpVtbl->Stop(wasapi_devs[i].client); return 0; }
    int pcm_drain(OsFileHandle fd)   { (void)fd; return 0; }
    int pcm_pause(OsFileHandle fd)   { return pcm_stop(fd); }
    int pcm_unpause(OsFileHandle fd) { int i=FD_TO_IDX(fd); if(i>=0&&i<wasapi_dev_count) wasapi_devs[i].client->lpVtbl->Start(wasapi_devs[i].client); return 0; }
#else
    typedef struct snd_pcm_mmap_status  pcm_status_t;
    typedef struct snd_pcm_mmap_control pcm_control_t;
    struct pcm_sync { pcm_status_t status; pcm_control_t control; };
    typedef struct pcm_sync pcm_sync_t;
    typedef struct snd_pcm_hw_params pcm_hw_params_t;
    typedef struct snd_pcm_sw_params pcm_sw_params_t;
    struct pcm_params { pcm_hw_params_t hw_params; pcm_sw_params_t sw_params; };
    typedef struct pcm_params pcm_params_t;
    enum pcm_clock_type_t { PCM_CLOCK_REALTIME = SNDRV_PCM_TSTAMP_TYPE_GETTIMEOFDAY, PCM_CLOCK_MONOTONIC = SNDRV_PCM_TSTAMP_TYPE_MONOTONIC, PCM_CLOCK_MONOTONIC_RAW = SNDRV_PCM_TSTAMP_TYPE_MONOTONIC_RAW};
    typedef enum pcm_clock_type_t pcm_clock_type_t;
    enum pcm_param {
        PCM_ACCESS       = SNDRV_PCM_HW_PARAM_ACCESS,       // mask (pcm_access_t)
        PCM_FORMAT       = SNDRV_PCM_HW_PARAM_FORMAT,       // mask (pcm_format_t)
        PCM_RATE         = SNDRV_PCM_HW_PARAM_RATE,         // interval
        PCM_CHANNELS     = SNDRV_PCM_HW_PARAM_CHANNELS,     // interval
        PCM_PERIOD_SIZE  = SNDRV_PCM_HW_PARAM_PERIOD_SIZE,  // interval
        PCM_BUFFER_SIZE  = SNDRV_PCM_HW_PARAM_BUFFER_SIZE,  // interval
        PCM_PERIODS      = SNDRV_PCM_HW_PARAM_PERIODS,      // interval (variant of BUFFER_SIZE)
        PCM_INTERRUPT         = SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 1, // flag (in hw_params)
        PCM_TSTAMP_TYPE       = SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 2, // value (in sw_params)
        PCM_AVAIL_MIN         = SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 3, // value (in sw_params)
        PCM_START_THRESHOLD   = SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 4, // value (in sw_params)
        PCM_XRUN_THRESHOLD    = SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 5, // value (in sw_params)
        PCM_SILENCE_THRESHOLD = SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 6, // value (in sw_params)
        PCM_SILENCE_SIZE      = SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 7, // value (in sw_params)
    };
    typedef enum pcm_param pcm_param_t;
    static inline int pcm_prepare(int fd) { return OS_IOControlSimple(fd,SNDRV_PCM_IOCTL_PREPARE); }
    static inline int pcm_write(int fd, void *buf, int frames) { struct snd_xferi tmp={.buf=buf,.frames=frames,.result=0}; return OS_IOControl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES,(void*)&tmp) ? -1 : (int) tmp.result; }
    static void hw_params_set_mask(struct snd_pcm_hw_params *p, int parameter, unsigned int value);
    static void hw_params_set_interval(struct snd_pcm_hw_params *p, int parameter, unsigned int min, unsigned int max);
    static void hw_params_set(struct snd_pcm_hw_params *p, int parameter, unsigned int value);
    static unsigned int hw_params_get_mask(struct snd_pcm_hw_params *p, int parameter, unsigned int value);
    static void hw_params_get_interval(struct snd_pcm_hw_params *p, int parameter, unsigned int *min, unsigned int *max);
    static unsigned int hw_params_get(struct snd_pcm_hw_params *p, int parameter, unsigned int value);
    static void hw_params_fill(struct snd_pcm_hw_params *p);
    #define get_index(i)       ((i) / 32)
    #define get_mask(i)  (1 << ((i) % 32))
    #define is_mask(parameter) (parameter >= SNDRV_PCM_HW_PARAM_FIRST_MASK && parameter <= SNDRV_PCM_HW_PARAM_LAST_MASK)
    #define is_interval(parameter) (parameter >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL && parameter <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL)
    static inline struct snd_mask* get_mask_struct(struct snd_pcm_hw_params *p, unsigned int parameter) { return &p->masks[parameter - SNDRV_PCM_HW_PARAM_FIRST_MASK]; }
    static inline struct snd_interval* get_interval_struct(struct snd_pcm_hw_params *p, unsigned int parameter) { return &p->intervals[parameter - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]; }
    static void hw_params_set_mask(struct snd_pcm_hw_params *p, int parameter, unsigned int value) { struct snd_mask *m = get_mask_struct(p,parameter); if (m->bits[get_index(value)] & get_mask(value)) {MemSetToValueForNBytes(m, 0x00, sizeof(*m));} m->bits[get_index(value)] |= get_mask(value); }
    static void hw_params_set_interval(struct snd_pcm_hw_params *p, int parameter, unsigned int min, unsigned int max) { struct snd_interval *i = get_interval_struct(p,parameter); i->openmin = i->openmax = 0; i->integer = 1; i->min = min; i->max = max; }
    static void hw_params_set(struct snd_pcm_hw_params *p, int parameter, unsigned int value) {
        if (is_mask(parameter)) hw_params_set_mask(p,parameter,value);
        else if (is_interval(parameter)) hw_params_set_interval(p,parameter,value,value);
    }

    static unsigned int hw_params_get_mask(struct snd_pcm_hw_params *p, int parameter, unsigned int value) { struct snd_mask *m=get_mask_struct(p,parameter); return m->bits[get_index(value)]&get_mask(value); }
    static void hw_params_get_interval(struct snd_pcm_hw_params *p, int parameter, unsigned int *min, unsigned int *max) {
        struct snd_interval *i = get_interval_struct(p, parameter);
        *min = i->min + i->openmin;
        *max = i->max - i->openmax;
    }

    static unsigned int hw_params_get(struct snd_pcm_hw_params *p, int parameter, unsigned int value) {
        unsigned int ret, tmp;
        if (is_mask(parameter)) ret = hw_params_get_mask(p, parameter, value);
        else if (is_interval(parameter)) hw_params_get_interval(p, parameter, &ret, &tmp);
        else return 0;
        return ret;
    }

    static const int INTERVAL_COUNT = SNDRV_PCM_HW_PARAM_LAST_INTERVAL - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL;
    static void hw_params_fill(struct snd_pcm_hw_params *p) {
        int i;
        MemSetToValueForNBytes(p,0,sizeof(*p));
        MemSetToValueForNBytes(p->masks,0xff,sizeof(p->masks));
        for (i = 0; i <= INTERVAL_COUNT; i++) { p->intervals[i].min = 0; p->intervals[i].max = UINT_MAX; }
        p->rmask = p->info = UINT_MAX;
        p->cmask = 0;
        p->msbits = 0;   // sample bits
        p->rate_num = 0; // rate
        p->rate_den = 0; // always 1
    }

    int pcm_sync(int fd, struct pcm_sync *sync, unsigned int flags) {
        struct snd_pcm_sync_ptr tmp;
        flags^=SNDRV_PCM_SYNC_PTR_APPL|SNDRV_PCM_SYNC_PTR_AVAIL_MIN;
        tmp.flags=flags;
        if (OS_IOControl(fd,SNDRV_PCM_IOCTL_SYNC_PTR,(void*)&tmp) == -1) return -1;
        sync->control = tmp.c.control; sync->status = tmp.s.status;
        return 0;
    }

    int pcm_action_timestamp(int fd, struct timespec *ts) {
        struct snd_pcm_status status;
        if (OS_IOControl(fd, SNDRV_PCM_IOCTL_STATUS,(void*)&status) == -1) return -1;
        *ts = status.trigger_tstamp;
        return 0;
    }

    void pcm_params_init(pcm_params_t *p) {
        hw_params_fill(&p->hw_params);
        pcm_sw_params_t *sw = &p->sw_params;
        MemSetToValueForNBytes(sw,0,sizeof(*sw));
        sw->start_threshold = 1; sw->period_step = 1;
    }

    void pcm_set(pcm_params_t *params, pcm_param_t parameter, unsigned long value) {
        pcm_hw_params_t *hw = &params->hw_params; pcm_sw_params_t *sw = &params->sw_params;
        switch (parameter) {
        default: hw_params_set(hw, parameter, value); break;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 1: hw->flags = value ? hw->flags|SNDRV_PCM_HW_PARAMS_NO_PERIOD_WAKEUP : hw->flags & ~SNDRV_PCM_HW_PARAMS_NO_PERIOD_WAKEUP; break;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 2: sw->tstamp_mode = SNDRV_PCM_TSTAMP_ENABLE; sw->tstamp_type = value; break;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 3:         sw->avail_min         = value; break;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 4:   sw->start_threshold   = value; break;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 5:    sw->stop_threshold    = value; break;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 6: sw->silence_threshold = value; break;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 7:      sw->silence_size      = value; break;
        }
    }

    void pcm_set_range(pcm_params_t *params, pcm_param_t parameter, unsigned int min, unsigned int max) {
        if (parameter <= SNDRV_PCM_HW_PARAM_LAST_MASK || parameter > SNDRV_PCM_HW_PARAM_LAST_INTERVAL) pcm_set(params,parameter,min);
        else hw_params_set_interval(&params->hw_params,parameter,min,max);
    }

    unsigned long pcm_get(pcm_params_t *params, pcm_param_t parameter, unsigned int value) {
        pcm_hw_params_t *hw = &params->hw_params;
        pcm_sw_params_t *sw = &params->sw_params;
        switch (parameter) {
        default:                    return hw_params_get(hw, parameter, value);
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 1:         return hw->flags & SNDRV_PCM_HW_PARAMS_NO_PERIOD_WAKEUP;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 2:       return sw->tstamp_mode ? sw->tstamp_type : UINT_MAX;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 3:         return sw->avail_min;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 4:   return sw->start_threshold;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 5:    return sw->stop_threshold;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 6: return sw->silence_threshold;
        case SNDRV_PCM_HW_PARAM_LAST_INTERVAL + 7:      return sw->silence_size;
        }
    }

    void pcm_get_range(pcm_params_t *params, pcm_param_t parameter, unsigned int *min, unsigned int *max) { hw_params_get_interval(&params->hw_params,parameter,min,max); }
//     static inline unsigned int pcm_get_min(pcm_params_t *params, pcm_param_t parameter) { unsigned int min,max; pcm_get_range(params,parameter,&min,&max); return min; }
//     static inline unsigned int pcm_get_max(pcm_params_t *params, pcm_param_t parameter) { unsigned int min,max; pcm_get_range(params,parameter,&min,&max); return max; }
    int pcm_params_refine(int fd, pcm_params_t *params) { return ioctl(fd,SNDRV_PCM_IOCTL_HW_REFINE,&params->hw_params); }
    int pcm_params_setup(int fd, pcm_params_t *params) {
        if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &params->hw_params) == -1) return -1;
        if (!pcm_get(params, PCM_AVAIL_MIN, 0)) pcm_set(params, PCM_AVAIL_MIN, pcm_get(params, PCM_PERIOD_SIZE, 0));
        if (!pcm_get(params, PCM_XRUN_THRESHOLD, 0)) pcm_set(params, PCM_XRUN_THRESHOLD, pcm_get(params, PCM_BUFFER_SIZE, 0));
        if (ioctl(fd, SNDRV_PCM_IOCTL_TTSTAMP, &params->sw_params.tstamp_type) == -1) return -1; // Support ancient kernels
        if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, &params->sw_params) == -1) return -1;
        return ioctl(fd, SNDRV_PCM_IOCTL_PREPARE);
    }

    int pcm_open(int card, int device, int flags) {
        char path[4096];
        StringFormat(path,sizeof(path),"/dev/snd/pcmC%uD%u%c",card,device,(flags&1)==0 ? 'c' : 'p');
        return OS_Open(path,O_RDWR|(flags&(1 << 1)?O_NONBLOCK:0),0);
    }
#endif

// MP3 parsing
#define DRMP3_UINT64_MAX            (((u64)0xFFFFFFFF << 32) | (u64)0xFFFFFFFF)
#define DRMP3_MAX_PCM_FRAMES_PER_MP3_FRAME  1152
#define DRMP3_MAX_SAMPLES_PER_FRAME         (DRMP3_MAX_PCM_FRAMES_PER_MP3_FRAME * 2)
#define DRMP3_HDR_SIZE                      4
#define DRMP3_HDR_IS_MONO(h)             (((h[3]) & 0xC0) == 0xC0)
#define DRMP3_HDR_IS_MS_STEREO(h)        (((h[3]) & 0xE0) == 0x60)
#define DRMP3_HDR_IS_FREE_FORMAT(h)      (((h[2]) & 0xF0) == 0)
#define DRMP3_HDR_IS_CRC(h)              (!((h[1]) & 1))
#define DRMP3_HDR_TEST_PADDING(h)        ((h[2]) & 0x2)
#define DRMP3_HDR_TEST_MPEG1(h)          ((h[1]) & 0x8)
#define DRMP3_HDR_TEST_NOT_MPEG25(h)     ((h[1]) & 0x10)
#define DRMP3_HDR_TEST_I_STEREO(h)       ((h[3]) & 0x10)
#define DRMP3_HDR_TEST_MS_STEREO(h)      ((h[3]) & 0x20)
#define DRMP3_HDR_GET_STEREO_MODE(h)     (((h[3]) >> 6) & 3)
#define DRMP3_HDR_GET_STEREO_MODE_EXT(h) (((h[3]) >> 4) & 3)
#define DRMP3_HDR_GET_LAYER(h)           (((h[1]) >> 1) & 3)
#define DRMP3_HDR_GET_BITRATE(h)         ((h[2]) >> 4)
#define DRMP3_HDR_GET_SAMPLE_RATE(h)     (((h[2]) >> 2) & 3)
#define DRMP3_HDR_GET_MY_SAMPLE_RATE(h)  (DRMP3_HDR_GET_SAMPLE_RATE(h) + (((h[1]>>3)&1)+((h[1]>>4)&1))*3)
#define DRMP3_HDR_IS_FRAME_576(h)        ((h[1] & 14) == 2)
#define DRMP3_HDR_IS_LAYER_1(h)          ((h[1] & 6) == 6)
#define DRMP3_COPY_MEMORY(dst,src,sz) CopyMemoryFromBtoAForNBytes((dst),(src),(sz))
#define DRMP3_MOVE_MEMORY(dst,src,sz) MoveMemoryFromBtoAForNBytes((dst),(src),(sz))
#define DRMP3_ZERO_MEMORY(p,sz)       MemSetToValueForNBytes((p),0,(sz))
#define DRMP3_ZERO_OBJECT(p)          DRMP3_ZERO_MEMORY((p),sizeof(*(p)))
#define DRMP3_OFFSET_PTR(p,offset) ((void*)((u8*)(p)+(offset)))
typedef enum { DRMP3_SEEK_SET, DRMP3_SEEK_CUR, DRMP3_SEEK_END } drmp3_seek_origin;
typedef struct { int frame_bytes, channels, sample_rate, layer, bitrate_kbps; } drmp3dec_frame_info;
typedef struct { const u8 *buf; int pos,limit; } drmp3_bs;
typedef struct { const u8 *sfbtab; u16 part_23_length,big_values,scalefac_compress; u8 global_gain,block_type,mixed_block_flag,n_long_sfb,n_short_sfb,table_select[3],region_count[3],subblock_gain[3],preflag,scalefac_scale,count1_table,scfsi; } drmp3_L3_gr_info;
typedef struct { drmp3_bs bs; u8 maindata[511 + 2304]; drmp3_L3_gr_info gr_info[4]; float grbuf[2][576],scf[40],syn[18+15][2*32]; u8 ist_pos[2][39]; } drmp3dec_scratch;
typedef struct { float mdct_overlap[2][9*32], qmf_state[15*2*32]; int reserv,free_format_bytes; u8 header[4],reserv_buf[511]; drmp3dec_scratch scratch; } drmp3dec;
typedef size_t (*drmp3_read_proc)(void*, void*, size_t);
typedef bool (*drmp3_seek_proc)(void*, int, drmp3_seek_origin);
typedef struct {
    drmp3dec decoder;
    u32 channels,sampleRate,mp3FrameChannels,mp3FrameSampleRate,pcmFramesConsumedInMP3Frame,pcmFramesRemainingInMP3Frame,delayInPCMFrames,paddingInPCMFrames;
    drmp3_read_proc onRead;
    drmp3_seek_proc onSeek;
    void *pUserData;
    u8 pcmFrames[sizeof(float)*DRMP3_MAX_SAMPLES_PER_FRAME];
    u64 currentPCMFrame,streamCursor,streamLength,streamStartOffset,totalPCMFrameCount;
    bool atEnd;
    size_t dataSize,dataCapacity,dataConsumed;
    u8 *pData;
} drmp3;

static u32 drmp3_bs_get_bits(drmp3_bs *bs, int n) {
    u32 next,cache=0, s=bs->pos&7; int shl=n+s;
    const u8 *p=bs->buf+(bs->pos>>3);
    if ((bs->pos+=n)>bs->limit) return 0;
    
    next=*p++&(255>>s);
    while ((shl-=8)>0) { cache|=next<<shl; next=*p++; }
    return cache|(next>>-shl);
}

static int drmp3_hdr_valid(const u8 *h) { return h[0]==0xff && ((h[1]&0xF0)==0xf0||(h[1]&0xFE)==0xe2) && (DRMP3_HDR_GET_LAYER(h)!=0) && (DRMP3_HDR_GET_BITRATE(h)!=15) && (DRMP3_HDR_GET_SAMPLE_RATE(h)!=3); }
static int drmp3_hdr_compare(const u8 *h1, const u8 *h2) { return drmp3_hdr_valid(h2) && ((h1[1]^h2[1])&0xFE)==0 && ((h1[2]^h2[2])&0x0C)==0 && !(DRMP3_HDR_IS_FREE_FORMAT(h1)^DRMP3_HDR_IS_FREE_FORMAT(h2)); }
static unsigned drmp3_hdr_bitrate_kbps(const u8 *h) {
    static const u8 halfrate[2][3][15]={
        {{0,4,8,12,16,20,24,28,32,40,48,56,64,72,80},{0,4,8,12,16,20,24,28,32,40,48,56,64,72,80},{0,16,24,28,32,40,48,56,64,72,80,88,96,112,128}},
        {{0,16,20,24,28,32,40,48,56,64,80,96,112,128,160},{0,16,24,28,32,40,48,56,64,80,96,112,128,160,192},{0,16,32,48,64,80,96,112,128,144,160,176,192,208,224}}
    };
    return 2*halfrate[!!DRMP3_HDR_TEST_MPEG1(h)][DRMP3_HDR_GET_LAYER(h)-1][DRMP3_HDR_GET_BITRATE(h)];
}
static unsigned drmp3_hdr_sample_rate_hz(const u8 *h) { static const unsigned g_hz[3]={44100,48000,32000}; return g_hz[DRMP3_HDR_GET_SAMPLE_RATE(h)]>>(int)!DRMP3_HDR_TEST_MPEG1(h)>>(int)!DRMP3_HDR_TEST_NOT_MPEG25(h); }
static unsigned drmp3_hdr_frame_samples(const u8 *h) { return ((h[1]&6) == 6) ? 384 : (1152>>(int)DRMP3_HDR_IS_FRAME_576(h)); }
static int drmp3_hdr_frame_bytes(const u8 *h, int free_format_size) { int fb=drmp3_hdr_frame_samples(h)*drmp3_hdr_bitrate_kbps(h)*125/drmp3_hdr_sample_rate_hz(h); if (DRMP3_HDR_IS_LAYER_1(h)) {fb&=~3;} return fb?fb:free_format_size; }
static int drmp3_hdr_padding(const u8 *h) { return DRMP3_HDR_TEST_PADDING(h)?(DRMP3_HDR_IS_LAYER_1(h)?4:1):0; }
#define SCF_START 4,4,4,4,4,4,6,6
#define SCF_MIXED_ROW_A 8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0
#define SCF_SHORT_ROW_A 4,4,4,SCF_START,6,SCF_MIXED_ROW_A
#define SCF_LONG_ROW_A 6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54,0
static int drmp3_L3_read_side_info(drmp3_bs *bs, drmp3_L3_gr_info *gr, const u8 *hdr) {
    static const u8 g_scf_long[8][23]={
        {SCF_LONG_ROW_A},{12,12,12,12,12,12,16,20,24,28,32,40,48,56,64,76,90,2,2,2,2,2,0},{SCF_LONG_ROW_A},{6,6,6,6,6,6,8,10,12,14,16,18,22,26,32,38,46,54,62,70,76,36,0},
        {SCF_LONG_ROW_A},{SCF_START,8,8,10,12,16,20,24,28,34,42,50,54,76,158,0},{SCF_START,6,8,10,12,16,18,22,28,34,40,46,54,54,192,0},{SCF_START,8,10,12,16,20,24,30,38,46,56,68,84,102,26,0}
    };
    static const u8 g_scf_short[8][40]={ {SCF_SHORT_ROW_A},{8,8,8,8,8,8,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0},
        {4,4,4,SCF_START,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0},
        {4,4,4,SCF_START,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0},
        {SCF_SHORT_ROW_A}, {4,4,4,4,4,4,SCF_START,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0},
        {4,4,4,4,4,4,SCF_START,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0},
        {4,4,4,4,4,4,SCF_START,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0}
    };
    static const u8 g_scf_mixed[8][40]={
        {6,6,6,6,6,6,6,6,6,SCF_MIXED_ROW_A}, {12,12,12,4,4,4,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0},
        {6,6,6,6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0},
        {6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0},
        {6,6,6,6,6,6,6,6,6,SCF_MIXED_ROW_A}, {SCF_START,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0},
        {SCF_START,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0},
        {SCF_START,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0}
    };
    unsigned tables, scfsi=0;
    int main_data_begin, part_23_sum=0;
    int gr_count=DRMP3_HDR_IS_MONO(hdr)?1:2;
    int sr_idx=DRMP3_HDR_GET_MY_SAMPLE_RATE(hdr); sr_idx-=(sr_idx!=0);
    if (DRMP3_HDR_TEST_MPEG1(hdr)) {
        gr_count*=2;
        main_data_begin=drmp3_bs_get_bits(bs,9);
        scfsi=drmp3_bs_get_bits(bs,7+gr_count);
    } else main_data_begin=drmp3_bs_get_bits(bs,8+gr_count)>>gr_count;
    do {
        if (DRMP3_HDR_IS_MONO(hdr)) scfsi<<=4;
        gr->part_23_length=(u16)drmp3_bs_get_bits(bs,12);
        part_23_sum+=gr->part_23_length;
        gr->big_values=(u16)drmp3_bs_get_bits(bs,9);
        if (gr->big_values>288) return -1;
        gr->global_gain=(u8)drmp3_bs_get_bits(bs,8);
        gr->scalefac_compress=(u16)drmp3_bs_get_bits(bs,DRMP3_HDR_TEST_MPEG1(hdr)?4:9);
        gr->sfbtab=g_scf_long[sr_idx];
        gr->n_long_sfb=22; gr->n_short_sfb=0;
        if (drmp3_bs_get_bits(bs,1)) {
            gr->block_type=(u8)drmp3_bs_get_bits(bs,2);
            if (!gr->block_type) return -1;
            gr->mixed_block_flag=(u8)drmp3_bs_get_bits(bs,1);
            gr->region_count[0]=7; gr->region_count[1]=255;
            if (gr->block_type==2) {
                scfsi&=0x0F0F;
                if (!gr->mixed_block_flag) {
                    gr->region_count[0]=8;
                    gr->sfbtab=g_scf_short[sr_idx];
                    gr->n_long_sfb=0; gr->n_short_sfb=39;
                } else {
                    gr->sfbtab=g_scf_mixed[sr_idx];
                    gr->n_long_sfb=DRMP3_HDR_TEST_MPEG1(hdr)?8:6;
                    gr->n_short_sfb=30;
                }
            }
            tables=drmp3_bs_get_bits(bs,10)<<5;
            gr->subblock_gain[0]=(u8)drmp3_bs_get_bits(bs,3);
            gr->subblock_gain[1]=(u8)drmp3_bs_get_bits(bs,3);
            gr->subblock_gain[2]=(u8)drmp3_bs_get_bits(bs,3);
        } else {
            gr->block_type=0; gr->mixed_block_flag=0;
            tables=drmp3_bs_get_bits(bs,15);
            gr->region_count[0]=(u8)drmp3_bs_get_bits(bs,4);
            gr->region_count[1]=(u8)drmp3_bs_get_bits(bs,3);
            gr->region_count[2]=255;
        }
        gr->table_select[0]=(u8)(tables>>10);
        gr->table_select[1]=(u8)((tables>>5)&31);
        gr->table_select[2]=(u8)((tables)&31);
        gr->preflag=(u8)(DRMP3_HDR_TEST_MPEG1(hdr)?drmp3_bs_get_bits(bs,1):(gr->scalefac_compress>=500));
        gr->scalefac_scale=(u8)drmp3_bs_get_bits(bs,1);
        gr->count1_table=(u8)drmp3_bs_get_bits(bs,1);
        gr->scfsi=(u8)((scfsi>>12)&15);
        scfsi<<=4; gr++;
    } while(--gr_count);
    if (part_23_sum+bs->pos > bs->limit+main_data_begin*8) return -1;
    return main_data_begin;
}

static void drmp3_L3_read_scalefactors(u8 *scf, u8 *ist_pos, const u8 *scf_size, const u8 *scf_count, drmp3_bs *bs, int scfsi) {
    int i,k;
    for (i=0; i<4&&scf_count[i]; i++,scfsi*=2) {
        int cnt=scf_count[i];
        if (scfsi&8) CopyMemoryFromBtoAForNBytes(scf,ist_pos,cnt);
        else {
            int bits=scf_size[i];
            if (!bits) { DRMP3_ZERO_MEMORY(scf,cnt); DRMP3_ZERO_MEMORY(ist_pos,cnt); }
            else {
                int max_scf=(scfsi<0)?((1<<bits)-1):-1;
                for (k=0;k<cnt;k++) {
                    int s=drmp3_bs_get_bits(bs,bits);
                    ist_pos[k]=(u8)(s==max_scf?-1:s);
                    scf[k]=(u8)s;
                }
            }
        }
        ist_pos+=cnt; scf+=cnt;
    }
    scf[0]=scf[1]=scf[2]=0;
}

static float drmp3_L3_ldexp_q2(float y, int exp_q2) {
    static const float g_expfrac[4]={9.31322575e-10f,7.83145814e-10f,6.58544508e-10f,5.53767716e-10f};
    int e;
    do { e=vmin(30*4,exp_q2); y*=g_expfrac[e&3]*(1<<30>>(e>>2)); } while ((exp_q2-=e)>0);
    return y;
}

#define DRMP3_BITS_DEQUANTIZER_OUT -1
#define DRMP3_MAX_SCF  (255+DRMP3_BITS_DEQUANTIZER_OUT*4-210)
#define DRMP3_MAX_SCFI ((DRMP3_MAX_SCF+3)&~3)
static void drmp3_L3_decode_scalefactors(const u8 *hdr, u8 *ist_pos, drmp3_bs *bs, const drmp3_L3_gr_info *gr, float *scf, int ch) {
    static const u8 g_scf_partitions[3][28]={
        {6,5,5,5,6,5,5,5,6,5,7,3,11,10,0,0,7,7,7,0,6,6,6,3,8,8,5,0},
        {8,9,6,12,6,9,9,9,6,9,12,6,15,18,0,0,6,15,12,0,6,12,9,6,6,18,9,0},
        {9,9,6,12,9,9,9,9,9,9,12,6,18,18,0,0,12,12,12,0,12,9,9,6,15,12,9,0}
    };
    const u8 *scf_partition=g_scf_partitions[!!gr->n_short_sfb+!gr->n_long_sfb];
    u8 scf_size[4],iscf[40];
    int i,scf_shift=gr->scalefac_scale+1,gain_exp,scfsi=gr->scfsi;
    float gain;
    if (DRMP3_HDR_TEST_MPEG1(hdr)) {
        static const u8 g_scfc_decode[16]={0,1,2,3,12,5,6,7,9,10,11,13,14,15,18,19};
        int part=g_scfc_decode[gr->scalefac_compress];
        scf_size[1]=scf_size[0]=(u8)(part>>2);
        scf_size[3]=scf_size[2]=(u8)(part&3);
    } else {
        static const u8 g_mod[6*4]={5,5,4,4,5,5,4,1,4,3,1,1,5,6,6,1,4,4,4,1,4,3,1,1};
        int k,modprod,sfc,ist=DRMP3_HDR_TEST_I_STEREO(hdr)&&ch;
        sfc=gr->scalefac_compress>>ist;
        for (k=ist*3*4; sfc>=0; sfc-=modprod,k+=4) {
            for (modprod=1,i=3;i>=0;i--) { scf_size[i]=(u8)(sfc/modprod%g_mod[k+i]); modprod*=g_mod[k+i]; }
        }
        scf_partition+=k; scfsi=-16;
    }
    drmp3_L3_read_scalefactors(iscf,ist_pos,scf_size,scf_partition,bs,scfsi);
    if (gr->n_short_sfb) {
        int sh=3-scf_shift;
        for (i=0;i<gr->n_short_sfb;i+=3) {
            iscf[gr->n_long_sfb+i+0]=(u8)(iscf[gr->n_long_sfb+i+0]+(gr->subblock_gain[0]<<sh));
            iscf[gr->n_long_sfb+i+1]=(u8)(iscf[gr->n_long_sfb+i+1]+(gr->subblock_gain[1]<<sh));
            iscf[gr->n_long_sfb+i+2]=(u8)(iscf[gr->n_long_sfb+i+2]+(gr->subblock_gain[2]<<sh));
        }
    } else if (gr->preflag) {
        static const u8 g_preamp[10]={1,1,1,1,2,2,3,3,3,2};
        for (i=0;i<10;i++) iscf[11+i]=(u8)(iscf[11+i]+g_preamp[i]);
    }
    gain_exp=gr->global_gain+DRMP3_BITS_DEQUANTIZER_OUT*4-210-(DRMP3_HDR_IS_MS_STEREO(hdr)?2:0);
    gain=drmp3_L3_ldexp_q2(1<<(DRMP3_MAX_SCFI/4),DRMP3_MAX_SCFI-gain_exp);
    for (i=0;i<(int)(gr->n_long_sfb+gr->n_short_sfb);i++) scf[i]=drmp3_L3_ldexp_q2(gain,iscf[i]<<scf_shift);
}

static const float g_drmp3_pow43[129+16]={
    0,-1,-2.519842f,-4.326749f,-6.349604f,-8.549880f,-10.902724f,-13.390518f,-16.000000f,-18.720754f,-21.544347f,-24.463781f,-27.473142f,-30.567351f,-33.741992f,-36.993181f,
    0,1,2.519842f,4.326749f,6.349604f,8.549880f,10.902724f,13.390518f,16.000000f,18.720754f,21.544347f,24.463781f,27.473142f,30.567351f,33.741992f,36.993181f,40.317474f,43.711787f,47.173345f,50.699631f,54.288352f,57.937408f,61.644865f,65.408941f,69.227979f,73.100443f,77.024898f,81.000000f,85.024491f,89.097188f,93.216975f,97.382800f,101.593667f,105.848633f,110.146801f,114.487321f,118.869381f,123.292209f,127.755065f,132.257246f,136.798076f,141.376907f,145.993119f,150.646117f,155.335327f,160.060199f,164.820202f,169.614826f,174.443577f,179.305980f,184.201575f,189.129918f,194.090580f,199.083145f,204.107210f,209.162385f,214.248292f,219.364564f,224.510845f,229.686789f,234.892058f,240.126328f,245.389280f,250.680604f,256.000000f,261.347174f,266.721841f,272.123723f,277.552547f,283.008049f,288.489971f,293.998060f,299.532071f,305.091761f,310.676898f,316.287249f,321.922592f,327.582707f,333.267377f,338.976394f,344.709550f,350.466646f,356.247482f,362.051866f,367.879608f,373.730522f,379.604427f,385.501143f,391.420496f,397.362314f,403.326427f,409.312672f,415.320884f,421.350905f,427.402579f,433.475750f,439.570269f,445.685987f,451.822757f,457.980436f,464.158883f,470.357960f,476.577530f,482.817459f,489.077615f,495.357868f,501.658090f,507.978156f,514.317941f,520.677324f,527.056184f,533.454404f,539.871867f,546.308458f,552.764065f,559.238575f,565.731879f,572.243870f,578.774440f,585.323483f,591.890898f,598.476581f,605.080431f,611.702349f,618.342238f,625.000000f,631.675540f,638.368763f,645.079578f
};
static float drmp3_L3_pow_43(int x) {
    float frac; int sign,mult=256;
    if (x<129) return g_drmp3_pow43[16+x];
    if (x<1024) { mult=16; x<<=3; }
    sign=2*x&64;
    frac=(float)((x&63)-sign)/((x&~63)+sign);
    return g_drmp3_pow43[16+((x+sign)>>6)]*(1.f+frac*((4.f/3)+frac*(2.f/9)))*mult;
}

static void drmp3_L3_huffman(float *dst, drmp3_bs *bs, const drmp3_L3_gr_info *gr_info, const float *scf, int layer3gr_limit) {
    static const i16 tabs[]={ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        785,785,785,785,784,784,784,784,513,513,513,513,513,513,513,513,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,
        -255,1313,1298,1282,785,785,785,785,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,290,288,
        -255,1313,1298,1282,769,769,769,769,529,529,529,529,529,529,529,529,528,528,528,528,528,528,528,528,512,512,512,512,512,512,512,512,290,288,
        -253,-318,-351,-367,785,785,785,785,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,819,818,547,547,275,275,275,275,561,560,515,546,289,274,288,258,
        -254,-287,1329,1299,1314,1312,1057,1057,1042,1042,1026,1026,784,784,784,784,529,529,529,529,529,529,529,529,769,769,769,769,768,768,768,768,563,560,306,306,291,259,
        -252,-413,-477,-542,1298,-575,1041,1041,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-383,-399,1107,1092,1106,1061,849,849,789,789,1104,1091,773,773,1076,1075,341,340,325,309,834,804,577,577,532,532,516,516,832,818,803,816,561,561,531,531,515,546,289,289,288,258,
        -252,-429,-493,-559,1057,1057,1042,1042,529,529,529,529,529,529,529,529,784,784,784,784,769,769,769,769,512,512,512,512,512,512,512,512,-382,1077,-415,1106,1061,1104,849,849,789,789,1091,1076,1029,1075,834,834,597,581,340,340,339,324,804,833,532,532,832,772,818,803,817,787,816,771,290,290,290,290,288,258,
        -253,-349,-414,-447,-463,1329,1299,-479,1314,1312,1057,1057,1042,1042,1026,1026,785,785,785,785,784,784,784,784,769,769,769,769,768,768,768,768,-319,851,821,-335,836,850,805,849,341,340,325,336,533,533,579,579,564,564,773,832,578,548,563,516,321,276,306,291,304,259,
        -251,-572,-733,-830,-863,-879,1041,1041,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-511,-527,-543,1396,1351,1381,1366,1395,1335,1380,-559,1334,1138,1138,1063,1063,1350,1392,1031,1031,1062,1062,1364,1363,1120,1120,1333,1348,881,881,881,881,375,374,359,373,343,358,341,325,791,791,1123,1122,-703,1105,1045,-719,865,865,790,790,774,774,1104,1029,338,293,323,308,-799,-815,833,788,772,818,803,816,322,292,307,320,561,531,515,546,289,274,288,258,
        -251,-525,-605,-685,-765,-831,-846,1298,1057,1057,1312,1282,785,785,785,785,784,784,784,784,769,769,769,769,512,512,512,512,512,512,512,512,1399,1398,1383,1367,1382,1396,1351,-511,1381,1366,1139,1139,1079,1079,1124,1124,1364,1349,1363,1333,882,882,882,882,807,807,807,807,1094,1094,1136,1136,373,341,535,535,881,775,867,822,774,-591,324,338,-671,849,550,550,866,864,609,609,293,336,534,534,789,835,773,-751,834,804,308,307,833,788,832,772,562,562,547,547,305,275,560,515,290,290,
        -252,-397,-477,-557,-622,-653,-719,-735,-750,1329,1299,1314,1057,1057,1042,1042,1312,1282,1024,1024,785,785,785,785,784,784,784,784,769,769,769,769,-383,1127,1141,1111,1126,1140,1095,1110,869,869,883,883,1079,1109,882,882,375,374,807,868,838,881,791,-463,867,822,368,263,852,837,836,-543,610,610,550,550,352,336,534,534,865,774,851,821,850,805,593,533,579,564,773,832,578,578,548,548,577,577,307,276,306,291,516,560,259,259,
        -250,-2107,-2507,-2764,-2909,-2974,-3007,-3023,1041,1041,1040,1040,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-767,-1052,-1213,-1277,-1358,-1405,-1469,-1535,-1550,-1582,-1614,-1647,-1662,-1694,-1726,-1759,-1774,-1807,-1822,-1854,-1886,1565,-1919,-1935,-1951,-1967,1731,1730,1580,1717,-1983,1729,1564,-1999,1548,-2015,-2031,1715,1595,-2047,1714,-2063,1610,-2079,1609,-2095,1323,1323,1457,1457,1307,1307,1712,1547,1641,1700,1699,1594,1685,1625,1442,1442,1322,1322,-780,-973,-910,1279,1278,1277,1262,1276,1261,1275,1215,1260,1229,-959,974,974,989,989,-943,735,478,478,495,463,506,414,-1039,1003,958,1017,927,942,987,957,431,476,1272,1167,1228,-1183,1256,-1199,895,895,941,941,1242,1227,1212,1135,1014,1014,490,489,503,487,910,1013,985,925,863,894,970,955,1012,847,-1343,831,755,755,984,909,428,366,754,559,-1391,752,486,457,924,997,698,698,983,893,740,740,908,877,739,739,667,667,953,938,497,287,271,271,683,606,590,712,726,574,302,302,738,736,481,286,526,725,605,711,636,724,696,651,589,681,666,710,364,467,573,695,466,466,301,465,379,379,709,604,665,679,316,316,634,633,436,436,464,269,424,394,452,332,438,363,347,408,393,448,331,422,362,407,392,421,346,406,391,376,375,359,1441,1306,-2367,1290,-2383,1337,-2399,-2415,1426,1321,-2431,1411,1336,-2447,-2463,-2479,1169,1169,1049,1049,1424,1289,1412,1352,1319,-2495,1154,1154,1064,1064,1153,1153,416,390,360,404,403,389,344,374,373,343,358,372,327,357,342,311,356,326,1395,1394,1137,1137,1047,1047,1365,1392,1287,1379,1334,1364,1349,1378,1318,1363,792,792,792,792,1152,1152,1032,1032,1121,1121,1046,1046,1120,1120,1030,1030,-2895,1106,1061,1104,849,849,789,789,1091,1076,1029,1090,1060,1075,833,833,309,324,532,532,832,772,818,803,561,561,531,560,515,546,289,274,288,258,
        -250,-1179,-1579,-1836,-1996,-2124,-2253,-2333,-2413,-2477,-2542,-2574,-2607,-2622,-2655,1314,1313,1298,1312,1282,785,785,785,785,1040,1040,1025,1025,768,768,768,768,-766,-798,-830,-862,-895,-911,-927,-943,-959,-975,-991,-1007,-1023,-1039,-1055,-1070,1724,1647,-1103,-1119,1631,1767,1662,1738,1708,1723,-1135,1780,1615,1779,1599,1677,1646,1778,1583,-1151,1777,1567,1737,1692,1765,1722,1707,1630,1751,1661,1764,1614,1736,1676,1763,1750,1645,1598,1721,1691,1762,1706,1582,1761,1566,-1167,1749,1629,767,766,751,765,494,494,735,764,719,749,734,763,447,447,748,718,477,506,431,491,446,476,461,505,415,430,475,445,504,399,460,489,414,503,383,474,429,459,502,502,746,752,488,398,501,473,413,472,486,271,480,270,-1439,-1455,1357,-1471,-1487,-1503,1341,1325,-1519,1489,1463,1403,1309,-1535,1372,1448,1418,1476,1356,1462,1387,-1551,1475,1340,1447,1402,1386,-1567,1068,1068,1474,1461,455,380,468,440,395,425,410,454,364,467,466,464,453,269,409,448,268,432,1371,1473,1432,1417,1308,1460,1355,1446,1459,1431,1083,1083,1401,1416,1458,1445,1067,1067,1370,1457,1051,1051,1291,1430,1385,1444,1354,1415,1400,1443,1082,1082,1173,1113,1186,1066,1185,1050,-1967,1158,1128,1172,1097,1171,1081,-1983,1157,1112,416,266,375,400,1170,1142,1127,1065,793,793,1169,1033,1156,1096,1141,1111,1155,1080,1126,1140,898,898,808,808,897,897,792,792,1095,1152,1032,1125,1110,1139,1079,1124,882,807,838,881,853,791,-2319,867,368,263,822,852,837,866,806,865,-2399,851,352,262,534,534,821,836,594,594,549,549,593,593,533,533,848,773,579,579,564,578,548,563,276,276,577,576,306,291,516,560,305,305,275,259,
        -251,-892,-2058,-2620,-2828,-2957,-3023,-3039,1041,1041,1040,1040,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-511,-527,-543,-559,1530,-575,-591,1528,1527,1407,1526,1391,1023,1023,1023,1023,1525,1375,1268,1268,1103,1103,1087,1087,1039,1039,1523,-604,815,815,815,815,510,495,509,479,508,463,507,447,431,505,415,399,-734,-782,1262,-815,1259,1244,-831,1258,1228,-847,-863,1196,-879,1253,987,987,748,-767,493,493,462,477,414,414,686,669,478,446,461,445,474,429,487,458,412,471,1266,1264,1009,1009,799,799,-1019,-1276,-1452,-1581,-1677,-1757,-1821,-1886,-1933,-1997,1257,1257,1483,1468,1512,1422,1497,1406,1467,1496,1421,1510,1134,1134,1225,1225,1466,1451,1374,1405,1252,1252,1358,1480,1164,1164,1251,1251,1238,1238,1389,1465,-1407,1054,1101,-1423,1207,-1439,830,830,1248,1038,1237,1117,1223,1148,1236,1208,411,426,395,410,379,269,1193,1222,1132,1235,1221,1116,976,976,1192,1162,1177,1220,1131,1191,963,963,-1647,961,780,-1663,558,558,994,993,437,408,393,407,829,978,813,797,947,-1743,721,721,377,392,844,950,828,890,706,706,812,859,796,960,948,843,934,874,571,571,-1919,690,555,689,421,346,539,539,944,779,918,873,932,842,903,888,570,570,931,917,674,674,-2575,1562,-2591,1609,-2607,1654,1322,1322,1441,1441,1696,1546,1683,1593,1669,1624,1426,1426,1321,1321,1639,1680,1425,1425,1305,1305,1545,1668,1608,1623,1667,1592,1638,1666,1320,1320,1652,1607,1409,1409,1304,1304,1288,1288,1664,1637,1395,1395,1335,1335,1622,1636,1394,1394,1319,1319,1606,1621,1392,1392,1137,1137,1137,1137,345,390,360,375,404,373,1047,-2751,-2767,-2783,1062,1121,1046,-2799,1077,-2815,1106,1061,789,789,1105,1104,263,355,310,340,325,354,352,262,339,324,1091,1076,1029,1090,1060,1075,833,833,788,788,1088,1028,818,818,803,803,561,561,531,531,816,771,546,546,289,274,288,258,
        -253,-317,-381,-446,-478,-509,1279,1279,-811,-1179,-1451,-1756,-1900,-2028,-2189,-2253,-2333,-2414,-2445,-2511,-2526,1313,1298,-2559,1041,1041,1040,1040,1025,1025,1024,1024,1022,1007,1021,991,1020,975,1019,959,687,687,1018,1017,671,671,655,655,1016,1015,639,639,758,758,623,623,757,607,756,591,755,575,754,559,543,543,1009,783,-575,-621,-685,-749,496,-590,750,749,734,748,974,989,1003,958,988,973,1002,942,987,957,972,1001,926,986,941,971,956,1000,910,985,925,999,894,970,-1071,-1087,-1102,1390,-1135,1436,1509,1451,1374,-1151,1405,1358,1480,1420,-1167,1507,1494,1389,1342,1465,1435,1450,1326,1505,1310,1493,1373,1479,1404,1492,1464,1419,428,443,472,397,736,526,464,464,486,457,442,471,484,482,1357,1449,1434,1478,1388,1491,1341,1490,1325,1489,1463,1403,1309,1477,1372,1448,1418,1433,1476,1356,1462,1387,-1439,1475,1340,1447,1402,1474,1324,1461,1371,1473,269,448,1432,1417,1308,1460,-1711,1459,-1727,1441,1099,1099,1446,1386,1431,1401,-1743,1289,1083,1083,1160,1160,1458,1445,1067,1067,1370,1457,1307,1430,1129,1129,1098,1098,268,432,267,416,266,400,-1887,1144,1187,1082,1173,1113,1186,1066,1050,1158,1128,1143,1172,1097,1171,1081,420,391,1157,1112,1170,1142,1127,1065,1169,1049,1156,1096,1141,1111,1155,1080,1126,1154,1064,1153,1140,1095,1048,-2159,1125,1110,1137,-2175,823,823,1139,1138,807,807,384,264,368,263,868,838,853,791,867,822,852,837,866,806,865,790,-2319,851,821,836,352,262,850,805,849,-2399,533,533,835,820,336,261,578,548,563,577,532,532,832,772,562,562,547,547,305,275,560,515,290,290,288,258
    };
    static const u8 tab32[]={130,162,193,209,44,28,76,140,9,9,9,9,9,9,9,9,190,254,222,238,126,94,157,157,109,61,173,205};
    static const u8 tab33[]={252,236,220,204,188,172,156,140,124,108,92,76,60,44,28,12};
    static const i16 tabindex[2*16]={0,32,64,98,0,132,180,218,292,364,426,538,648,746,0,1126,1460,1460,1460,1460,1460,1460,1460,1460,1842,1842,1842,1842,1842,1842,1842,1842};
    static const u8 g_linbits[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,6,8,10,13,4,5,6,7,8,9,11,13};
#define DRMP3_PEEK_BITS(n)  (bs_cache>>(32-(n)))
#define DRMP3_FLUSH_BITS(n) { bs_cache<<=(n); bs_sh+=(n); }
#define DRMP3_CHECK_BITS    while(bs_sh>=0){bs_cache|=(u32)*bs_next_ptr++<<bs_sh;bs_sh-=8;}
#define DRMP3_BSPOS         ((bs_next_ptr-bs->buf)*8-24+bs_sh)
    float one=0.0f;
    int ireg=0,big_val_cnt=gr_info->big_values;
    const u8 *sfb=gr_info->sfbtab;
    const u8 *bs_next_ptr=bs->buf+bs->pos/8;
    u32 bs_cache=(((bs_next_ptr[0]*256u+bs_next_ptr[1])*256u+bs_next_ptr[2])*256u+bs_next_ptr[3])<<(bs->pos&7);
    int pairs_to_decode,np,bs_sh=(bs->pos&7)-8;
    bs_next_ptr+=4;
    while (big_val_cnt>0) {
        int tab_num=gr_info->table_select[ireg];
        int sfb_cnt=gr_info->region_count[ireg++];
        const i16 *codebook=tabs+tabindex[tab_num];
        int linbits=g_linbits[tab_num];
        if (linbits) {
            do {
                np=*sfb++/2; pairs_to_decode=vmin(big_val_cnt,np); one=*scf++;
                do {
                    int j,w=5,leaf=codebook[DRMP3_PEEK_BITS(w)];
                    while (leaf<0){DRMP3_FLUSH_BITS(w);w=leaf&7;leaf=codebook[DRMP3_PEEK_BITS(w)-(leaf>>3)];}
                    DRMP3_FLUSH_BITS(leaf>>8);
                    for (j=0;j<2;j++,dst++,leaf>>=4){
                        int lsb=leaf&0x0F;
                        if (lsb==15){lsb+=DRMP3_PEEK_BITS(linbits);DRMP3_FLUSH_BITS(linbits);DRMP3_CHECK_BITS;*dst=one*drmp3_L3_pow_43(lsb)*((i32)bs_cache<0?-1:1);}
                        else *dst=g_drmp3_pow43[16+lsb-16*(bs_cache>>31)]*one;
                        DRMP3_FLUSH_BITS(lsb?1:0);
                    }
                    DRMP3_CHECK_BITS;
                } while(--pairs_to_decode);
            } while((big_val_cnt-=np)>0&&--sfb_cnt>=0);
        } else {
            do {
                np=*sfb++/2; pairs_to_decode=vmin(big_val_cnt,np); one=*scf++;
                do {
                    int j,w=5,leaf=codebook[DRMP3_PEEK_BITS(w)];
                    while (leaf<0){DRMP3_FLUSH_BITS(w);w=leaf&7;leaf=codebook[DRMP3_PEEK_BITS(w)-(leaf>>3)];}
                    DRMP3_FLUSH_BITS(leaf>>8);
                    for (j=0;j<2;j++,dst++,leaf>>=4){
                        int lsb=leaf&0x0F;
                        *dst=g_drmp3_pow43[16+lsb-16*(bs_cache>>31)]*one;
                        DRMP3_FLUSH_BITS(lsb?1:0);
                    }
                    DRMP3_CHECK_BITS;
                } while(--pairs_to_decode);
            } while((big_val_cnt-=np)>0&&--sfb_cnt>=0);
        }
    }
    for (np=1-big_val_cnt;;dst+=4) {
        const u8 *codebook_count1=(gr_info->count1_table)?tab33:tab32;
        int leaf=codebook_count1[DRMP3_PEEK_BITS(4)];
        if (!(leaf&8)) leaf=codebook_count1[(leaf>>3)+(bs_cache<<4>>(32-(leaf&3)))];
        DRMP3_FLUSH_BITS(leaf&7);
        if (DRMP3_BSPOS>layer3gr_limit) break;
#define DRMP3_RELOAD_SCALEFACTOR if(!--np){np=*sfb++/2;if(!np)break;one=*scf++;}
#define DRMP3_DEQ_COUNT1(s) if(leaf&(128>>s)){dst[s]=((i32)bs_cache<0)?-one:one;DRMP3_FLUSH_BITS(1)}
        DRMP3_RELOAD_SCALEFACTOR; DRMP3_DEQ_COUNT1(0); DRMP3_DEQ_COUNT1(1);
        DRMP3_RELOAD_SCALEFACTOR; DRMP3_DEQ_COUNT1(2); DRMP3_DEQ_COUNT1(3);
        DRMP3_CHECK_BITS;
    }
    bs->pos=layer3gr_limit;
}

static void drmp3_L3_midside_stereo(float *left, int n) {
    int i=0; float *right=left+576;
    for (; i<n; i++) { float a=left[i],b=right[i]; left[i]=a+b; right[i]=a-b; }
}
static void drmp3_L3_intensity_stereo_band(float *left, int n, float kl, float kr)
{ int i; for(i=0;i<n;i++){left[i+576]=left[i]*kr;left[i]=left[i]*kl;} }
static void drmp3_L3_stereo_top_band(const float *right, const u8 *sfb, int nbands, int max_band[3]) {
    int i,k; max_band[0]=max_band[1]=max_band[2]=-1;
    for (i=0;i<nbands;i++){for(k=0;k<sfb[i];k+=2){if(right[k]!=0||right[k+1]!=0){max_band[i%3]=i;break;}}right+=sfb[i];}
}
static void drmp3_L3_stereo_process(float *left, const u8 *ist_pos, const u8 *sfb, const u8 *hdr, int max_band[3], int mpeg2_sh) {
    static const float g_pan[7*2]={0,1,0.21132487f,0.78867513f,0.36602540f,0.63397460f,0.5f,0.5f,0.63397460f,0.36602540f,0.78867513f,0.21132487f,1,0};
    unsigned i,max_pos=DRMP3_HDR_TEST_MPEG1(hdr)?7:64;
    for (i=0;sfb[i];i++){
        unsigned ipos=ist_pos[i];
        if ((int)i>max_band[i%3]&&ipos<max_pos){
            float kl,kr,s=DRMP3_HDR_TEST_MS_STEREO(hdr)?1.41421356f:1;
            if(DRMP3_HDR_TEST_MPEG1(hdr)){kl=g_pan[2*ipos];kr=g_pan[2*ipos+1];}
            else{kl=1;kr=drmp3_L3_ldexp_q2(1,(ipos+1)>>1<<mpeg2_sh);if(ipos&1){kl=kr;kr=1;}}
            drmp3_L3_intensity_stereo_band(left,sfb[i],kl*s,kr*s);
        } else if (DRMP3_HDR_TEST_MS_STEREO(hdr)) drmp3_L3_midside_stereo(left,sfb[i]);
        left+=sfb[i];
    }
}
static void drmp3_L3_intensity_stereo(float *left, u8 *ist_pos, const drmp3_L3_gr_info *gr, const u8 *hdr) {
    int max_band[3],n_sfb=gr->n_long_sfb+gr->n_short_sfb,i,max_blocks=gr->n_short_sfb?3:1;
    drmp3_L3_stereo_top_band(left+576,gr->sfbtab,n_sfb,max_band);
    if (gr->n_long_sfb) max_band[0]=max_band[1]=max_band[2]=vmax(vmax(max_band[0],max_band[1]),max_band[2]);
    for (i=0;i<max_blocks;i++){
        int default_pos=DRMP3_HDR_TEST_MPEG1(hdr)?3:0,itop=n_sfb-max_blocks+i,prev=itop-max_blocks;
        ist_pos[itop]=(u8)(max_band[i]>=prev?default_pos:ist_pos[prev]);
    }
    drmp3_L3_stereo_process(left,ist_pos,gr->sfbtab,hdr,max_band,gr[1].scalefac_compress&1);
}

static void drmp3_L3_reorder(float *grbuf, float *scratch, const u8 *sfb) {
    int i,len; float *src=grbuf,*dst=scratch;
    for(;0!=(len=*sfb);sfb+=3,src+=2*len){for(i=0;i<len;i++,src++){*dst++=src[0*len];*dst++=src[1*len];*dst++=src[2*len];}}
    DRMP3_COPY_MEMORY(grbuf,scratch,(dst-scratch)*sizeof(float));
}
static void drmp3_L3_antialias(float *grbuf, int nbands) {
    static const float g_aa[2][8]={{0.85749293f,0.88174200f,0.94962865f,0.98331459f,0.99551782f,0.99916056f,0.99989920f,0.99999316f},{0.51449576f,0.47173197f,0.31337745f,0.18191320f,0.09457419f,0.04096558f,0.01419856f,0.00369997f}};
    for(;nbands>0;nbands--,grbuf+=18){
        int i=0;
        for(;i<8;i++){float u=grbuf[18+i],d=grbuf[17-i];grbuf[18+i]=u*g_aa[0][i]-d*g_aa[1][i];grbuf[17-i]=u*g_aa[1][i]+d*g_aa[0][i];}
    }
}
static void drmp3_L3_dct3_9(float *y) {
    float s0,s1,s2,s3,s4,s5,s6,s7,s8,t0,t2,t4;
    s0=y[0];s2=y[2];s4=y[4];s6=y[6];s8=y[8];
    t0=s0+s6*0.5f;s0-=s6;t4=(s4+s2)*0.93969262f;t2=(s8+s2)*0.76604444f;
    s6=(s4-s8)*0.17364818f;s4+=s8-s2;s2=s0-s4*0.5f;y[4]=s4+s0;
    s8=t0-t2+s6;s0=t0-t4+t2;s4=t0+t4-s6;
    s1=y[1];s3=y[3];s5=y[5];s7=y[7];
    s3*=0.86602540f;t0=(s5+s1)*0.98480775f;t4=(s5-s7)*0.34202014f;t2=(s1+s7)*0.64278761f;
    s1=(s1-s5-s7)*0.86602540f;s5=t0-s3-t2;s7=t4-s3-t0;s3=t4+s3-t2;
    y[0]=s4-s7;y[1]=s2+s1;y[2]=s0-s3;y[3]=s8+s5;y[5]=s8-s5;y[6]=s0+s3;y[7]=s2-s1;y[8]=s4+s7;
}
static void drmp3_L3_imdct36(float *grbuf, float *overlap, const float *window, int nbands) {
    int i,j;
    static const float g_twid9[18]={0.73727734f,0.79335334f,0.84339145f,0.88701083f,0.92387953f,0.95371695f,0.97629601f,0.99144486f,0.99904822f,0.67559021f,0.60876143f,0.53729961f,0.46174861f,0.38268343f,0.30070580f,0.21643961f,0.13052619f,0.04361938f};
    for (j=0;j<nbands;j++,grbuf+=18,overlap+=9){
        float co[9],si[9];
        co[0]=-grbuf[0]; si[0]=grbuf[17];
        for(i=0;i<4;i++){si[8-2*i]=grbuf[4*i+1]-grbuf[4*i+2];co[1+2*i]=grbuf[4*i+1]+grbuf[4*i+2];si[7-2*i]=grbuf[4*i+4]-grbuf[4*i+3];co[2+2*i]=-(grbuf[4*i+3]+grbuf[4*i+4]);}
        drmp3_L3_dct3_9(co); drmp3_L3_dct3_9(si);
        si[1]=-si[1];si[3]=-si[3];si[5]=-si[5];si[7]=-si[7];
        i=0;
        for(;i<9;i++){float ovl=overlap[i],sum=co[i]*g_twid9[9+i]+si[i]*g_twid9[i];overlap[i]=co[i]*g_twid9[i]-si[i]*g_twid9[9+i];grbuf[i]=ovl*window[i]-sum*window[9+i];grbuf[17-i]=ovl*window[9+i]+sum*window[i];}
    }
}
static void drmp3_L3_idct3(float x0,float x1,float x2,float *dst){float m1=x1*0.86602540f,a1=x0-x2*0.5f;dst[1]=x0+x2;dst[0]=a1+m1;dst[2]=a1-m1;}
static void drmp3_L3_imdct12(float *x,float *dst,float *overlap){
    static const float g_twid3[6]={0.79335334f,0.92387953f,0.99144486f,0.60876143f,0.38268343f,0.13052619f};
    float co[3],si[3]; int i;
    drmp3_L3_idct3(-x[0],x[6]+x[3],x[12]+x[9],co);
    drmp3_L3_idct3(x[15],x[12]-x[9],x[6]-x[3],si);
    si[1]=-si[1];
    for(i=0;i<3;i++){float ovl=overlap[i],sum=co[i]*g_twid3[3+i]+si[i]*g_twid3[i];overlap[i]=co[i]*g_twid3[i]-si[i]*g_twid3[3+i];dst[i]=ovl*g_twid3[2-i]-sum*g_twid3[5-i];dst[5-i]=ovl*g_twid3[5-i]+sum*g_twid3[2-i];}
}
static void drmp3_L3_imdct_short(float *grbuf,float *overlap,int nbands){
    for(;nbands>0;nbands--,overlap+=9,grbuf+=18){float tmp[18];DRMP3_COPY_MEMORY(tmp,grbuf,sizeof(tmp));DRMP3_COPY_MEMORY(grbuf,overlap,6*sizeof(float));drmp3_L3_imdct12(tmp,grbuf+6,overlap+6);drmp3_L3_imdct12(tmp+1,grbuf+12,overlap+6);drmp3_L3_imdct12(tmp+2,overlap,overlap+6);}
}
static void drmp3_L3_change_sign(float *grbuf){int b,i;for(b=0,grbuf+=18;b<32;b+=2,grbuf+=36)for(i=1;i<18;i+=2)grbuf[i]=-grbuf[i];}
static void drmp3_L3_imdct_gr(float *grbuf,float *overlap,unsigned block_type,unsigned n_long_bands){
    static const float g_mdct_window[2][18]={
        {0.99904822f,0.99144486f,0.97629601f,0.95371695f,0.92387953f,0.88701083f,0.84339145f,0.79335334f,0.73727734f,0.04361938f,0.13052619f,0.21643961f,0.30070580f,0.38268343f,0.46174861f,0.53729961f,0.60876143f,0.67559021f},
        {1,1,1,1,1,1,0.99144486f,0.92387953f,0.79335334f,0,0,0,0,0,0,0.13052619f,0.38268343f,0.60876143f}
    };
    if (n_long_bands){drmp3_L3_imdct36(grbuf,overlap,g_mdct_window[0],n_long_bands);grbuf+=18*n_long_bands;overlap+=9*n_long_bands;}
    if (block_type==2) drmp3_L3_imdct_short(grbuf,overlap,32-n_long_bands);
    else drmp3_L3_imdct36(grbuf,overlap,g_mdct_window[block_type==3],32-n_long_bands);
}

static void drmp3_L3_save_reservoir(drmp3dec *h, drmp3dec_scratch *s){
    int pos=(s->bs.pos+7)/8u,remains=s->bs.limit/8u-pos;
    if (remains>511){pos+=remains-511;remains=511;}
    if (remains>0) DRMP3_MOVE_MEMORY(h->reserv_buf,s->maindata+pos,remains);
    h->reserv=remains;
}
static int drmp3_L3_restore_reservoir(drmp3dec *h, drmp3_bs *bs, drmp3dec_scratch *s, int main_data_begin){
    int frame_bytes=(bs->limit-bs->pos)/8,bytes_have=vmin(h->reserv,main_data_begin);
    DRMP3_COPY_MEMORY(s->maindata,h->reserv_buf+vmax(0,h->reserv-main_data_begin),vmin(h->reserv,main_data_begin));
    DRMP3_COPY_MEMORY(s->maindata+bytes_have,bs->buf+bs->pos/8,frame_bytes);
    s->bs.buf=s->maindata; s->bs.pos=0; s->bs.limit=(bytes_have+frame_bytes) * 8;
    return h->reserv>=main_data_begin;
}

static void drmp3_L3_decode(drmp3dec *h, drmp3dec_scratch *s, drmp3_L3_gr_info *gr_info, int nch){
    int ch;
    for(ch=0;ch<nch;ch++){
        int limit=s->bs.pos+gr_info[ch].part_23_length;
        drmp3_L3_decode_scalefactors(h->header,s->ist_pos[ch],&s->bs,gr_info+ch,s->scf,ch);
        drmp3_L3_huffman(s->grbuf[ch],&s->bs,gr_info+ch,s->scf,limit);
    }
    if (DRMP3_HDR_TEST_I_STEREO(h->header)) drmp3_L3_intensity_stereo(s->grbuf[0],s->ist_pos[1],gr_info,h->header);
    else if (DRMP3_HDR_IS_MS_STEREO(h->header)) drmp3_L3_midside_stereo(s->grbuf[0],576);
    for(ch=0;ch<nch;ch++,gr_info++){
        int aa_bands=31,n_long_bands=(gr_info->mixed_block_flag?2:0)<<(int)(DRMP3_HDR_GET_MY_SAMPLE_RATE(h->header)==2);
        if (gr_info->n_short_sfb){aa_bands=n_long_bands-1;drmp3_L3_reorder(s->grbuf[ch]+n_long_bands*18,s->syn[0],gr_info->sfbtab+gr_info->n_long_sfb);}
        drmp3_L3_antialias(s->grbuf[ch],aa_bands);
        drmp3_L3_imdct_gr(s->grbuf[ch],h->mdct_overlap[ch],gr_info->block_type,n_long_bands);
        drmp3_L3_change_sign(s->grbuf[ch]);
    }
}

static void drmp3d_DCT_II(float *grbuf, int n){
    static const float g_sec[24]={10.19000816f,0.50060302f,0.50241929f,3.40760851f,0.50547093f,0.52249861f,2.05778098f,0.51544732f,0.56694406f,1.48416460f,0.53104258f,0.64682180f,1.16943991f,0.55310392f,0.78815460f,0.97256821f,0.58293498f,1.06067765f,0.83934963f,0.62250412f,1.72244716f,0.74453628f,0.67480832f,5.10114861f};
    int i,k=0;
    for(;k<n;k++){
        float t[4][8],*x,*y=grbuf+k;
        for(x=t[0],i=0;i<8;i++,x++){float x0=y[i*18],x1=y[(15-i)*18],x2=y[(16+i)*18],x3=y[(31-i)*18],t0=x0+x3,t1=x1+x2,t2=(x1-x2)*g_sec[3*i],t3=(x0-x3)*g_sec[3*i+1];x[0]=t0+t1;x[8]=(t0-t1)*g_sec[3*i+2];x[16]=t3+t2;x[24]=(t3-t2)*g_sec[3*i+2];}
        for(x=t[0],i=0;i<4;i++,x+=8){float x0=x[0],x1=x[1],x2=x[2],x3=x[3],x4=x[4],x5=x[5],x6=x[6],x7=x[7],xt;xt=x0-x7;x0+=x7;x7=x1-x6;x1+=x6;x6=x2-x5;x2+=x5;x5=x3-x4;x3+=x4;x4=x0-x3;x0+=x3;x3=x1-x2;x1+=x2;x[0]=x0+x1;x[4]=(x0-x1)*0.70710677f;x5+=x6;x6=(x6+x7)*0.70710677f;x7+=xt;x3=(x3+x4)*0.70710677f;x5-=x7*0.198912367f;x7+=x5*0.382683432f;x5-=x7*0.198912367f;x0=xt-x6;xt+=x6;x[1]=(xt+x7)*0.50979561f;x[2]=(x4+x3)*0.54119611f;x[3]=(x0-x5)*0.60134488f;x[5]=(x0+x5)*0.89997619f;x[6]=(x4-x3)*1.30656302f;x[7]=(xt-x7)*2.56291556f;}
        for(i=0;i<7;i++,y+=4*18){y[0]=t[0][i];y[18]=t[2][i]+t[3][i]+t[3][i+1];y[36]=t[1][i]+t[1][i+1];y[54]=t[2][i+1]+t[3][i]+t[3][i+1];}
        y[0]=t[0][7];y[18]=t[2][7]+t[3][7];y[36]=t[1][7];y[54]=t[3][7];
    }
}

static float drmp3d_scale_pcm(float sample) { return sample*(1.f/32768.f); }
typedef float drmp3d_sample_t;

static void drmp3d_synth_pair(drmp3d_sample_t *pcm, int nch, const float *z){
    float a;
    a =(z[14*64]-z[    0])*29; a+=(z[ 1*64]+z[13*64])*213; a+=(z[12*64]-z[ 2*64])*459;
    a+=(z[ 3*64]+z[11*64])*2037; a+=(z[10*64]-z[ 4*64])*5153; a+=(z[ 5*64]+z[ 9*64])*6574;
    a+=(z[ 8*64]-z[ 6*64])*37489; a+=z[7*64]*75038;
    pcm[0]=drmp3d_scale_pcm(a);
    z+=2;
    a =z[14*64]*104; a+=z[12*64]*1567; a+=z[10*64]*9727; a+=z[8*64]*64019;
    a+=z[6*64]*-9975; a+=z[4*64]*-45; a+=z[2*64]*146; a+=z[0*64]*-5;
    pcm[16*nch]=drmp3d_scale_pcm(a);
}
static void drmp3d_synth(float *xl, drmp3d_sample_t *dstl, int nch, float *lins){
    int i; float *xr=xl+576*(nch-1); drmp3d_sample_t *dstr=dstl+(nch-1);
    static const float g_win[]={
        -1,26,-31,208,218,401,-519,2063,2000,4788,-5517,7134,5959,35640,-39336,74992,
        -1,24,-35,202,222,347,-581,2080,1952,4425,-5879,7640,5288,33791,-41176,74856,
        -1,21,-38,196,225,294,-645,2087,1893,4063,-6237,8092,4561,31947,-43006,74630,
        -1,19,-41,190,227,244,-711,2085,1822,3705,-6589,8492,3776,30112,-44821,74313,
        -1,17,-45,183,228,197,-779,2075,1739,3351,-6935,8840,2935,28289,-46617,73908,
        -1,16,-49,176,228,153,-848,2057,1644,3004,-7271,9139,2037,26482,-48390,73415,
        -2,14,-53,169,227,111,-919,2032,1535,2663,-7597,9389,1082,24694,-50137,72835,
        -2,13,-58,161,224,72,-991,2001,1414,2330,-7910,9592,70,22929,-51853,72169,
        -2,11,-63,154,221,36,-1064,1962,1280,2006,-8209,9750,-998,21189,-53534,71420,
        -2,10,-68,147,215,2,-1137,1919,1131,1692,-8491,9863,-2122,19478,-55178,70590,
        -3,9,-73,139,208,-29,-1210,1870,970,1388,-8755,9935,-3300,17799,-56778,69679,
        -3,8,-79,132,200,-57,-1283,1817,794,1095,-8998,9966,-4533,16155,-58333,68692,
        -4,7,-85,125,189,-83,-1356,1759,605,814,-9219,9959,-5818,14548,-59838,67629,
        -4,7,-91,117,177,-106,-1428,1698,402,545,-9416,9916,-7154,12980,-61289,66494,
        -5,6,-97,111,163,-127,-1498,1634,185,288,-9585,9838,-8540,11455,-62684,65290
    };
    float *zlin=lins+15*64; const float *w=g_win;
    zlin[4*15]=xl[18*16];zlin[4*15+1]=xr[18*16];zlin[4*15+2]=xl[0];zlin[4*15+3]=xr[0];
    zlin[4*31]=xl[1+18*16];zlin[4*31+1]=xr[1+18*16];zlin[4*31+2]=xl[1];zlin[4*31+3]=xr[1];
    drmp3d_synth_pair(dstr,nch,lins+4*15+1);drmp3d_synth_pair(dstr+32*nch,nch,lins+4*15+64+1);
    drmp3d_synth_pair(dstl,nch,lins+4*15);drmp3d_synth_pair(dstl+32*nch,nch,lins+4*15+64);
    for(i=14;i>=0;i--){
#define DRMP3_LOAD(k) float w0=*w++;float w1=*w++;float *vz=&zlin[4*i-k*64];float *vy=&zlin[4*i-(15-k)*64];
#define DRMP3_S0(k) {int j;DRMP3_LOAD(k) for(j=0;j<4;j++)b[j]=vz[j]*w1+vy[j]*w0,a[j]=vz[j]*w0-vy[j]*w1;}
#define DRMP3_S1(k) {int j;DRMP3_LOAD(k) for(j=0;j<4;j++)b[j]+=vz[j]*w1+vy[j]*w0,a[j]+=vz[j]*w0-vy[j]*w1;}
#define DRMP3_S2(k) {int j;DRMP3_LOAD(k) for(j=0;j<4;j++)b[j]+=vz[j]*w1+vy[j]*w0,a[j]+=vy[j]*w1-vz[j]*w0;}
        float a[4],b[4];
        zlin[4*i]=xl[18*(31-i)];zlin[4*i+1]=xr[18*(31-i)];zlin[4*i+2]=xl[1+18*(31-i)];zlin[4*i+3]=xr[1+18*(31-i)];
        zlin[4*(i+16)]=xl[1+18*(1+i)];zlin[4*(i+16)+1]=xr[1+18*(1+i)];zlin[4*(i-16)+2]=xl[18*(1+i)];zlin[4*(i-16)+3]=xr[18*(1+i)];
        DRMP3_S0(0) DRMP3_S2(1) DRMP3_S1(2) DRMP3_S2(3) DRMP3_S1(4) DRMP3_S2(5) DRMP3_S1(6) DRMP3_S2(7)
        dstr[(15-i)*nch]=drmp3d_scale_pcm(a[1]);dstr[(17+i)*nch]=drmp3d_scale_pcm(b[1]);
        dstl[(15-i)*nch]=drmp3d_scale_pcm(a[0]);dstl[(17+i)*nch]=drmp3d_scale_pcm(b[0]);
        dstr[(47-i)*nch]=drmp3d_scale_pcm(a[3]);dstr[(49+i)*nch]=drmp3d_scale_pcm(b[3]);
        dstl[(47-i)*nch]=drmp3d_scale_pcm(a[2]);dstl[(49+i)*nch]=drmp3d_scale_pcm(b[2]);
    }
}
static void drmp3d_synth_granule(float *qmf_state, float *grbuf, int nbands, int nch, drmp3d_sample_t *pcm, float *lins){
    int i;
    for(i=0;i<nch;i++) drmp3d_DCT_II(grbuf+576*i,nbands);
    DRMP3_COPY_MEMORY(lins,qmf_state,sizeof(float)*15*64);
    for(i=0;i<nbands;i+=2) drmp3d_synth(grbuf+i,pcm+32*nch*i,nch,lins+i*64);
    DRMP3_COPY_MEMORY(qmf_state,lins+nbands*64,sizeof(float)*15*64);
}

static int drmp3d_match_frame(const u8 *hdr, int mp3_bytes, int frame_bytes){
    int i,nmatch;
    for(i=0,nmatch=0;nmatch<10;nmatch++){
        i+=drmp3_hdr_frame_bytes(hdr+i,frame_bytes)+drmp3_hdr_padding(hdr+i);
        if (i+DRMP3_HDR_SIZE>mp3_bytes) return nmatch>0;
        if (!drmp3_hdr_compare(hdr,hdr+i)) return 0;
    }
    return 1;
}
static int drmp3d_find_frame(const u8 *mp3, int mp3_bytes, int *free_format_bytes, int *ptr_frame_bytes){
    int i,k;
    for(i=0;i<mp3_bytes-DRMP3_HDR_SIZE;i++,mp3++){
        if (drmp3_hdr_valid(mp3)){
            int frame_bytes=drmp3_hdr_frame_bytes(mp3,*free_format_bytes);
            int frame_and_padding=frame_bytes+drmp3_hdr_padding(mp3);
            for(k=DRMP3_HDR_SIZE;!frame_bytes&&k<2304&&i+2*k<mp3_bytes-DRMP3_HDR_SIZE;k++){
                if(drmp3_hdr_compare(mp3,mp3+k)){int fb=k-drmp3_hdr_padding(mp3),nextfb=fb+drmp3_hdr_padding(mp3+k);if(i+k+nextfb+DRMP3_HDR_SIZE>mp3_bytes||!drmp3_hdr_compare(mp3,mp3+k+nextfb))continue;frame_and_padding=k;frame_bytes=fb;*free_format_bytes=fb;}
            }
            if((frame_bytes&&i+frame_and_padding<=mp3_bytes&&drmp3d_match_frame(mp3,mp3_bytes-i,frame_bytes))||(!i&&frame_and_padding==mp3_bytes)){*ptr_frame_bytes=frame_and_padding;return i;}
            *free_format_bytes=0;
        }
    }
    *ptr_frame_bytes=0; return mp3_bytes;
}

static void drmp3dec_init(drmp3dec *dec) { dec->header[0]=0; }
static int drmp3dec_decode_frame(drmp3dec *dec, const u8 *mp3, int mp3_bytes, void *pcm, drmp3dec_frame_info *info){
    int i=0,igr,frame_size=0,success=1;
    const u8 *hdr; drmp3_bs bs_frame[1];
    if (mp3_bytes>4&&dec->header[0]==0xff&&drmp3_hdr_compare(dec->header,mp3)){
        frame_size=drmp3_hdr_frame_bytes(mp3,dec->free_format_bytes)+drmp3_hdr_padding(mp3);
        if(frame_size!=mp3_bytes&&(frame_size+DRMP3_HDR_SIZE>mp3_bytes||!drmp3_hdr_compare(mp3,mp3+frame_size))) frame_size=0;
    }
    if (!frame_size){
        DRMP3_ZERO_MEMORY(dec,sizeof(drmp3dec));
        i=drmp3d_find_frame(mp3,mp3_bytes,&dec->free_format_bytes,&frame_size);
        if(!frame_size||i+frame_size>mp3_bytes){info->frame_bytes=i;return 0;}
    }
    hdr=mp3+i;
    DRMP3_COPY_MEMORY(dec->header,hdr,DRMP3_HDR_SIZE);
    info->frame_bytes=i+frame_size;
    info->channels=DRMP3_HDR_IS_MONO(hdr)?1:2;
    info->sample_rate=drmp3_hdr_sample_rate_hz(hdr);
    info->layer=4-DRMP3_HDR_GET_LAYER(hdr);
    info->bitrate_kbps=drmp3_hdr_bitrate_kbps(hdr);
    bs_frame[0].buf=hdr+DRMP3_HDR_SIZE; bs_frame[0].pos=0; bs_frame[0].limit=(frame_size-DRMP3_HDR_SIZE) * 8;
    if(DRMP3_HDR_IS_CRC(hdr)) drmp3_bs_get_bits(bs_frame,16);
    if(info->layer!=3) return 0;  /* Layer 1/2 not supported */
    {
        int main_data_begin=drmp3_L3_read_side_info(bs_frame,dec->scratch.gr_info,hdr);
        if(main_data_begin<0||bs_frame->pos>bs_frame->limit){drmp3dec_init(dec);return 0;}
        success=drmp3_L3_restore_reservoir(dec,bs_frame,&dec->scratch,main_data_begin);
        if(success&&pcm!=NULL){
            for(igr=0;igr<(DRMP3_HDR_TEST_MPEG1(hdr)?2:1);igr++,pcm=DRMP3_OFFSET_PTR(pcm,sizeof(drmp3d_sample_t)*576*info->channels)){
                DRMP3_ZERO_MEMORY(dec->scratch.grbuf[0],576*2*sizeof(float));
                drmp3_L3_decode(dec,&dec->scratch,dec->scratch.gr_info+igr*info->channels,info->channels);
                drmp3d_synth_granule(dec->qmf_state,dec->scratch.grbuf[0],18,info->channels,(drmp3d_sample_t*)pcm,dec->scratch.syn[0]);
            }
        }
        drmp3_L3_save_reservoir(dec,&dec->scratch);
    }
    return success*drmp3_hdr_frame_samples(dec->header);
}

#include <stdio.h>
static size_t drmp3__on_read_stdio(void *ud, void *buf, size_t n) { return fread(buf,1,n,(FILE*)ud); }
static bool drmp3__on_seek_stdio(void *ud, int offset, drmp3_seek_origin origin){
    int w=SEEK_SET; if(origin==DRMP3_SEEK_CUR)w=SEEK_CUR; else if(origin==DRMP3_SEEK_END)w=SEEK_END;
    return fseek((FILE*)ud,offset,w)==0;
}

static size_t drmp3__on_read(drmp3 *p, void *buf, size_t n){
    size_t r=p->onRead(p->pUserData,buf,n); p->streamCursor+=r; return r;
}
static size_t drmp3__on_read_clamped(drmp3 *p, void *buf, size_t n){
    if (p->streamLength==DRMP3_UINT64_MAX) return drmp3__on_read(p,buf,n);
    u64 rem=p->streamLength-p->streamCursor; if(n>rem)n=(size_t)rem;
    return drmp3__on_read(p,buf,n);
}
static bool drmp3__on_seek(drmp3 *p, int offset, drmp3_seek_origin origin){
    if(!p->onSeek(p->pUserData,offset,origin)) return 0;
    if(origin==DRMP3_SEEK_SET) p->streamCursor=(u64)offset; else p->streamCursor+=offset;
    return 1;
}
static bool drmp3__on_seek_64(drmp3 *p, u64 offset, drmp3_seek_origin origin){
    if(offset<=0x7FFFFFFF) return drmp3__on_seek(p,(int)offset,origin);
    if(!drmp3__on_seek(p,0x7FFFFFFF,DRMP3_SEEK_SET)) return 0;
    offset-=0x7FFFFFFF;
    while(offset>0){
        int chunk=(offset<=0x7FFFFFFF)?(int)offset:0x7FFFFFFF;
        if(!drmp3__on_seek(p,chunk,DRMP3_SEEK_CUR)) return 0;
        offset-=chunk;
    }
    return 1;
}

static u32 drmp3_decode_next_frame_ex(drmp3 *p, drmp3d_sample_t *pPCMFrames, drmp3dec_frame_info *pInfo) {
    u32 pcmFramesRead=0;
    if (p->atEnd) return 0;
    for(;;){
        drmp3dec_frame_info info;
        if (p->dataSize<16384){
            size_t bytesRead;
            if(p->pData) DRMP3_MOVE_MEMORY(p->pData,p->pData+p->dataConsumed,p->dataSize);
            p->dataConsumed=0;
            if(p->dataCapacity<(16384*4)){
                u8 *nd=(u8*)OS_Realloc(p->pData,p->dataCapacity,16384*4);
                p->pData=nd; p->dataCapacity=16384*4;
            }
            bytesRead=drmp3__on_read_clamped(p,p->pData+p->dataSize,p->dataCapacity-p->dataSize);
            if(!bytesRead&&p->dataSize==0){p->atEnd=1;return 0;}
            p->dataSize+=bytesRead;
        }
        if(p->dataSize>INT_MAX){p->atEnd=1;return 0;}
        if(!p->pData) return 0;
        pcmFramesRead=drmp3dec_decode_frame(&p->decoder,p->pData+p->dataConsumed,(int)p->dataSize,pPCMFrames,&info);
        p->dataConsumed+=(size_t)info.frame_bytes;
        p->dataSize    -=(size_t)info.frame_bytes;
        if(pcmFramesRead>0){
            pcmFramesRead=drmp3_hdr_frame_samples(p->decoder.header);
            p->pcmFramesConsumedInMP3Frame=0;
            p->pcmFramesRemainingInMP3Frame=pcmFramesRead;
            p->mp3FrameChannels=info.channels;
            p->mp3FrameSampleRate=info.sample_rate;
            if(pInfo) *pInfo=info;
            break;
        } else if(info.frame_bytes==0){
            size_t bytesRead;
            DRMP3_MOVE_MEMORY(p->pData,p->pData+p->dataConsumed,p->dataSize);
            p->dataConsumed=0;
            if(p->dataCapacity==p->dataSize){
                size_t needed = p->dataCapacity+16384*4;
                u8 *nd=(u8*)OS_Realloc(p->pData,p->dataCapacity,needed);
                p->pData=nd; p->dataCapacity=needed;
            }
            bytesRead=drmp3__on_read_clamped(p,p->pData+p->dataSize,p->dataCapacity-p->dataSize);
            if(!bytesRead){p->atEnd=1;return 0;}
            p->dataSize+=bytesRead;
        }
    }
    return pcmFramesRead;
}
static u32 drmp3_decode_next_frame(drmp3 *p)
{ return drmp3_decode_next_frame_ex(p,(drmp3d_sample_t*)p->pcmFrames,NULL); }

/* skip ID3v2 at start; ignore all metadata, just advance past it */
static void drmp3__skip_id3v2(drmp3 *p){
    char h[10];
    if(p->onRead(p->pUserData,h,10)!=10) return;
    if(h[0]=='I'&&h[1]=='D'&&h[2]=='3'){
        u32 sz=(((u32)h[6]&0x7F)<<21)|(((u32)h[7]&0x7F)<<14)|(((u32)h[8]&0x7F)<<7)|((u32)h[9]&0x7F);
        if(h[5]&0x10) sz+=10;
        p->onSeek(p->pUserData,(int)sz,DRMP3_SEEK_CUR);
        p->streamStartOffset+=10+sz;
        p->streamCursor=p->streamStartOffset;
    } else {
        p->onSeek(p->pUserData,0,DRMP3_SEEK_SET);
    }
}

static bool drmp3_init_internal(drmp3 *p, drmp3_read_proc onRead, drmp3_seek_proc onSeek){
    drmp3dec_frame_info firstFrameInfo;
    u32 firstFramePCMFrameCount;
    drmp3dec_init(&p->decoder);
    p->onRead=onRead; p->onSeek=onSeek;
    p->streamCursor=0; p->streamLength=DRMP3_UINT64_MAX;
    p->streamStartOffset=0;
    p->delayInPCMFrames=0; p->paddingInPCMFrames=0;
    p->totalPCMFrameCount=DRMP3_UINT64_MAX;

    /* detect stream length via seek-to-end (skip ID3v1/APE quietly) */
    if(onSeek){
        if(onSeek(p->pUserData,0,DRMP3_SEEK_END)){
            long slen=(long)ftell((FILE*)p->pUserData);
            if(slen>0){
                /* check for ID3v1 tag (128 bytes at end) */
                if(slen>128){
                    char tag[3];
                    fseek((FILE*)p->pUserData,-128,SEEK_END);
                    if(fread(tag,1,3,(FILE*)p->pUserData)==3&&tag[0]=='T'&&tag[1]=='A'&&tag[2]=='G') slen-=128;
                }
                p->streamLength=(u64)slen;
            }
            onSeek(p->pUserData,0,DRMP3_SEEK_SET);
            p->streamCursor=0;
        }
    }

    drmp3__skip_id3v2(p);
    firstFramePCMFrameCount=drmp3_decode_next_frame_ex(p,(drmp3d_sample_t*)p->pcmFrames,&firstFrameInfo);
    if(firstFramePCMFrameCount==0){ OS_DeallocateRAM(p->pData,p->dataCapacity); p->pData=NULL; p->dataCapacity=0; return 0; }

    /* check for Xing/Info VBR header to get total frame count */
    {
        drmp3_bs bs; drmp3_L3_gr_info grInfo[4];
        const u8 *pFirstFrameData=(const u8*)p->pData+(p->dataConsumed-(size_t)firstFrameInfo.frame_bytes);
        bs.buf=pFirstFrameData+DRMP3_HDR_SIZE; bs.pos=0; bs.limit=(firstFrameInfo.frame_bytes-DRMP3_HDR_SIZE) * 8;
        if(DRMP3_HDR_IS_CRC(pFirstFrameData)) drmp3_bs_get_bits(&bs,16);
        if(drmp3_L3_read_side_info(&bs,grInfo,pFirstFrameData)>=0){
            const u8 *td=pFirstFrameData+DRMP3_HDR_SIZE+(bs.pos/8);
            if((size_t)(firstFrameInfo.frame_bytes-(td-pFirstFrameData))>=8){
                int isXing=(td[0]=='X'&&td[1]=='i'&&td[2]=='n'&&td[3]=='g');
                int isInfo=(td[0]=='I'&&td[1]=='n'&&td[2]=='f'&&td[3]=='o');
                if(isXing||isInfo){
                    u32 flags=td[7]; td+=8;
                    if(flags&0x01&&(size_t)(firstFrameInfo.frame_bytes-(td-pFirstFrameData))>=4){
                        u32 mp3fc=(u32)td[0]<<24|(u32)td[1]<<16|(u32)td[2]<<8|(u32)td[3];
                        p->totalPCMFrameCount=(u64)mp3fc*firstFramePCMFrameCount;
                        td+=4;
                    }
                    if(flags&0x02&&(size_t)(firstFrameInfo.frame_bytes-(td-pFirstFrameData))>=4) td+=4;
                    if(flags&0x04&&(size_t)(firstFrameInfo.frame_bytes-(td-pFirstFrameData))>=100) td+=100;
                    if(flags&0x08&&(size_t)(firstFrameInfo.frame_bytes-(td-pFirstFrameData))>=4) td+=4;
                    /* LAME delay/padding */
                    if(td[0]&&(size_t)(firstFrameInfo.frame_bytes-(td-pFirstFrameData))>=36){
                        int d,pad; td+=21;
                        d  =(((u32)td[0]<<4)|((u32)td[1]>>4))+(528+1);
                        pad=((((u32)td[1]&0xF)<<8)|((u32)td[2]))  -(528+1);
                        if(pad<0) pad=0;
                        p->delayInPCMFrames=(u32)d; p->paddingInPCMFrames=(u32)pad;
                    }
                    /* reset and skip the Xing frame */
                    p->pcmFramesRemainingInMP3Frame=0;
                    p->streamStartOffset+=(u32)firstFrameInfo.frame_bytes;
                    p->streamCursor=p->streamStartOffset;
                    drmp3dec_init(&p->decoder);
                }
            }
        }
    }

    p->channels  =p->mp3FrameChannels;
    p->sampleRate=p->mp3FrameSampleRate;
    return 1;
}

static bool drmp3_init_file(drmp3 *pMP3, const char *pFilePath, void *unused){
    FILE *f; bool result;
    (void)unused;
    if(!pMP3||!pFilePath) return 0;
    DRMP3_ZERO_OBJECT(pMP3);
#if defined(_MSC_VER) && _MSC_VER>=1400
    if(fopen_s(&f,pFilePath,"rb")!=0) return 0;
#else
    f=fopen(pFilePath,"rb"); if(!f) return 0;
#endif
    pMP3->pUserData=(void*)f;
    result=drmp3_init_internal(pMP3,drmp3__on_read_stdio,drmp3__on_seek_stdio);
    if(!result){ fclose(f); return 0; }
    return 1;
}

static void drmp3_uninit(drmp3 *pMP3){
    if(!pMP3) return;
    if(pMP3->pUserData){ fclose((FILE*)pMP3->pUserData); pMP3->pUserData=NULL; }
    OS_DeallocateRAM(pMP3->pData,pMP3->dataCapacity); pMP3->pData=NULL; pMP3->dataCapacity=0;
}

static void drmp3_reset(drmp3 *p){
    p->pcmFramesConsumedInMP3Frame=0; p->pcmFramesRemainingInMP3Frame=0;
    p->currentPCMFrame=0; p->dataSize=0; p->atEnd=0;
    drmp3dec_init(&p->decoder);
}
static bool drmp3_seek_to_start_of_stream(drmp3 *p){
    if(!drmp3__on_seek_64(p,p->streamStartOffset,DRMP3_SEEK_SET)) return 0;
    drmp3_reset(p); return 1;
}

static u64 drmp3_read_pcm_frames_raw(drmp3 *p, u64 framesToRead, void *pBufferOut){
    u64 totalFramesRead=0;
    while(framesToRead>0){
        u32 framesToConsume;
        /* skip encoder delay */
        if(p->currentPCMFrame<p->delayInPCMFrames){
            u32 skip=(u32)vmin(p->pcmFramesRemainingInMP3Frame,p->delayInPCMFrames-p->currentPCMFrame);
            p->currentPCMFrame+=skip; p->pcmFramesConsumedInMP3Frame+=skip; p->pcmFramesRemainingInMP3Frame-=skip;
        }
        framesToConsume=(u32)vmin(p->pcmFramesRemainingInMP3Frame,framesToRead);
        /* clamp to padding boundary */
        if(p->totalPCMFrameCount!=DRMP3_UINT64_MAX&&p->totalPCMFrameCount>p->paddingInPCMFrames){
            if(p->currentPCMFrame<(p->totalPCMFrameCount-p->paddingInPCMFrames)){
                u64 rem=(p->totalPCMFrameCount-p->paddingInPCMFrames)-p->currentPCMFrame;
                if(framesToConsume>rem) framesToConsume=(u32)rem;
            } else break;
        }
        if(pBufferOut){
            float *out=(float*)DRMP3_OFFSET_PTR(pBufferOut,sizeof(float)*totalFramesRead*p->channels);
            float *in =(float*)DRMP3_OFFSET_PTR(&p->pcmFrames[0],sizeof(float)*p->pcmFramesConsumedInMP3Frame*p->mp3FrameChannels);
            DRMP3_COPY_MEMORY(out,in,sizeof(float)*framesToConsume*p->channels);
        }
        p->currentPCMFrame+=framesToConsume; p->pcmFramesConsumedInMP3Frame+=framesToConsume;
        p->pcmFramesRemainingInMP3Frame-=framesToConsume;
        totalFramesRead+=framesToConsume; framesToRead-=framesToConsume;
        if(framesToRead==0) break;
        if(p->totalPCMFrameCount!=DRMP3_UINT64_MAX&&p->totalPCMFrameCount>p->paddingInPCMFrames&&p->currentPCMFrame>=(p->totalPCMFrameCount-p->paddingInPCMFrames)) break;
        if(drmp3_decode_next_frame(p)==0) break;
    }
    return totalFramesRead;
}

static u64 drmp3_read_pcm_frames_f32(drmp3 *pMP3, u64 framesToRead, float *pBufferOut){
    if(!pMP3||!pMP3->onRead) return 0;
    return drmp3_read_pcm_frames_raw(pMP3,framesToRead,pBufferOut);
}

static bool drmp3_seek_to_pcm_frame(drmp3 *pMP3, u64 frameIndex){
    if(!pMP3||!pMP3->onSeek) return 0;
    if(frameIndex==0) return drmp3_seek_to_start_of_stream(pMP3);
    if(frameIndex<pMP3->currentPCMFrame){
        if(!drmp3_seek_to_start_of_stream(pMP3)) return 0;
    }
    
    u64 toSkip=frameIndex-pMP3->currentPCMFrame;
    u64 skipped=drmp3_read_pcm_frames_f32(pMP3,toSkip,NULL);
    return skipped==toSkip;
}

static u64 drmp3_get_pcm_frame_count(drmp3 *pMP3){    
    u64 total;
    if(pMP3->totalPCMFrameCount!=DRMP3_UINT64_MAX){
        total=pMP3->totalPCMFrameCount;
        if(total>=pMP3->delayInPCMFrames)   total-=pMP3->delayInPCMFrames;
        if(total>=pMP3->paddingInPCMFrames) total-=pMP3->paddingInPCMFrames;
        return total;
    }

    u64 savedFrame=pMP3->currentPCMFrame;
    if(!drmp3_seek_to_start_of_stream(pMP3)) return 0;
    total=0;
    for(;;){u32 n=drmp3_decode_next_frame_ex(pMP3,NULL,NULL);if(!n)break;total+=n;}
    drmp3_seek_to_start_of_stream(pMP3);
    drmp3_seek_to_pcm_frame(pMP3,savedFrame);
    return total;
}

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
                CopyMemoryFromBtoAForNBytes(&s,tmp + i*2,2);
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
static wav_channel_t wav_ch[MAX_CHANNELS],*ext_ch[MAX_CHANNELS];
static u32 wav_count,ext_count,mp3_slot,log_frame_count,log_frame_pos;
static mp3_channel_t mp3_ch[2];
static float *log_samples;
static size_t log_allocSize=0;
static bool log_playing,mp3_paused = false;
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

static void wave_mix(wav_channel_t* w, float* mix) {
    float vol = w->volume*sfx_scale();
    if (w->positional) vol *= spatial_atten(w->pos);
    for (i32 f = 0; f < AUDIO_FRAMES; f++) {
        if (w->frame_pos >= w->frame_count) { if (w->looping) w->frame_pos=0; else { w->playing=false; break; } }
        mix[f*2+0] += w->samples[w->frame_pos*2+0]*vol;
        mix[f*2+1] += w->samples[w->frame_pos*2+1]*vol;
        w->frame_pos++;
    }
}

static void audio_mix_period(i16 *out) {
    float mix[AUDIO_FRAMES*AUDIO_CHANNELS];
    MemSetToValueForNBytes(mix,0,sizeof(mix));
    for (u32 c=0;c<wav_count;c++) { wav_channel_t* w = &wav_ch[c]; if (w->playing && w->samples) {wave_mix(w,mix);} }
    for (u32 c=0;c<ext_count;c++) { wav_channel_t* w = ext_ch[c]; if (w->playing && w->samples) {wave_mix(w,mix);} }
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
        for (u32 s = 0; s < 2; s++) {
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

    for (u32 i = 0; i < AUDIO_FRAMES*AUDIO_CHANNELS; i++) out[i] = f32_to_s16(mix[i]);
}

ENGINE_TO_MOD void play_wav(const char *path,float volume,Vector3 pos,bool positional) {
    if (StringsEqual(path,"./Audio/misc/null.wav")) return;

    i32 slot = -1;
    for (u32 i = 0; i < wav_count; i++) if (!wav_ch[i].playing && wav_ch[i].samples) { OS_DeallocateRAM(wav_ch[i].samples,wav_ch[i].allocSize); wav_ch[i].samples=NULL; wav_ch[i].allocSize=0; slot=i; break; }
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
    for (u32 i=0;i<ext_count;i++) if (ext_ch[i]==w) return 0;
    if (ext_count<MAX_CHANNELS) ext_ch[ext_count++]=w;
    return 0;
}
ENGINE_TO_MOD i32 SoundStop(ma_sound *pSound) { ((wav_channel_t*)pSound)->playing=false; return 0; }
ENGINE_TO_MOD void SoundUninit(ma_sound *pSound) {
    wav_channel_t *w=(wav_channel_t*)pSound;
    if (w->samples){OS_DeallocateRAM(w->samples,w->allocSize);w->samples=NULL;w->allocSize=0;} w->playing=false;
    for (u32 i=0;i<ext_count;i++) if (ext_ch[i]==w) { ext_ch[i]=ext_ch[--ext_count]; break; }
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

#define MAX_PCM_DEVICES 8
static OsFileHandle pcm_fds[MAX_PCM_DEVICES];
static i32 pcm_fd_count = 0;
#ifndef WINDOWS
typedef void snd_pcm_t;
typedef int  (*pfn_snd_pcm_open)(snd_pcm_t**, const char*, int, int);
typedef int  (*pfn_snd_pcm_close)(snd_pcm_t*);
typedef int  (*pfn_snd_pcm_writei)(snd_pcm_t*, const void*, u32);
typedef int  (*pfn_snd_pcm_recover)(snd_pcm_t*, int, int);
typedef int  (*pfn_snd_pcm_prepare)(snd_pcm_t*);
typedef int  (*pfn_snd_pcm_hw_params_sizeof)(void);
typedef int  (*pfn_snd_pcm_hw_params_any)(snd_pcm_t*, void*);
typedef int  (*pfn_snd_pcm_hw_params_set_access)(snd_pcm_t*, void*, unsigned int);
typedef int  (*pfn_snd_pcm_hw_params_set_format)(snd_pcm_t*, void*, int);
typedef int  (*pfn_snd_pcm_hw_params_set_channels)(snd_pcm_t*, void*, unsigned int);
typedef int  (*pfn_snd_pcm_hw_params_set_rate_near)(snd_pcm_t*, void*, unsigned int*, int*);
typedef int  (*pfn_snd_pcm_hw_params_set_period_size_near)(snd_pcm_t*, void*, unsigned long*, int*);
typedef int  (*pfn_snd_pcm_hw_params_set_periods_near)(snd_pcm_t*, void*, unsigned int*, int*);
typedef int  (*pfn_snd_pcm_hw_params)(snd_pcm_t*, void*);
typedef const char* (*pfn_snd_strerror)(int);
static snd_pcm_t         *g_alsa_pcm;
static pfn_snd_pcm_writei g_snd_writei;
static pfn_snd_pcm_recover g_snd_recover;
static pfn_snd_pcm_prepare g_snd_prepare;
static pfn_snd_strerror    g_snd_strerror;
static bool alsa_try_open_default(void) {
    void *so = dlopen("libasound.so.2", RTLD_NOW|RTLD_LOCAL);
    if (!so) so = dlopen("libasound.so", RTLD_NOW|RTLD_LOCAL);
    if (!so) { DualLog("Audio: libasound not found\n"); return false; }
    DualLog("Audio: libasound loaded\n");

    pfn_snd_pcm_open                   snd_open      = dlsym(so, "snd_pcm_open");
    pfn_snd_pcm_hw_params_sizeof       hw_sizeof     = dlsym(so, "snd_pcm_hw_params_sizeof");
    pfn_snd_pcm_hw_params_any          hw_any        = dlsym(so, "snd_pcm_hw_params_any");
    pfn_snd_pcm_hw_params_set_access   hw_set_access = dlsym(so, "snd_pcm_hw_params_set_access");
    pfn_snd_pcm_hw_params_set_format   hw_set_fmt    = dlsym(so, "snd_pcm_hw_params_set_format");
    pfn_snd_pcm_hw_params_set_channels hw_set_ch     = dlsym(so, "snd_pcm_hw_params_set_channels");
    pfn_snd_pcm_hw_params_set_rate_near        hw_set_rate   = dlsym(so, "snd_pcm_hw_params_set_rate_near");
    pfn_snd_pcm_hw_params_set_period_size_near hw_set_period = dlsym(so, "snd_pcm_hw_params_set_period_size_near");
    pfn_snd_pcm_hw_params_set_periods_near     hw_set_periods= dlsym(so, "snd_pcm_hw_params_set_periods_near");
    pfn_snd_pcm_hw_params              hw_apply      = dlsym(so, "snd_pcm_hw_params");
    g_snd_writei  = dlsym(so, "snd_pcm_writei");
    g_snd_recover = dlsym(so, "snd_pcm_recover");
    g_snd_prepare = dlsym(so, "snd_pcm_prepare");
    g_snd_strerror= dlsym(so, "snd_strerror");

    if (!snd_open||!hw_sizeof||!hw_any||!hw_set_access||!hw_set_fmt||!hw_set_ch||
        !hw_set_rate||!hw_set_period||!hw_set_periods||!hw_apply||
        !g_snd_writei||!g_snd_recover||!g_snd_prepare) {
        DualLogError("Audio: libasound missing required symbols\n"); return false;
    }

    int r = snd_open(&g_alsa_pcm, "default", 0/*PLAYBACK*/, 0/*blocking*/);
    if (r < 0 || !g_alsa_pcm) {
        DualLogError("Audio: snd_pcm_open('default') failed: %d (%s)\n", r, g_snd_strerror ? g_snd_strerror(r) : "?");
        return false;
    }
    DualLog("Audio: snd_pcm_open('default') OK\n");

    int sz = hw_sizeof();
    DualLog("Audio: snd_pcm_hw_params_t size = %d\n", sz);
    u8 hw_params_buf[640];
    if (sz > (int)sizeof(hw_params_buf)) { DualLogError("Audio: hw_params_t too large (%d)\n", sz); return false; }
    void *hwp = hw_params_buf;

    if ((r = hw_any(g_alsa_pcm, hwp)) < 0) { DualLogError("Audio: hw_params_any failed: %s\n", g_snd_strerror(r)); return false; }
    if ((r = hw_set_access(g_alsa_pcm, hwp, 3/*RW_INTERLEAVED*/)) < 0) { DualLogError("Audio: set_access failed: %s\n", g_snd_strerror(r)); return false; }
    if ((r = hw_set_fmt(g_alsa_pcm, hwp, 2/*S16_LE*/)) < 0) { DualLogError("Audio: set_format S16_LE failed: %s\n", g_snd_strerror(r)); return false; }
    if ((r = hw_set_ch(g_alsa_pcm, hwp, AUDIO_CHANNELS)) < 0) { DualLogError("Audio: set_channels(%d) failed: %s\n", AUDIO_CHANNELS, g_snd_strerror(r)); return false; }

    unsigned int rate = AUDIO_RATE; int dir = 0;
    if ((r = hw_set_rate(g_alsa_pcm, hwp, &rate, &dir)) < 0) { DualLogError("Audio: set_rate(%u) failed: %s\n", AUDIO_RATE, g_snd_strerror(r)); return false; }
    DualLog("Audio: rate set to %u (requested %d)\n", rate, AUDIO_RATE);

    unsigned long period = AUDIO_FRAMES; dir = 0;
    if ((r = hw_set_period(g_alsa_pcm, hwp, &period, &dir)) < 0) { DualLogError("Audio: set_period_size(%d) failed: %s\n", AUDIO_FRAMES, g_snd_strerror(r)); return false; }
    DualLog("Audio: period size set to %lu (requested %d)\n", period, AUDIO_FRAMES);

    unsigned int periods = AUDIO_PERIODS; dir = 0;
    if ((r = hw_set_periods(g_alsa_pcm, hwp, &periods, &dir)) < 0) { DualLogError("Audio: set_periods(%d) failed: %s\n", AUDIO_PERIODS, g_snd_strerror(r)); return false; }
    DualLog("Audio: periods set to %u (requested %d)\n", periods, AUDIO_PERIODS);

    if ((r = hw_apply(g_alsa_pcm, hwp)) < 0) { DualLogError("Audio: hw_params apply failed: %s\n", g_snd_strerror(r)); return false; }
    DualLog("Audio: hw_params applied OK\n");

    if ((r = g_snd_prepare(g_alsa_pcm)) < 0) { DualLogError("Audio: snd_pcm_prepare failed: %s\n", g_snd_strerror(r)); return false; }
    DualLog("Audio: snd_pcm_prepare OK — libasound path active\n");
    return true;
}

static void init_pcm_device(i32 card, i32 dev) {
    OsFileHandle r = pcm_open(card, dev, 1|(1<<1));
    if (r == OS_INVALID_HANDLE) return;
    DualLog("Audio: trying raw device card=%d dev=%d fd=%d\n", card, dev, r);
    pcm_params_t p; pcm_params_init(&p);
    pcm_set(&p, PCM_FORMAT, SNDRV_PCM_FORMAT_S16_LE);
    pcm_set(&p, SNDRV_PCM_HW_PARAM_ACCESS, SNDRV_PCM_ACCESS_RW_INTERLEAVED);
    pcm_set(&p, PCM_RATE, AUDIO_RATE); pcm_set(&p, PCM_CHANNELS, AUDIO_CHANNELS);
    pcm_set(&p, PCM_PERIOD_SIZE, AUDIO_FRAMES); pcm_set(&p, SNDRV_PCM_HW_PARAM_PERIODS, AUDIO_PERIODS);
    if (pcm_params_setup(r, &p) >= 0 && pcm_fd_count < MAX_PCM_DEVICES) {
        DualLog("Audio: raw device card=%d dev=%d OK\n", card, dev);
        pcm_fds[pcm_fd_count++] = r;
    } else {
        DualLog("Audio: raw device card=%d dev=%d setup failed, closing\n", card, dev);
        OS_Close(r);
    }
}
#endif

void AudioUpdate(void) {
#ifndef WINDOWS
    if (g_alsa_pcm) {
        i16 buf[AUDIO_FRAMES*AUDIO_CHANNELS];
        audio_mix_period(buf);
        int r = g_snd_writei(g_alsa_pcm, buf, (u32)AUDIO_FRAMES);
        if (r < 0) {
            DualLog("Audio: snd_pcm_writei underrun %d, recovering\n", r);
            r = g_snd_recover(g_alsa_pcm, r, 0);
            if (r < 0) DualLogError("Audio: snd_pcm_recover failed %d\n", r);
            else g_snd_writei(g_alsa_pcm, buf, (u32)AUDIO_FRAMES);
        }
        return;
    }
#endif
    if (pcm_fd_count==0) return;
    i16 buf[AUDIO_FRAMES*AUDIO_CHANNELS]; pcm_sync_t sync;
    if (pcm_sync(pcm_fds[0],&sync,0)<0) return;
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

pthread_t audThreadID; int usleep(u32 usec);
void* AudThread(void* arg) { (void)arg; while (1) { AudioUpdate(); usleep(1000); } return NULL; }
void InitAudio(void) {
#ifdef WINDOWS
    OsFileHandle first = pcm_open_all(AUDIO_RATE, AUDIO_CHANNELS, AUDIO_FRAMES, AUDIO_PERIODS);
    if (first == OS_INVALID_HANDLE) { DualLog("ERROR: No WASAPI audio device found\n"); return; }
    pcm_fds[0] = first; pcm_fd_count = 1;
    DualLog("Audio: WASAPI %d device(s) active\n", wasapi_dev_count);
#else
    if (alsa_try_open_default()) {
        DualLog("Audio: using libasound 'default' path\n");
    } else {
        DualLog("Audio: libasound path failed, trying raw devices\n");
        for (i32 card = 0; card < 8; card++)
            for (i32 dev = 0; dev < 8; dev++)
                init_pcm_device(card, dev);
        if (pcm_fd_count == 0) DualLogError("Audio: no output device found\n");
        else DualLog("Audio: %d raw device(s) active\n", pcm_fd_count);
    }
#endif
    pthread_create(&audThreadID, NULL, AudThread, NULL);
}
