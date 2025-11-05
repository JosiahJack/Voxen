#include <stdio.h>
#include "event.h"
#include "voxen.h"

void exit(int status);

int32_t maxEventCount_debug = 0;
double lastJournalWriteTime = 0;
FILE* activeLogFile;
FILE* console_log_file = NULL;
const char* manualLogName;
bool log_playback = false;
Event eventQueue[MAX_EVENTS_PER_FRAME]; // Queue for events to process this frame
Event eventJournal[EVENT_JOURNAL_BUFFER_SIZE]; // Journal buffer for event history to write into the log/demo file
int32_t eventJournalIndex;
int32_t eventIndex; // Event that made it to the counter.  Indices below this were already executed and walked away from the counter.
int32_t eventQueueEnd; // End of the waiting line
bool journalFirstWrite = true;

// Logs both to log file and console, usage same as printf
void DualLogMain(FILE *stream, const char *prefix, const char *fmt, va_list args) {
    va_list copy; va_copy(copy, args);
    if (prefix) fprintf(stream, "%s\033[0m", prefix);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\033[0m"); fflush(stream);
    if (console_log_file) {
        if (prefix) fprintf(console_log_file, "%s ", prefix);
        vfprintf(console_log_file, fmt, copy);
        fflush(console_log_file);
    }
    va_end(copy);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
void DualLogEntity(uint16_t idx) {
    DualLog("Entity instance[%u]::\n"
            "    index: %u\n"
            "    entflags: %u [\n      ACTIVE:     %u\n      CARDCHUNK:  %u\n      GROUNDED:   %u\n      USEGRAVITY: %u\n    ]\n"
            "    position.x: %f, .y: %f, .z: %f\n"
            "    rotation.x: %f, .y: %f, .z: %f, .w: %f\n"
            "    scale.x: %f, .y: %f, .z: %f\n"
            "    velocity.x: %f, .y: %f, .z: %f\n"
            "    modelIndex: %u\n"
            "    texIndex:   %u\n"
            "    glowIndex:  %u\n"
            "    specIndex:  %u\n"
            "    normIndex:  %u\n"
            "    lodIndex:   %u\n",
            idx,
            instances[idx].index,
            instances[idx].entflags,
                (instances[idx].entflags & ENTFLAG_ACTIVE) > 0,
                (instances[idx].entflags & ENTFLAG_CARDCHUNK) > 0,
                (instances[idx].entflags & ENTFLAG_GROUNDED) > 0,
                (instances[idx].entflags & ENTFLAG_USEGRAVITY) > 0,
            instances[idx].position.x, instances[idx].position.y, instances[idx].position.z,
            instances[idx].rotation.x, instances[idx].rotation.y, instances[idx].rotation.z, instances[idx].rotation.w,
            instances[idx].scale.x, instances[idx].scale.y, instances[idx].scale.z,
            instances[idx].velocity.x, instances[idx].velocity.y, instances[idx].velocity.z,
            instances[idx].modelIndex,
            instances[idx].texIndex,
            instances[idx].glowIndex,
            instances[idx].specIndex,
            instances[idx].normIndex,
            instances[idx].lodIndex);
}
#pragma GCC diagnostic pop

void DualLog(const char* fmt, ...) { va_list args; va_start(args, fmt); DualLogMain(stdout, NULL, fmt, args); va_end(args); }
void DualLogWarn(const char* fmt, ...) { va_list args; va_start(args, fmt); DualLogMain(stdout, "\033[1;38;5;208mWARN:", fmt, args); va_end(args); }
void DualLogError(const char* fmt, ...) { va_list args; va_start(args, fmt); DualLogMain(stderr, "\033[1;31mERROR:", fmt, args); va_end(args); }

void OpenConsoleLogFile() {
    console_log_file = fopen("voxen.log", "w"); // Initialize log system for all prints to go to both stdout and voxen.log file
    if (!console_log_file) DualLogError("Failed to open log file voxen.log\n");
}

void ActiveLogFileInit() {
    activeLogFile = NULL;
}

void OpenLogForPlayback(const char* path) {
    activeLogFile = fopen(path, "rb");
    if (!activeLogFile) {
        DualLogError("Failed to read log: %s\n", path);
    } else {
        log_playback = true; // Perform log playback.
    }
}

// All core engine operations run through the EventExecute as an Event processed
// by the unified event system in the order it was enqueued.
int32_t EventExecute(Event* event) {
    if (event->type == EV_NULL) return 0;

    switch(event->type) {
        case EV_KEYDOWN: return Input_KeyDown(event->payload1i);
        case EV_KEYUP: return Input_KeyUp(event->payload1i);
        case EV_MOUSEMOVE: return Input_MouseMove(event->payload1i,event->payload2i);
        case EV_PHYSICS_TICK: return Physics();
        case EV_PARTICLE_TICK: return ParticleSystemStep();
        case EV_QUIT: return 1; break;
    }

    DualLogError("Unknown event %d\n",event->type);
    return 99;
}

int32_t EnqueueEvent(uint8_t type, int32_t payload1i, int32_t payload2i, float payload1f, float payload2f) {
    if (eventQueueEnd >= MAX_EVENTS_PER_FRAME) { DualLogError("Queue buffer filled!\n"); return 1; }

    //DualLog("Enqueued event type %d, at index %d\n",type,eventQueueEnd);
    eventQueue[eventQueueEnd].frameNum = globalFrameNum;
    eventQueue[eventQueueEnd].type = type;
    eventQueue[eventQueueEnd].timestamp = 0;
    eventQueue[eventQueueEnd].payload1i = payload1i;
    eventQueue[eventQueueEnd].payload2i = payload2i;
    eventQueue[eventQueueEnd].payload1f = payload1f;
    eventQueue[eventQueueEnd].payload2f = payload2f;
    eventQueueEnd++;
    return 0;
}

int32_t EnqueueEvent_IntInt(uint8_t type, int32_t payload1i, int32_t payload2i) {
    return EnqueueEvent(type,payload1i,payload2i,0.0f,0.0f);
}

int32_t EnqueueEvent_Int(uint8_t type, int32_t payload1i) {
    return EnqueueEvent(type,payload1i,0u,0.0f,0.0f);
}

int32_t EnqueueEvent_FloatFloat(uint8_t type, float payload1f, float payload2f) {
    return EnqueueEvent(type,0u,0u,payload1f,payload2f);
}

int32_t EnqueueEvent_Float(uint8_t type, float payload1f) {
    return EnqueueEvent(type,0u,0u,payload1f,0.0f);
}

// Enqueues an event with type only and no payload values.
int32_t EnqueueEvent_Simple(uint8_t type) {
    return EnqueueEvent(type,0u,0u,0.0f,0.0f);
}

// Intended to be called after each buffered write to the logfile in .dem
// format which is custom but similar concept to Quake 1 demos.
void clear_ev_journal(void) {
    //  Events will be buffer written until EV_NULL is seen so clear to EV_NULL.
    for (int32_t i=0;i<EVENT_JOURNAL_BUFFER_SIZE;i++) {
        eventJournal[i].type = EV_NULL;
        eventJournal[i].frameNum = 0;
        eventJournal[i].timestamp = 0.0;
        eventJournal[i].deltaTime_ns = 0.0;
    }

    eventJournalIndex = 0; // Restart at the beginning.
}

void JournalLog(void) {
//     double timestamp = get_time();
    FILE* fp;
    if (journalFirstWrite) {
        fp = fopen("./voxen.dem", "wb"); // Overwrite for first write.
        journalFirstWrite = false;
    } else fp = fopen("./voxen.dem", "ab"); // Append

    if (!fp) { DualLogError("Failed to open voxen.dem for journal log\n"); return; }

    for (int32_t i = 0; i < eventJournalIndex; i++) { // Write all valid events in eventJournal
        if (eventJournal[i].type != EV_NULL) fwrite(&eventJournal[i], sizeof(Event), 1, fp);
    }

    fflush(fp);
    fclose(fp);
//     DualLog("Writing to event journal at timestamp %f, took %f seconds\n", timestamp, get_time() - timestamp);
}

bool IsPlayableEventType(uint8_t type) {
    if (type == EV_KEYDOWN || type == EV_KEYUP) return true;
    return type != EV_NULL;
}

// Makes use of global activeLogFile handle to read through log and enqueue events with matching frameNum to globalFrameNum
int32_t ReadActiveLog() {
    static bool eof_reached = false; // Track EOF across calls
    Event event;
    int32_t events_processed = 0;
    if (eof_reached) return 2; // Indicate EOF was previously reached

    DualLog("------ ReadActiveLog start for frame %d ------\n",globalFrameNum);
    while (events_processed < MAX_EVENTS_PER_FRAME) {
        size_t read_count = fread(&event, sizeof(Event), 1, activeLogFile);
        if (read_count != 1) {
            if (feof(activeLogFile)) {
                eof_reached = true;
                log_playback = false; // Finished enqueuing last frame, main will finish processing the queue and return input to user.
                return events_processed > 0 ? 0 : 2; // 0 if events were processed, 2 if EOF and no events
            }

            if (ferror(activeLogFile)) { DualLogError("Could not read log file\n"); return -1; }
        }

        if (!IsPlayableEventType(event.type)) continue; // Skip unplayable events

        if (event.frameNum == globalFrameNum) {
            // Enqueue events matching the current frame
            EnqueueEvent(event.type, event.payload1i, event.payload2i, event.payload1f, event.payload2f);
            events_processed++;
            DualLog("Enqueued event %d from log for frame %d\n",event.type,event.frameNum);
        } else if (event.frameNum > globalFrameNum) {
            // Event is for a future frame; seek back and stop processing
            fseek(activeLogFile, -(long)sizeof(Event), SEEK_CUR);
            DualLog("Readback of %d events for this frame %d from log\n",events_processed,globalFrameNum);
            return events_processed > 0 ? 0 : 1; // 0 if events processed, 1 if no matching events
        } // If event.frameNum < globalFrameNum, skip it (past event)
    }

    DualLog("End of log. Readback of %d events for this frame %d from log\n",events_processed,globalFrameNum);
    return events_processed > 0 ? 0 : 1; // 0 if events processed, 1 if limit reached with no matching events
}

// Convert the binary .dem file into human readable text
void JournalDump(const char* dem_file) {
    FILE* fpR = fopen(dem_file, "rb");
    if (!fpR) { DualLogError("Failed to open .dem file\n"); exit(1); }

    FILE* fpW = fopen("./log_dump.txt", "wb");
    if (!fpW) { DualLogError("Failed to open voxen.dem\n"); exit(1); }

    Event event;
    while (fread(&event, sizeof(Event), 1, fpR) == 1) {
        fprintf(fpW,"frameNum: %d, ",event.frameNum);
        fprintf(fpW,"event type: %d, ",event.type);
        fprintf(fpW,"timestamp: %f, ", event.timestamp);
        fprintf(fpW,"delta time: %f, ", event.deltaTime_ns);
        fprintf(fpW,"payload1i: %d, ", event.payload1i);
        fprintf(fpW,"payload2i: %d, ", event.payload2i);
        fprintf(fpW,"payload1f: %f, ", event.payload1f);
        fprintf(fpW,"payload2f: %f\n", event.payload2f); // \n flushes write to file
    }

    fclose(fpW);
    fclose(fpR);
}

// Queue was processed for the frame, clear it so next frame starts fresh.
void clear_ev_queue(void) {
    //  Events will be buffer written until EV_NULL is seen so clear to EV_NULL.
    for (int32_t i=0;i<MAX_EVENTS_PER_FRAME;i++) {
        eventQueue[i].type = EV_NULL;
        eventQueue[i].frameNum = 0;
        eventQueue[i].timestamp = 0.0;
        eventQueue[i].deltaTime_ns = 0.0;
    }

    eventIndex = 0;
    eventQueueEnd = 0;
}

// Process the entire event queue. Events might add more new events to the queue.
// Intended to be called once per loop iteration by the main loop.
int32_t EventQueueProcess(void) {
    int32_t status = 0;
    double timestamp = 0.0;
    int32_t eventCount = 0;
    for (int32_t i=0;i<MAX_EVENTS_PER_FRAME;i++) {
        if (eventQueue[i].type != EV_NULL) {
            eventCount++;
        }
    }

    if (eventCount > maxEventCount_debug) maxEventCount_debug = eventCount;
    eventIndex = 0;
    while (eventIndex < MAX_EVENTS_PER_FRAME) {
        if (eventQueue[eventIndex].type == EV_NULL) break; // End of queue

        eventQueue[eventIndex].frameNum = globalFrameNum;
        timestamp = current_time;
        eventQueue[eventIndex].timestamp = timestamp;
        eventQueue[eventIndex].deltaTime_ns = timestamp - eventJournal[eventJournalIndex].timestamp; // Twould be zero if eventJournalIndex == 0, no need to try to assign it as something else; avoiding branch.

        // Journal buffer entry of this event.  Still written to during playback for time deltas but never logged to .dem
        eventJournalIndex++; // Increment now to then write event into the journal.
        if (eventJournalIndex >= EVENT_JOURNAL_BUFFER_SIZE || (timestamp - lastJournalWriteTime) > 5.0) {
            if (!log_playback) {
                JournalLog();
                lastJournalWriteTime = get_time();
            }

            clear_ev_journal(); // Also sets eventJournalIndex to 0.
        }

        eventJournal[eventJournalIndex].frameNum = eventQueue[eventIndex].frameNum;
        eventJournal[eventJournalIndex].type = eventQueue[eventIndex].type;
        eventJournal[eventJournalIndex].timestamp = eventQueue[eventIndex].timestamp;
        eventJournal[eventJournalIndex].deltaTime_ns = eventQueue[eventIndex].deltaTime_ns;
        eventJournal[eventJournalIndex].payload1i = eventQueue[eventIndex].payload1i;
        eventJournal[eventJournalIndex].payload2i = eventQueue[eventIndex].payload2i;
        eventJournal[eventJournalIndex].payload1f = eventQueue[eventIndex].payload1f;
        eventJournal[eventJournalIndex].payload2f = eventQueue[eventIndex].payload2f;

        // Execute event after journal buffer entry such that we can dump the
        // journal buffer on error and last entry will be the problematic event.
        status = EventExecute(&eventQueue[eventIndex]);
        if (status) {
            if (status != 1) DualLog("EventExecute returned nonzero status: %d\n", status);
            return status;
        }

        eventIndex++;
    }

    clear_ev_queue();
    return 0;
}
