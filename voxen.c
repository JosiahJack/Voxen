// File: voxen.c
// Description: A realtime OpenGL based application for experimenting with voxel lighting techniques to derive new methods of high speed accurate lighting in resource constrained environements (e.g. embedded).

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "event.h"
#include "quaternion.h"
#include "matrix.h"
#include "constants.h"

// Bindless texture function prototypes
// GLuint64 glGetTextureHandleARB(GLuint texture);
// void glMakeTextureHandleResidentARB(GLuint64 handle);
// void glMakeTextureHandleNonResidentARB(GLuint64 handle);

// Window and OpenGL context
SDL_Window *window;
SDL_GLContext gl_context;
int screen_width = 800, screen_height = 600;
TTF_Font *font = NULL;
bool window_has_focus = false;

// GL Shaders
GLuint shaderProgram;
GLuint textShaderProgram; // For text
GLuint vao, vbo; // Vertex Array Object and Vertex Buffer Object
GLuint textVAO, textVBO; // For text
bool use_bindless_textures = false;
const char *vertexShaderSource =
    "#version 450 core\n"
    "\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "layout(location = 2) in vec2 aTexCoord;\n"
    "layout(location = 3) in float aTexIndex;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec2 TexCoord;\n"
    "out float TexIndex;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = projection * view * vec4(aPos, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "    TexIndex = aTexIndex;\n"
    "}\n";

const char *fragmentShaderBindless = "#version 450 core\n"
    "#extension GL_ARB_bindless_texture : require\n"
    "in vec2 TexCoord;\n"
    "in float TexIndex;\n"
    "out vec4 FragColor;\n"
    "layout(bindless_sampler) uniform sampler2D uTextures[3];\n"
    "void main() {\n"
    "    int index = int(TexIndex);\n"
    "    FragColor = texture(uTextures[index], TexCoord);\n"
    "}\n";

const char *fragmentShaderTraditional = "#version 450 core\n"
    "in vec2 TexCoord;\n"
    "in float TexIndex;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D uTextures[3];\n"
    "void main() {\n"
    "    int index = int(TexIndex);\n"
    "    FragColor = texture(uTextures[index], TexCoord);\n"
    "}\n";


// Vertex Shader for Text
const char *textVertexShaderSource =
    "#version 450 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "uniform mat4 projection;\n"
    "out vec2 TexCoord;\n"
    "void main() {\n"
    "    gl_Position = projection * vec4(aPos, 0.0, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";

// Fragment Shader for Text
const char *textFragmentShaderSource =
    "#version 450 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D textTexture;\n"
    "uniform vec4 textColor;\n"
    "void main() {\n"
    "    vec4 sampled = texture(textTexture, TexCoord);\n"
    "    FragColor = vec4(textColor.rgb, sampled.a * textColor.a);\n"
    "}\n";

int CompileShaders(void) {
    // Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Vertex Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragSource = use_bindless_textures ? fragmentShaderBindless : fragmentShaderTraditional;
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Fragment Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Shader Program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Text Vertex Shader
    GLuint textVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(textVertexShader, 1, &textVertexShaderSource, NULL);
    glCompileShader(textVertexShader);
    glGetShaderiv(textVertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(textVertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Text Vertex Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Text Fragment Shader
    GLuint textFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(textFragmentShader, 1, &textFragmentShaderSource, NULL);
    glCompileShader(textFragmentShader);
    glGetShaderiv(textFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(textFragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Text Fragment Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Text Shader Program
    textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, textVertexShader);
    glAttachShader(textShaderProgram, textFragmentShader);
    glLinkProgram(textShaderProgram);
    glGetProgramiv(textShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(textShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Text Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }

    glDeleteShader(textVertexShader);
    glDeleteShader(textFragmentShader);
    return 0;
}

// Cube Geometry (positions, normals, tex coords, texture index)
float cubeVertices[] = {
    // Positions          // Normals           // Tex Coords  // Tex Index
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,  2.0f, // Top (med1_9.png)
     1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,  2.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,  2.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,  2.0f,

    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,  1.0f, // Bottom (med1_7.png)
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,  0.0f, // Side (med1_1.png)
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,  0.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,  0.0f,

    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  0.0f, // Side (med1_1.png)
     1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  0.0f,
     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,  0.0f,

     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f, // Side (med1_1.png)
     1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  0.0f,
     1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f,

    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f, // Side (med1_1.png)
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f,
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  0.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  0.0f
};

void SetupCube(void) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Tex Coord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Tex Index
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
}

// Quad for text (2 triangles, positions and tex coords)
float textQuadVertices[] = {
    // Positions   // Tex Coords
    0.0f, 0.0f,    0.0f, 0.0f, // Bottom-left
    1.0f, 0.0f,    1.0f, 0.0f, // Bottom-right
    1.0f, 1.0f,    1.0f, 1.0f, // Top-right
    0.0f, 1.0f,    0.0f, 1.0f  // Top-left
};

void SetupTextQuad(void) {
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textQuadVertices), textQuadVertices, GL_STATIC_DRAW);
    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Tex Coord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Camera variables
float cam_x = 0.0f, cam_y = -4.0f, cam_z = 0.0f; // Camera position
// float cam_yaw = 0.0f, cam_pitch = 0.0f;         // Camera orientation
Quaternion cam_rotation;
float cam_yaw = 180.0f;
float cam_pitch = 90.0f;
float cam_roll = 0.0f;
float move_speed = 0.1f;
float mouse_sensitivity = 0.1f;                 // Mouse look sensitivity

// Input states
bool keys[SDL_NUM_SCANCODES] = {0}; // SDL_NUM_SCANCODES 512b, covers all keys
int mouse_x = 0, mouse_y = 0; // Mouse position

const double time_step = 1.0 / 60.0; // 60fps
double last_time = 0.0;

// Data
// ============================================================================

// Textures
SDL_Surface *textureSurfaces[TEXTURE_COUNT];
GLuint textureIDs[TEXTURE_COUNT];
GLuint64 textureHandles[TEXTURE_COUNT];
const char *texturePaths[TEXTURE_COUNT] = {
    "./Textures/med1_1.png",
    "./Textures/med1_7.png",
    "./Textures/med1_9.png"
};

int LoadTextures(void) {
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        SDL_Surface *surface = IMG_Load(texturePaths[i]);
        if (!surface) {
            fprintf(stderr, "IMG_Load failed for %s: %s\n", texturePaths[i], IMG_GetError());
            return 1;
        }
        textureSurfaces[i] = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
        SDL_FreeSurface(surface);
        if (!textureSurfaces[i]) {
            fprintf(stderr, "SDL_ConvertSurfaceFormat failed for %s: %s\n", texturePaths[i], SDL_GetError());
            return 1;
        }
        glGenTextures(1, &textureIDs[i]);
        glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSurfaces[i]->w, textureSurfaces[i]->h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, textureSurfaces[i]->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (use_bindless_textures) {
            textureHandles[i] = glGetTextureHandleARB(textureIDs[i]); // Bindless handle
            glMakeTextureHandleResidentARB(textureHandles[i]); // Make resident
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    return 0;
}

// ============================================================================

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

double get_time(void) {
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
    clear_ev_queue();
    clear_ev_journal(); // Initialize the event journal as empty.
    eventQueue[eventIndex].type = EV_INIT;
    eventQueue[eventIndex].timestamp = get_time();
    eventQueue[eventIndex].deltaTime_ns = 0.0;
    return 0;
}

int EnqueueEvent(uint8_t type, uint32_t payload1u, uint32_t payload2u, float payload1f, float payload2f) {
    if (eventQueueEnd >= MAX_EVENTS_PER_FRAME) { printf("Queue buffer filled!\n"); return 1; }

    //printf("Enqueued event type %d, at index %d\n",type,eventQueueEnd);
    eventQueue[eventQueueEnd].type = type;
    eventQueue[eventQueueEnd].timestamp = 0;
    eventQueue[eventQueueEnd].payload1u = payload1u;
    eventQueue[eventQueueEnd].payload2u = payload2u;
    eventQueue[eventQueueEnd].payload1f = payload1f;
    eventQueue[eventQueueEnd].payload2f = payload2f;
    eventQueueEnd++;
    return 0;
}

int EnqueueEvent_UintUint(uint8_t type, uint32_t payload1u, uint32_t payload2u) {
    return EnqueueEvent(type,payload1u,payload2u,0.0f,0.0f);
}

int EnqueueEvent_Uint(uint8_t type, uint32_t payload1u) {
    return EnqueueEvent(type,payload1u,0u,0.0f,0.0f);
}

int EnqueueEvent_FloatFloat(uint8_t type, float payload1f, float payload2f) {
    return EnqueueEvent(type,0u,0u,payload1f,payload2f);
}

int EnqueueEvent_Float(uint8_t type, float payload1f) {
    return EnqueueEvent(type,0u,0u,payload1f,0.0f);
}

// Enqueues an event with type only and no payload values.
int EnqueueEvent_Simple(uint8_t type) {
    return EnqueueEvent(type,0u,0u,0.0f,0.0f);
}

bool in_cyberspace = true;

void Input_MouselookApply() {
    if (in_cyberspace) quat_from_yaw_pitch_roll(&cam_rotation,cam_yaw,cam_pitch,cam_roll);
    else quat_from_yaw_pitch(&cam_rotation,cam_yaw,cam_pitch);
}

int Input_KeyDown(uint32_t scancode) {
    keys[scancode] = true;
    if (scancode == SDL_SCANCODE_TAB) {
        in_cyberspace = !in_cyberspace;
        cam_roll = 0.0f; // Reset roll for sanity
        Input_MouselookApply();
    }
    return 0;
}

int Input_KeyUp(uint32_t scancode) {
    keys[scancode] = false;
    return 0;
}



int Input_MouseMove(float xrel, float yrel) {
    cam_yaw += xrel * mouse_sensitivity;
    cam_pitch += yrel * mouse_sensitivity;
    if (cam_pitch > 179.0f) cam_pitch = 179.0f;
    if (cam_pitch < 1.0f) cam_pitch = 1.0f;
    Input_MouselookApply();
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
    int eventCount = 0;
    for (int i=0;i<MAX_EVENTS_PER_FRAME;i++) {
        if (eventQueue[i].type != EV_NULL) {
            //printf("Queue contains:  ");
            //printf("%d, ", eventQueue[i].type);
            eventCount++;
        }
    }

//     printf("\n");

    //printf("EventQueueProcess start with %d events in queue\n",eventCount);
    eventIndex = 0;
    while (eventIndex < MAX_EVENTS_PER_FRAME) {
        //printf("Iterating over event queue, eventIndex this loop: %d\n",eventIndex);
        if (eventQueue[eventIndex].type == EV_NULL) {
            break; // End of queue
        }

        timestamp = get_time();
        eventQueue[eventIndex].timestamp = timestamp;
        eventQueue[eventIndex].deltaTime_ns = timestamp - eventJournal[eventJournalIndex].timestamp; // Twould be zero if eventJournalIndex == 0, no need to try to assign it as something else; avoiding branch.
        //printf("Current event deltaTime: %f\n",eventQueue[eventIndex].deltaTime_ns);
        // Journal buffer entry of this event
        eventJournalIndex++; // Increment now to then write event into the journal.
        if (eventJournalIndex >= EVENT_JOURNAL_BUFFER_SIZE) {
            // TODO: Write to journal log file ./voxen.dem WriteJournalBuffer();
            clear_ev_journal(); // Also sets eventJournalIndex to 0.
            //printf("Event queue cleared after journal filled\n");
        }

        eventJournal[eventJournalIndex].type = eventQueue[eventIndex].type;
        eventJournal[eventJournalIndex].timestamp = eventQueue[eventIndex].timestamp;
        eventJournal[eventJournalIndex].deltaTime_ns = eventQueue[eventIndex].deltaTime_ns;
        eventJournal[eventJournalIndex].payload1u = eventQueue[eventIndex].payload1u;
        eventJournal[eventJournalIndex].payload2u = eventQueue[eventIndex].payload2u;
        eventJournal[eventJournalIndex].payload1f = eventQueue[eventIndex].payload1f;
        eventJournal[eventJournalIndex].payload2f = eventQueue[eventIndex].payload2f;
//         printf("t:%d,ts:%f,dt:%f,p1u:%d,p2u:%d,p1f:%f,p2f:%f\n",
//                eventQueue[eventIndex].type,
//                eventQueue[eventIndex].timestamp,
//                eventQueue[eventIndex].deltaTime_ns,
//                eventQueue[eventIndex].payload1u,
//                eventQueue[eventIndex].payload2u,
//                eventQueue[eventIndex].payload1f,
//                eventQueue[eventIndex].payload2f);

        // Execute event after journal buffer entry such that we can dump the
        // journal buffer on error and last entry will be the problematic event.
        status = EventExecute(&eventQueue[eventIndex]);
        if (status) {
            if (status != 1) printf("EventExecute returned nonzero status: %d !", status);
            return status;
        }

        eventIndex++;
    }

    clear_ev_queue();
    return 0;
}

// Matrix helper for 2D orthographic projection
void mat4_ortho(float* m, float left, float right, float bottom, float top, float near, float far) {
    mat4_identity(m);
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (far - near);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
}

// Renders text at x,y coordinates specified using pointer to the string array.
void render_debug_text(float x, float y, const char *text, SDL_Color color) {
    if (!font || !text) { fprintf(stderr, "Font or text is NULL\n"); return; }
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) { fprintf(stderr, "TTF_RenderText_Solid failed: %s\n", TTF_GetError()); return; }
    SDL_Surface *rgba_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba_surface) { fprintf(stderr, "SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError()); return; }

    // Create and bind texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_surface->w, rgba_surface->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Use text shader
    glUseProgram(textShaderProgram);

    // Set up orthographic projection
    float projection[16];
    mat4_ortho(projection, 0.0f, (float)screen_width, (float)screen_height, 0.0f, -1.0f, 1.0f);
    GLint projLoc = glGetUniformLocation(textShaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);

    // Set text color (convert SDL_Color to 0-1 range)
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    float a = color.a / 255.0f;
    GLint colorLoc = glGetUniformLocation(textShaderProgram, "textColor");
    glUniform4f(colorLoc, r, g, b, a);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLint texLoc = glGetUniformLocation(textShaderProgram, "textTexture");
    glUniform1i(texLoc, 0);

    // Enable blending for text transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // Disable depth test for 2D overlay

    // Bind VAO and adjust quad position/size
    glBindVertexArray(textVAO);
    float scaleX = (float)rgba_surface->w;
    float scaleY = (float)rgba_surface->h;
    float vertices[] = {
        x,          y,          0.0f, 0.0f, // Bottom-left
        x + scaleX, y,          1.0f, 0.0f, // Bottom-right
        x + scaleX, y + scaleY, 1.0f, 1.0f, // Top-right
        x,          y + scaleY, 0.0f, 1.0f  // Top-left
    };
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Render quad (two triangles)
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); // Re-enable depth test for 3D rendering
    glUseProgram(0);
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(rgba_surface);
}

int RenderStaticMeshes(void) {
    glUseProgram(shaderProgram);
    // Set up matrices
    float view[16], projection[16];
    float fov = 65.0f;
    mat4_perspective(projection, fov, (float)screen_width / screen_height, 0.1f, 100.0f);
    mat4_lookat(view, cam_x, cam_y, cam_z, &cam_rotation);
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    if (use_bindless_textures) {
        GLint texLoc = glGetUniformLocation(shaderProgram, "uTextures");
        glUniformHandleui64vARB(texLoc, TEXTURE_COUNT, textureHandles);
    } else {
        for (int i = 0; i < TEXTURE_COUNT; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
            char uniformName[32];
            snprintf(uniformName, sizeof(uniformName), "uTextures[%d]", i);
            GLint texLoc = glGetUniformLocation(shaderProgram, uniformName);
            glUniform1i(texLoc, i);
        }
    }

    // Render cube
    glBindVertexArray(vao);
    glDrawArrays(GL_QUADS, 0, 24); // 24 vertices for 6 quads
    glBindVertexArray(0);
    glUseProgram(0);
    return 0;
}

int RenderUI(double deltaTime) {
    glDisable(GL_LIGHTING); // Disable lighting for text
    SDL_Color textCol = {255, 255, 255, 255}; // White

    // Draw debug text
    char text0[64];
    snprintf(text0, sizeof(text0), "Frame time: %f", deltaTime * 1000.0);
    render_debug_text(10, 10, text0, textCol); // Top-left corner (10, 10)

    char text1[64];
    snprintf(text1, sizeof(text1), "x: %.2f, y: %.2f, z: %.2f", cam_x, cam_y, cam_z);
    render_debug_text(10, 25, text1, textCol); // Top-left corner (10, 10)

    float cam_quat_yaw = 0.0f;
    float cam_quat_pitch = 0.0f;
    float cam_quat_roll = 0.0f;
    quat_to_euler(&cam_rotation,&cam_quat_yaw,&cam_quat_pitch,&cam_quat_roll);
    char text2[64];
    snprintf(text2, sizeof(text2), "cam yaw: %.2f, cam pitch: %.2f, cam roll: %.2f", cam_yaw, cam_pitch, cam_roll);
    render_debug_text(10, 40, text2, textCol);

    char text3[64];
    snprintf(text3, sizeof(text3), "cam quat yaw: %.2f, cam quat pitch: %.2f, cam quat roll: %.2f", cam_quat_yaw, cam_quat_pitch, cam_quat_roll);
    render_debug_text(10, 55, text3, textCol);
    return 0;
}

// Update camera position based on input
void ProcessInput(void) {
    // Extract forward and right vectors from quaternion
    float rotation[16];
    quat_to_matrix(&cam_rotation, rotation);
    float facing_x = rotation[8];  // Forward X
    float facing_y = rotation[9];  // Forward Y
    float facing_z = rotation[10]; // Forward Z
    float strafe_x = rotation[0];  // Right X
    float strafe_y = rotation[1];  // Right Y
    float strafe_z = rotation[2];  // Right Z

    // Normalize forward
    float len = sqrt(facing_x * facing_x + facing_y * facing_y + facing_z * facing_z);
    if (len > 0) {
        facing_x /= len;
        facing_y /= len;
        facing_z /= len;
    }
    // Normalize strafe
    len = sqrt(strafe_x * strafe_x + strafe_y * strafe_y + strafe_z * strafe_z);
    if (len > 0) {
        strafe_x /= len;
        strafe_y /= len;
        strafe_z /= len;
    }

    if (keys[SDL_SCANCODE_F]) {
        cam_x += move_speed * facing_x; // Move forward
        cam_y += move_speed * facing_y;
        cam_z += move_speed * facing_z;
    } else if (keys[SDL_SCANCODE_S]) {
        cam_x -= move_speed * facing_x; // Move backward
        cam_y -= move_speed * facing_y;
        cam_z -= move_speed * facing_z;
    }

    if (keys[SDL_SCANCODE_A]) {
        cam_x -= move_speed * strafe_x; // Strafe left
        cam_y -= move_speed * strafe_y;
        cam_z -= move_speed * strafe_z;
    } else if (keys[SDL_SCANCODE_D]) {
        cam_x += move_speed * strafe_x; // Strafe right
        cam_y += move_speed * strafe_y;
        cam_z += move_speed * strafe_z;
    }

    if (keys[SDL_SCANCODE_V]) {
        cam_z += move_speed; // Move up
    } else if (keys[SDL_SCANCODE_C]) {
        cam_z -= move_speed; // Move down
    }

    if (keys[SDL_SCANCODE_T]) {
        cam_roll += move_speed * 5.0f; // Move up
        Input_MouselookApply();
    } else if (keys[SDL_SCANCODE_Q]) {
        cam_roll -= move_speed * 5.0f; // Move down
        Input_MouselookApply();
    }
}

int InitializeEnvironment(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError()); return 1; }
    if (TTF_Init() < 0) { fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError()); return 2; }
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) { fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError()); return 3; }

    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 10);
    if (!font) { fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError()); return 4; }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    window = SDL_CreateWindow("Voxen, the OpenGL Voxel Lit Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 5;
    }

    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return 6;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    glewExperimental = GL_TRUE; // Enable modern OpenGL support
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "GLEW initialization failed\n");
        return 7;
    }

     // Check bindless texture support
    use_bindless_textures = GLEW_ARB_bindless_texture;
    if (!use_bindless_textures) {
        fprintf(stderr, "GL_ARB_bindless_texture not supported, falling back to traditional binding\n");
    } else {
        printf("use_bindless_textures was true\n");
        fprintf(stderr, "Forcing traditional binding due to potential Intel/Mesa incompatibility\n");
        use_bindless_textures = false;
    }
    // Diagnostic: Print OpenGL version and renderer
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    fprintf(stderr, "OpenGL Version: %s\n", version ? (const char*)version : "unknown");
    fprintf(stderr, "Renderer: %s\n", renderer ? (const char*)renderer : "unknown");

    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE); // Enable backface culling
    glCullFace(GL_BACK);
    glFrontFace(GL_CW); // Flip triangle sorting order
    glEnable(GL_NORMALIZE); // Normalize normals for lighting
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); // One-sided lighting
    glViewport(0, 0, screen_width, screen_height);
    if (!GLEW_ARB_bindless_texture) {
        fprintf(stderr, "GL_ARB_bindless_texture not supported\n");
        return 8;
    }

    if (CompileShaders()) return 9;
    SetupCube();
    SetupTextQuad();
    quat_identity(&cam_rotation);
    Quaternion pitch_quat;
    quat_from_axis_angle(&pitch_quat, 1.0f, 0.0f, 0.0f, -90.0f * M_PI / 180.0f); // Pitch -90° to look toward Y+
    quat_multiply(&cam_rotation, &pitch_quat, &cam_rotation);
    return 0;
}

int ExitCleanup(int status) {
    switch(status) {
        case 2: SDL_Quit(); break; // SDL was init'ed, so SDL_Quit
        case 3: TTF_Quit(); SDL_Quit(); break; // TTF was init'ed, so also TTF Quit
        case 4: IMG_Quit(); TTF_Quit(); SDL_Quit(); break;
        case 5: if (font) TTF_CloseFont(font); // Font was loaded, so clean it up
                TTF_Quit(); SDL_Quit(); break;
        case 6: SDL_DestroyWindow(window); // SDL window was created so destroy the window
                if (font) TTF_CloseFont(font);
                TTF_Quit(); SDL_Quit(); break;
        default:
                for (int i = 0; i < TEXTURE_COUNT; i++) {
                    if (textureHandles[i]) glMakeTextureHandleNonResidentARB(textureHandles[i]);
                    if (textureIDs[i]) glDeleteTextures(1, &textureIDs[i]);
                    if (textureSurfaces[i]) SDL_FreeSurface(textureSurfaces[i]);
                }
                glDeleteProgram(shaderProgram);
                glDeleteVertexArrays(1, &vao);
                glDeleteBuffers(1, &vbo);

                glDeleteProgram(textShaderProgram);
                glDeleteVertexArrays(1, &textVAO);
                glDeleteBuffers(1, &textVBO);

                SDL_DestroyWindow(window); SDL_GL_DeleteContext(gl_context); // GL context was created so delete the context
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
        case EV_RENDER_UI: return RenderUI(get_time() - last_time);
        case EV_LOAD_TEXTURES: return LoadTextures();
        case EV_QUIT: return 1; break;
    }

    return 99; // Something went wrong
}

int main(void) {
    int exitCode = 0;
    exitCode = EventInit();
    if (exitCode) return ExitCleanup(exitCode);

    EnqueueEvent_Simple(EV_INIT);
    EnqueueEvent_Simple(EV_LOAD_TEXTURES);
    double accumulator = 0.0;
    last_time = get_time();
    while(1) {
        // Enqueue input events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) EnqueueEvent_Simple(EV_QUIT);
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) EnqueueEvent_Simple(EV_QUIT);
                else EnqueueEvent_Uint(EV_KEYDOWN,(uint32_t)event.key.keysym.scancode);
            } else if (event.type == SDL_KEYUP) {
                EnqueueEvent_Uint(EV_KEYUP,(uint32_t)event.key.keysym.scancode);
            } else if (event.type == SDL_MOUSEMOTION && window_has_focus) {
                EnqueueEvent_FloatFloat(EV_MOUSEMOVE,event.motion.xrel,event.motion.yrel);

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

        double current_time = get_time();
        double frame_time = current_time - last_time;
        last_time = current_time;
        accumulator += frame_time;
        while (accumulator >= time_step) {
            if (window_has_focus) ProcessInput();
            accumulator -= time_step;
        }

        // Enqueue render events in pipeline order
        EnqueueEvent_Simple(EV_CLEAR_FRAME_BUFFERS);
        EnqueueEvent_Simple(EV_RENDER_STATICS);
        EnqueueEvent_Simple(EV_RENDER_UI);
        exitCode = EventQueueProcess(); // Do everything
        if (exitCode) break;

        SDL_GL_SwapWindow(window); // Present frame
    }

    // Cleanup
    return ExitCleanup(exitCode);
}
