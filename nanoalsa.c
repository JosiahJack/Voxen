#include "os.h"
#include "voxen.h"
#include <sys/ioctl.h>
#include <sound/asound.h>
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
static inline int pcm_start(int fd)   { return OS_IOControlSimple(fd,SNDRV_PCM_IOCTL_START); }
static inline int pcm_stop(int fd)    { return OS_IOControlSimple(fd,SNDRV_PCM_IOCTL_DROP); }
static inline int pcm_drain(int fd)   { return OS_IOControlSimple(fd,SNDRV_PCM_IOCTL_DRAIN); }
static inline int pcm_xrun(int fd)    { return OS_IOControlSimple(fd,SNDRV_PCM_IOCTL_XRUN); }
static inline int pcm_reset(int fd)   { return OS_IOControlSimple(fd,SNDRV_PCM_IOCTL_RESET); }
static inline int pcm_resume(int fd)  { return OS_IOControlSimple(fd,SNDRV_PCM_IOCTL_RESUME); }
static inline int pcm_pause(int fd)   { return OS_IOControl(fd,SNDRV_PCM_IOCTL_PAUSE,(void*)1); }
static inline int pcm_unpause(int fd) { return OS_IOControl(fd,SNDRV_PCM_IOCTL_PAUSE,(void*)0); }
static inline int pcm_mmap_sync_pos(int fd) { return OS_IOControlSimple(fd, SNDRV_PCM_IOCTL_HWSYNC); }
static inline int pcm_move_app_pos(int fd, int frames) { return frames < 0 ? OS_IOControl(fd,SNDRV_PCM_IOCTL_REWIND,(void*)-frames) : OS_IOControl(fd,SNDRV_PCM_IOCTL_FORWARD,(void*)frames); }
static inline int pcm_link(int fd, int fd2) { return OS_IOControl(fd,SNDRV_PCM_IOCTL_LINK,(void*)fd2); }
static inline int pcm_unlink(int fd) { return OS_IOControlSimple(fd,SNDRV_PCM_IOCTL_UNLINK); }
static inline int pcm_write(int fd, void *buf, int frames) { struct snd_xferi tmp={.buf=buf,.frames=frames,.result=0}; return OS_IOControl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES,(void*)&tmp) ? -1 : (int) tmp.result; }
static inline int pcm_read(int fd, void *buf, int frames) { struct snd_xferi tmp ={.buf=buf,.frames=frames,.result=0}; return OS_IOControl(fd, SNDRV_PCM_IOCTL_READI_FRAMES,(void*)&tmp) ? -1 : (int)tmp.result; }
static inline int pcm_write_scattered(int fd, void **bufs, int frames) { struct snd_xfern tmp={.bufs=bufs,.frames=frames,.result = 0}; return OS_IOControl(fd,SNDRV_PCM_IOCTL_WRITEN_FRAMES,(void*)&tmp) ? -1 : (int)tmp.result; }
static inline int pcm_read_scattered(int fd, void **bufs, int frames) { struct snd_xfern tmp={.bufs=bufs,.frames=frames,.result=0}; return OS_IOControl(fd,SNDRV_PCM_IOCTL_READN_FRAMES,(void*)&tmp) ? -1 : (int)tmp.result; }
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
static inline unsigned int pcm_get_min(pcm_params_t *params, pcm_param_t parameter) { unsigned int min,max; pcm_get_range(params,parameter,&min,&max); return min; }
static inline unsigned int pcm_get_max(pcm_params_t *params, pcm_param_t parameter) { unsigned int min,max; pcm_get_range(params,parameter,&min,&max); return max; }
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
