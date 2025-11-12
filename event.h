// event.h - Unified Event system declarations
#ifndef VOXEN_EVENT_H
#define VOXEN_EVENT_H

#include <stdint.h>
#include <stdbool.h>

// Event System
#define EV_NULL 0u
#define EV_INIT 1u
#define EV_KEYDOWN 10u
#define EV_KEYUP 11u
#define EV_MOUSEMOVE 12u
#define EV_MOUSEDOWN 13u
#define EV_MOUSEUP 14u
#define EV_MOUSEWARP 15u
#define EV_PLAYAUDIO_CLIP 40u
#define EV_PLAYAUDIO_STREAM 41u
#define EV_PHYSICS_TICK 50u
#define EV_PARTICLE_TICK 60u
#define EV_PAUSE 254u
#define EV_QUIT 255u

// Event Journal Buffer
#define EVENT_JOURNAL_BUFFER_SIZE 1000

// Event Queue
#define MAX_EVENTS_PER_FRAME 100

// Event System variables
typedef struct {
    double timestamp;
    double deltaTime_ns;
    uint32_t frameNum;
    int32_t payload1i; // First one used for payloads less than or equal to 4 bytes
    int32_t payload2i; // Second one used for more values or for long ints by using bitpacking
    float payload1f; // First one used for float payloads
    float payload2f; // Second one used for a 2nd value or for double via bitpacking
    uint8_t type;
} Event;
extern Event eventQueue[MAX_EVENTS_PER_FRAME];
extern int32_t eventJournalIndex;
extern bool journalFirstWrite;
// Journal buffer for event history to write into the log/demo file
extern Event eventJournal[EVENT_JOURNAL_BUFFER_SIZE];
extern int32_t eventIndex; // Event that made it to the counter.  Indices below this were
                // already executed and walked away from the counter.
extern int32_t eventQueueEnd; // End of the waiting line
extern bool log_playback;
extern double lastJournalWriteTime;
extern double cpuTime;
extern const char* manualLogName;
extern int32_t maxEventCount_debug;
extern uint32_t globalFrameNum;
extern double last_time;
extern double current_time;
extern float pauseRelativeTime;
void ActiveLogFileInit();
void OpenLogForPlayback(const char* path);
int32_t ReadActiveLog();
void clear_ev_queue(void);
int32_t EventExecute(Event* event);
int32_t EventInit(void);
int32_t EnqueueEvent(uint8_t type, int32_t payload1i, int32_t payload2i, float payload1f, float payload2f);
int32_t EnqueueEvent_IntInt(uint8_t type, int32_t payload1i, int32_t payload2i);
int32_t EnqueueEvent_Int(uint8_t type, int32_t payload1i);
int32_t EnqueueEvent_FloatFloat(uint8_t type, float payload1f, float payload2f);
int32_t EnqueueEvent_Float(uint8_t type, float payload1f);
int32_t EnqueueEvent_Simple(uint8_t type);
void clear_ev_journal(void);
void JournalLog(void);
double get_time(void);
int32_t EventQueueProcess(void);
#endif // VOXEN_EVENT_H
