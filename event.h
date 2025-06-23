#ifndef EVENT_H
#define EVENT_H

#include <stdio.h>
#include <time.h>

// Event types for debugging and playback callbacks
#define EV_NULL 0u
#define EV_INIT 1u

#define EV_KEYDOWN 10u
#define EV_KEYUP 11u
#define EV_MOUSEMOVE 12u
#define EV_MOUSEDOWN 13u
#define EV_MOUSEUP 14u
#define EV_MOUSEWARP 15u

#define EV_LOAD_TEXTURES 20u
#define EV_LOAD_AUDIO 21u
#define EV_LOAD_MODELS 22u

#define EV_NETWORK_PACKET_RX 30u
#define EV_NETWORK_PACKET_TX 31u
#define EV_NETWORK_CONNECT 32u
#define EV_NETWORK_DISCONNECT 33u

#define EV_PLAYAUDIO_CLIP 40u
#define EV_PLAYAUDIO_STREAM 41u

#define EV_PAUSE 254u
#define EV_QUIT 255u

// Event Journal Buffer
#define EVENT_JOURNAL_BUFFER_SIZE 30000

// Event Queue
#define MAX_EVENTS_PER_FRAME 100

// Event System variables
typedef struct {
    double timestamp;
    double deltaTime_ns;
    uint32_t frameNum;
    uint32_t payload1u; // First one used for payloads less than or equal to 4 bytes
    uint32_t payload2u; // Second one used for more values or for long ints by using bitpacking
    float payload1f; // First one used for float payloads
    float payload2f; // Second one used for a 2nd value or for double via bitpacking
    uint8_t type;
} Event;

extern Event eventQueue[MAX_EVENTS_PER_FRAME];
extern int eventJournalIndex;

// Journal buffer for event history to write into the log/demo file
extern Event eventJournal[EVENT_JOURNAL_BUFFER_SIZE];

extern int eventIndex; // Event that made it to the counter.  Indices below this were
                // already executed and walked away from the counter.

extern int eventQueueEnd; // End of the waiting line

extern FILE* activeLogFile;
extern bool log_playback;
extern double lastJournalWriteTime;
extern const char* manualLogName;

extern int maxEventCount_debug;
extern uint32_t globalFrameNum;
extern double last_time;
extern double current_time;
extern double start_frame_time;

int EventExecute(Event* event);
int EventInit(void);
int EnqueueEvent(uint8_t type, uint32_t payload1u, uint32_t payload2u, float payload1f, float payload2f);
int EnqueueEvent_UintUint(uint8_t type, uint32_t payload1u, uint32_t payload2u);
int EnqueueEvent_Uint(uint8_t type, uint32_t payload1u);
int EnqueueEvent_FloatFloat(uint8_t type, float payload1f, float payload2f);
int EnqueueEvent_Float(uint8_t type, float payload1f);
int EnqueueEvent_Simple(uint8_t type);
void clear_ev_journal(void);
void JournalLog(void);
int ReadActiveLog();
int JournalDump(const char* dem_file);
void clear_ev_queue(void);
double get_time(void);
int EventQueueProcess(void);

#endif // EVENT_H
