// nanowasapi.c
#include "os.h"
#include "voxen.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

typedef u32 snd_pcm_uframes_t; // match nanoalsa

#define PCM_OUTPUT   1
#define PCM_NONBLOCK (1<<1)
#define PCM_FORMAT_S16_LE 2
#define PCM_ACCESS_RW     3
#define SNDRV_PCM_SYNC_PTR_HWSYNC 0

typedef struct { int format,access,rate,channels,period_frames,periods; } pcm_params_t;
typedef struct { snd_pcm_uframes_t hw_ptr; }  pcm_status_t;
typedef struct { snd_pcm_uframes_t appl_ptr; } pcm_control_t;
typedef struct { pcm_status_t status; pcm_control_t control; } pcm_sync_t;

typedef enum { PCM_FORMAT=0,PCM_ACCESS,PCM_RATE,PCM_CHANNELS,PCM_PERIOD_SIZE,PCM_PERIODS,PCM_INTERRUPT } pcm_param_t;

#define MAX_WASAPI_DEVICES 8
typedef struct {
    IAudioClient       *client;
    IAudioRenderClient *render;
    UINT32              buffer_frames;
    int                 rate,channels,period_frames;
    bool                open;
} wasapi_dev_t;

static wasapi_dev_t wasapi_devs[MAX_WASAPI_DEVICES];
static int          wasapi_dev_count = 0;

#define FD_TO_IDX(fd) ((int)(intptr_t)(fd)-100)
#define IDX_TO_FD(i)  ((OsFileHandle)(intptr_t)((i)+100))

static const CLSID CLSID_MMDeviceEnumerator_ = {0xBCDE0395,0xE52F,0x467C,{0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E}};
static const IID   IID_IMMDeviceEnumerator_  = {0xA95664D2,0x9614,0x4F35,{0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6}};
static const IID   IID_IAudioClient_         = {0x1CB9AD4C,0xDBFA,0x4C32,{0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2}};
static const IID   IID_IAudioRenderClient_   = {0xF294ACFC,0x3146,0x4483,{0xA7,0xBF,0xAD,0xDC,0xA7,0xC2,0x60,0xE2}};

static int wasapi_init_device(IMMDevice *dev,int rate,int channels,int period_frames,int periods) {
    if (wasapi_dev_count>=MAX_WASAPI_DEVICES) return -1;
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

void pcm_params_init(pcm_params_t *p) { SetMemoryToValueForNBytes(p,0,sizeof(*p)); }
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
