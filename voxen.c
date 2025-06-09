// File: voxen.c
// Description: A realtime OpenGL based application for experimenting with voxel lighting techniques to derive new methods of high speed accurate lighting in resource constrained environements (e.g. embedded).

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "event.h"

// Window and OpenGL context
SDL_Window *window;
SDL_GLContext gl_context;
int screen_width = 800, screen_height = 600;
TTF_Font *font = NULL;
bool window_has_focus = false;

// Camera variables
float cam_x = 0.0f, cam_y = -4.0f, cam_z = 0.0f; // Camera position
float cam_yaw = 0.0f, cam_pitch = -90.0f;         // Camera orientation
float move_speed = 0.1f;
float mouse_sensitivity = 0.1f;                 // Mouse look sensitivity
float M_PI = 3.141592653f;

// Input states
bool keys[SDL_NUM_SCANCODES] = {0}; // SDL_NUM_SCANCODES 512b, covers all keys
int mouse_x = 0, mouse_y = 0; // Mouse position

const double time_step = 1.0 / 60.0; // 60fps
double last_time = 0.0;

// Queue for events to process this frame
Event eventQueue[MAX_EVENTS_PER_FRAME];
int eventJournalIndex;

// Journal buffer for event history to write into the log/demo file
Event eventJournal[EVENT_JOURNAL_BUFFER_SIZE];

int eventIndex; // Event that made it to the counter.  Indices below this were
                // already executed and walked away from the counter.

int eventQueueEnd; // End of the waiting line

// Intended to be called after each buffered write to the logfile in .dem
// format which is custom but similar concept to Quake 1 demos.
void clear_ev_journal(void) {
    //  Events will be buffer written until EV_NULL is seen so clear to EV_NULL.
    for (int i=0;i<EVENT_JOURNAL_BUFFER_SIZE;i++) {
        eventJournal[i].type = EV_NULL;
        eventJournal[i].timestamp = 0.0;
        eventJournal[i].deltaTime_ns = 0.0;
    }

    eventJournalIndex = 0; // Restart at the beginning.
}

// Queue was processed for the frame, clear it so next frame starts fresh.
void clear_ev_queue(void) {
    //  Events will be buffer written until EV_NULL is seen so clear to EV_NULL.
    for (int i=0;i<MAX_EVENTS_PER_FRAME;i++) {
        eventQueue[i].type = EV_NULL;
        eventQueue[i].timestamp = 0.0;
        eventQueue[i].deltaTime_ns = 0.0;
    }

    eventIndex = 0;
    eventQueueEnd = 0;
}

// Function to get current time in nanoseconds
double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_nsec;
}

double get_time_secs(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        fprintf(stderr, "Error: clock_gettime failed\n");
        return 0.0;
    }

    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9; // Full time in seconds
}

// Initializes unified event system variables
int EventInit(void) {
    // Initialize the eventQueue as empty
    for (int i=0;i<MAX_EVENTS_PER_FRAME;i++) {
        eventQueue[i].type = EV_NULL;
        eventQueue[i].timestamp = 0.0;
        eventQueue[i].deltaTime_ns = 0.0;
    }

    clear_ev_journal(); // Initialize the event journal as empty.

    eventIndex = 0;
    eventQueue[eventIndex].type = EV_INIT;
    eventQueue[eventIndex].timestamp = get_time();
    eventQueue[eventIndex].deltaTime_ns = 0.0;
    return 0;
}

int EnqueueEvent(uint8_t type, uint32_t payload1u, uint32_t payload2u, float payload1f, float payload2f) {
    eventQueueEnd++;
    if (eventQueueEnd >= MAX_EVENTS_PER_FRAME) eventQueueEnd--;

    printf("Enqueued event type %d",type);
    eventQueue[eventQueueEnd].type = type;
    eventQueue[eventQueueEnd].timestamp = 0;
    eventQueue[eventQueueEnd].payload1u = payload1u;
    eventQueue[eventQueueEnd].payload2u = payload2u;
    eventQueue[eventQueueEnd].payload1f = payload1f;
    eventQueue[eventQueueEnd].payload2f = payload2f;
    return 0;
}

int EnqueuEvent_UintUint(uint8_t type, uint32_t payload1u, uint32_t payload2u) {
    return EnqueueEvent(type,payload1u,payload2u,0.0f,0.0f);
}

int EnqueuEvent_Uint(uint8_t type, uint32_t payload1u) {
    return EnqueueEvent(type,payload1u,0u,0.0f,0.0f);
}

int EnqueuEvent_FloatFloat(uint8_t type, float payload1f, float payload2f) {
    return EnqueueEvent(type,0u,0u,payload1f,payload2f);
}

int EnqueuEvent_Float(uint8_t type, float payload1f) {
    return EnqueueEvent(type,0u,0u,payload1f,0.0f);
}

// Enqueues an event with type only and no payload values.
int EnqueuEvent_Simple(uint8_t type) {
    return EnqueueEvent(type,0u,0u,0.0f,0.0f);
}

int Input_KeyDown(uint32_t scancode) {
    keys[scancode] = true;
    return 0;
}

int Input_KeyUp(uint32_t scancode) {
    keys[scancode] = false;
    return 0;
}

int Input_MouseMove(float xrel, float yrel) {
    cam_yaw += xrel * mouse_sensitivity;
    cam_pitch += yrel * mouse_sensitivity;
    if (cam_pitch < -179.0f) cam_pitch = -179.0f;
    if (cam_pitch > -1.0f) cam_pitch = -1.0f;
    return 0;
}

int ClearFrameBuffers(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return 0;
}

// Process the entire event queue. Events might add more new events to the queue.
// Intended to be called once per loop iteration by the main loop.
int EventQueueProcess(void) {
    int status = 0;
    double timestamp = 0.0;
    while (eventIndex < MAX_EVENTS_PER_FRAME) {
        if (eventQueue[eventIndex].type == EV_NULL) break; // End of queue

        timestamp = get_time();
        eventQueue[eventIndex].timestamp = timestamp;
        eventQueue[eventIndex].deltaTime_ns = timestamp - eventJournal[eventJournalIndex].timestamp; // Twould be zero if eventJournalIndex == 0, no need to try to assign it as something else; avoiding branch.

        // Journal buffer entry of this event
        eventJournalIndex++; // Increment now to then write event into the journal.
        if (eventJournalIndex >= EVENT_JOURNAL_BUFFER_SIZE) {
            // TODO: Write to journal log file ./voxen.dem WriteJournalBuffer();
            clear_ev_journal(); // Also sets eventJournalIndex to 0.
        }

        eventJournal[eventJournalIndex].type = eventQueue[eventIndex].type;
        eventJournal[eventJournalIndex].timestamp = eventQueue[eventIndex].timestamp;
        eventJournal[eventJournalIndex].deltaTime_ns = eventQueue[eventIndex].deltaTime_ns;
        eventJournal[eventJournalIndex].payload1u = eventQueue[eventIndex].payload1u;
        eventJournal[eventJournalIndex].payload2u = eventQueue[eventIndex].payload2u;
        eventJournal[eventJournalIndex].payload1f = eventQueue[eventIndex].payload1f;
        eventJournal[eventJournalIndex].payload2f = eventQueue[eventIndex].payload2f;
        printf("t:%d,ts:%f,dt:%f,p1u:%d,p2u:%d,p1f:%f,p2f:%f",
               eventQueue[eventIndex].type,
               eventQueue[eventIndex].timestamp,
               eventQueue[eventIndex].deltaTime_ns,
               eventQueue[eventIndex].payload1u,
               eventQueue[eventIndex].payload2u,
               eventQueue[eventIndex].payload1f,
               eventQueue[eventIndex].payload2f);

        // Execute event after journal buffer entry such that we can dump the
        // journal buffer on error and last entry will be the problematic event.
        status = EventExecute(&eventQueue[eventIndex]);
        if (status) return status;

        eventIndex++;
    }

    clear_ev_queue();
    return 0;
}

// Function to set up OpenGL projection
void setup_projection(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)screen_width / screen_height;
    float fov = 90.0f, near = 0.1f, far = 100.0f;
    float f = 1.0f / tan(fov * M_PI / 360.0f);
    float proj[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (far + near) / (near - far), -1,
        0, 0, (2 * far * near) / (near - far), 0
    };
    glMultMatrixf(proj);
    glMatrixMode(GL_MODELVIEW);
}

// Renders text at x,y coordinates specified using pointer to the string array.
void render_debug_text(float x, float y, const char *text, SDL_Color color) {
    if (!font || !text) { fprintf(stderr, "Font or text is NULL\n"); return; } // Skip if font not loaded

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) { fprintf(stderr, "TTF_RenderText_Solid failed: %s\n", TTF_GetError()); return; }

    SDL_Surface *rgba_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba_surface) { fprintf(stderr, "SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError()); return; }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_surface->w, rgba_surface->h, 0,
    GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screen_width, 0, screen_height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(1, 1, 1); // White multiplier (color from texture)
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(x, screen_height - y - rgba_surface->h);
    glTexCoord2f(1, 1); glVertex2f(x + rgba_surface->w, screen_height - y - rgba_surface->h);
    glTexCoord2f(1, 0); glVertex2f(x + rgba_surface->w, screen_height - y);
    glTexCoord2f(0, 0); glVertex2f(x, screen_height - y);
    glEnd ();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDeleteTextures(1, &texture);
    SDL_FreeSurface(rgba_surface);
}

// Function to draw a cube
void draw_cube(void) {
    glBegin(GL_QUADS);

    // Front face
    glColor3f(1, 0, 0);
    glNormal3f(0, 0, 1); // Normal for lighting
    glVertex3f(-1, -1, 1); glVertex3f(1, -1, 1);
    glVertex3f(1, 1, 1); glVertex3f(-1, 1, 1);

    // Back face
    glColor3f(0, 1, 0);
    glNormal3f(0, 0, -1);
    glVertex3f(-1, -1, -1); glVertex3f(-1, 1, -1);
    glVertex3f(1, 1, -1); glVertex3f(1, -1, -1);

    // Top face
    glColor3f(0, 0, 1);
    glVertex3f(-1, 1, -1); glVertex3f(-1, 1, 1);
    glVertex3f(1, 1, 1); glVertex3f(1, 1, -1);

    // Bottom face
    glColor3f(1, 1, 0);
    glNormal3f(0, -1, 0);
    glVertex3f(-1, -1, -1); glVertex3f(1, -1, -1);
    glVertex3f(1, -1, 1); glVertex3f(-1, -1, 1);

    // Right face
    glColor3f(1, 0, 1);
    glNormal3f(1, 0, 0);
    glVertex3f(1, -1, -1); glVertex3f(1, 1, -1);
    glVertex3f(1, 1, 1); glVertex3f(1, -1, 1);

    // Left face
    glColor3f(0, 1, 1);
    glNormal3f(-1, 0, 0);
    glVertex3f(-1, -1, -1); glVertex3f(-1, -1, 1);
    glVertex3f(-1, 1, 1); glVertex3f(-1, 1, -1);
    glEnd();
}

void PositionCamera(void) {
    glRotatef(cam_pitch, 1, 0, 0);
    glRotatef(cam_yaw, 0, 0, 1);
    glTranslatef(-cam_x, -cam_y, -cam_z);
}

int RenderStaticMeshes(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setup_projection(); // Reset projection for 3D
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    PositionCamera();

    // Set up lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    float ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float light_pos0[] = {2.0f, 3.0f, 2.0f, 1.0f};
    float light_pos1[] = {2.0f, -3.0f, 2.0f, 1.0f};
    float diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT1, GL_POSITION, light_pos1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glEnable(GL_COLOR_MATERIAL); // Use glColor for material
    glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glShadeModel(GL_SMOOTH);

    draw_cube();

    return 0;
}

int RenderUI(void) {
    glDisable(GL_LIGHTING); // Disable lighting for text

    // Draw debug text
    char text[64];
    snprintf(text, sizeof(text), "x: %.2f y: %.2f z: %.2f", cam_x, cam_y, cam_z);
    SDL_Color textCol = {255, 255, 255, 255}; // White
    render_debug_text(10, 10, text, textCol); // Top-left corner (10, 10)
    return 0;
}

// Update camera based on input
void ProcessInput(void) {
    // WASD movement (fixed to match inverted mouselook)
    float yaw_rad = -cam_yaw * M_PI / 180.0f;
//     float pitch_rad = cam_pitch * M_PI / 180.0f;
    // Forward direction (Z-up, adjusted for yaw and pitch)
    float facing_x = sin(yaw_rad);
    float facing_y = -cos(yaw_rad);
    // Normalize
    float len = sqrt(facing_x * facing_x + facing_y * facing_y);
    if (len > 0) { facing_x /= len; facing_y /= len; }
    // Strafe direction (perpendicular)
    float strafe_x = -facing_y;
    float strafe_y = facing_x;

    if (keys[SDL_SCANCODE_F]) {
        cam_x -= move_speed * facing_x; // Move forward (inverted)
        cam_y -= move_speed * facing_y;
    } else if (keys[SDL_SCANCODE_S]) {
        cam_x += move_speed * facing_x; // Move backward
        cam_y += move_speed * facing_y;
    }
    if (keys[SDL_SCANCODE_A]) {
        cam_x -= move_speed * strafe_x; // Strafe left
        cam_y -= move_speed * strafe_y;
    } else if (keys[SDL_SCANCODE_D]) {
        cam_x += move_speed * strafe_x; // Strafe right
        cam_y += move_speed * strafe_y;
    }
    if (keys[SDL_SCANCODE_V]) {
        cam_z += move_speed;
    } else if (keys[SDL_SCANCODE_C]) {
        cam_z -= move_speed;
    }
}

int InitializeEnvironment(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError()); return 1; }
    if (TTF_Init() < 0) { fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError()); return 2; }

    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 10);
    if (!font) { fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError()); return 3; }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    window = SDL_CreateWindow("Voxen, the OpenGL Voxel Lit Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 4;
    }

    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return 5;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE); // Enable backface culling
    glCullFace(GL_BACK);
    glEnable(GL_NORMALIZE); // Normalize normals for lighting
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); // One-sided lighting
    glViewport(0, 0, screen_width, screen_height);
    setup_projection();
    return 0;
}

int ExitCleanup(int status) {
    switch(status) {
        case 2: SDL_Quit(); break; // SDL was init'ed, so SDL_Quit
        case 3: TTF_Quit(); SDL_Quit(); break; // TTF was init'ed, so also TTF Quit
        case 4: if (font) TTF_CloseFont(font); // Font was loaded, so clean it up
                TTF_Quit(); SDL_Quit(); break;
        case 5: SDL_DestroyWindow(window); // SDL window was created so destroy the window
                if (font) TTF_CloseFont(font);
                TTF_Quit(); SDL_Quit(); break;
        default:SDL_DestroyWindow(window); SDL_GL_DeleteContext(gl_context); // GL context was created so delete the context
                if (font) TTF_CloseFont(font);
                TTF_Quit(); SDL_Quit(); break;
    }

    return status;
}

int EventExecute(Event* event) {
    switch(event->type) {
        case EV_INIT: return InitializeEnvironment();
        case EV_KEYDOWN: return Input_KeyDown(event->payload1u);
        case EV_KEYUP: return Input_KeyUp(event->payload1u);
        case EV_MOUSEMOVE: return Input_MouseMove(event->payload1f,event->payload2f);
        case EV_CLEAR_FRAME_BUFFERS: return ClearFrameBuffers();
        case EV_RENDER_STATICS: return RenderStaticMeshes();
        case EV_RENDER_UI: return RenderUI();
        case EV_QUIT: return 1; break;
    }

    return 99; // Something went wrong
}

int main(void) {
    int exitCode = 0;
    exitCode = EventInit();
    if (exitCode) return ExitCleanup(exitCode);

    EnqueuEvent_Simple(EV_INIT);
    double accumulator = 0.0;
    last_time = get_time_secs();
    while(1) {
        // Enqueue input events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) EnqueuEvent_Simple(EV_QUIT);
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) EnqueuEvent_Simple(EV_QUIT);
                else EnqueuEvent_Uint(EV_KEYDOWN,(uint32_t)event.key.keysym.scancode);
            } else if (event.type == SDL_KEYUP) {
                EnqueuEvent_Uint(EV_KEYUP,(uint32_t)event.key.keysym.scancode);
            } else if (event.type == SDL_MOUSEMOTION && window_has_focus) {
                EnqueuEvent_FloatFloat(EV_MOUSEMOVE,event.motion.xrel,event.motion.yrel);

            // These aren't really events so just handle them here.
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    window_has_focus = true;
                    //SDL_SetRelativeMouseMode(SDL_TRUE);
                } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    window_has_focus = false;
                    //SDL_SetRelativeMouseMode(SDL_FALSE);
                }
            }
        }

        double current_time = get_time_secs();
        double frame_time = current_time - last_time;
        last_time = current_time;
        accumulator += frame_time;
        while (accumulator >= time_step) {
            if (window_has_focus) ProcessInput();
            accumulator -= time_step;
        }

        // Enqueue render events in pipeline order
        EnqueuEvent_Simple(EV_CLEAR_FRAME_BUFFERS);
        EnqueuEvent_Simple(EV_RENDER_STATICS);
        EnqueuEvent_Simple(EV_RENDER_UI);
        exitCode = EventQueueProcess(); // Do everything
        if (exitCode) break;

        SDL_GL_SwapWindow(window); // Present frame
    }

    // Cleanup
    return ExitCleanup(exitCode);
}
