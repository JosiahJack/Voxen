// Event types for debugging and playback callbacks
#define EV_NULL 0u
#define EV_INIT 1u
#define EV_CLEAR_FRAME_BUFFERS 2u
#define EV_RENDER_UI 3u
#define EV_RENDER_STATICS 4u
#define EV_RENDER_DYNAMICS 5u
#define EV_RENDER_TRANSPARENTS 6u

#define EV_KEYDOWN 10u
#define EV_KEYUP 11u
#define EV_MOUSEMOVE 12u
#define EV_MOUSEDOWN 13u
#define EV_MOUSEUP 14u
#define EV_MOUSEWARP 15u

#define EV_LOAD_TEXTURE 20u
#define EV_LOAD_AUDIO 21u
#define EV_LOAD_MODEL 22u

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
    uint32_t payload1u; // First one used for payloads less than or equal to 4 bytes
    uint32_t payload2u; // Second one used for more values or for long ints by using bitpacking
    float payload1f; // First one used for float payloads
    float payload2f; // Second one used for a 2nd value or for double via bitpacking
    uint8_t type;
} Event;

int EventExecute(Event* event);
