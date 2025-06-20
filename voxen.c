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
#include <string.h>
#include "event.h"
#include "constants.h"
#include "shaders.glsl.h"
#include "input.h"
#include "data_textures.h"
#include "data_models.h"
#include "render.h"
#include "cli_args.h"

// Window
SDL_Window *window;
int screen_width = 800, screen_height = 600;
bool window_has_focus = false;

// Event System states
int maxEventCount_debug = 0;
double lastJournalWriteTime = 0;
uint32_t globalFrameNum = 0;
FILE* activeLogFile;
const char* manualLogName;
bool log_playback = false;
Event eventQueue[MAX_EVENTS_PER_FRAME]; // Queue for events to process this frame
Event eventJournal[EVENT_JOURNAL_BUFFER_SIZE]; // Journal buffer for event history to write into the log/demo file
int eventJournalIndex;
int eventIndex; // Event that made it to the counter.  Indices below this were
                // already executed and walked away from the counter.
int eventQueueEnd; // End of the waiting line
const double time_step = 1.0 / 60.0; // 60fps
double last_time = 0.0;
double current_time = 0.0;
double start_frame_time = 0.0;
                
// OpenGL
SDL_GLContext gl_context;
GLuint shaderProgram;
GLuint textShaderProgram;
GLuint vao, vbo; // Vertex Array Object and Vertex Buffer Object
TTF_Font* font = NULL;
GLuint textVAO, textVBO;

typedef enum {
    SYS_SDL = 0,
    SYS_TTF,
    SYS_IMG,
    SYS_WIN,
    SYS_CTX,
    SYS_OGL,
    SYS_COUNT // Number of subsystems
} SystemType;

bool systemInitialized[SYS_COUNT] = {false,false,false,false,false,false};

int InitializeEnvironment(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError()); return SYS_SDL + 1; }
    systemInitialized[SYS_SDL] = true;
    
    if (TTF_Init() < 0) { fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError()); return SYS_TTF + 1; }
    systemInitialized[SYS_TTF] = true;

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) { fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError()); return SYS_IMG + 1; }
    systemInitialized[SYS_IMG] = true;

    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 10);
    if (!font) { fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError()); return SYS_TTF + 1; }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    window = SDL_CreateWindow("Voxen, the OpenGL Voxel Lit Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return SYS_WIN + 1;
    }
    
    systemInitialized[SYS_WIN] = true;

    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return SYS_CTX + 1;
    }
    
    systemInitialized[SYS_CTX] = true;

    SDL_GL_MakeCurrent(window, gl_context);
    glewExperimental = GL_TRUE; // Enable modern OpenGL support
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "GLEW initialization failed\n");
        return SYS_OGL + 1;
    }
    
    systemInitialized[SYS_OGL] = true;

    // Diagnostic: Print OpenGL version and renderer
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    fprintf(stderr, "OpenGL Version: %s\n", version ? (const char*)version : "unknown");
    fprintf(stderr, "Renderer: %s\n", renderer ? (const char*)renderer : "unknown");

    int vsync_enable = 0;//1; // Set to 0 for false.
    SDL_GL_SetSwapInterval(vsync_enable);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE); // Enable backface culling
    glCullFace(GL_BACK);
    glFrontFace(GL_CW); // Flip triangle sorting order
    glEnable(GL_NORMALIZE); // Normalize normals for lighting
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); // One-sided lighting
    glViewport(0, 0, screen_width, screen_height);

    if (CompileShaders()) return SYS_COUNT + 1;
    SetupTextQuad();
    Input_Init();
    return 0;
}

int ExitCleanup(int status) {
    if (activeLogFile) fclose(activeLogFile); // Close log playback file.

    // OpenGL Cleanup
    if (colorBufferID) glDeleteBuffers(1, &colorBufferID);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (vao) glDeleteVertexArrays(1, &vao);
    for (int i=0;i<MODEL_COUNT;i++) {
        if (vbos[i]) glDeleteBuffers(1, &vbos[i]);
    }

    if (textShaderProgram) glDeleteProgram(textShaderProgram);
    if (textVAO) glDeleteVertexArrays(1, &textVAO);
    if (textVBO) glDeleteBuffers(1, &textVBO);

    // Cleanup initialized systems in reverse order.
    // Independent ifs so that we can exit from anywhere and de-init only as needed.
    
    // if (systemInitialized[SYS_OGL]) Nothing to be done for GLEW de-init.
    
    // Delete context after deleting buffers relevant to that context.
    if (systemInitialized[SYS_CTX]) SDL_GL_DeleteContext(gl_context); // GL context was created so delete the context
    
    if (systemInitialized[SYS_WIN]) SDL_DestroyWindow(window); // SDL window was created so destroy the window
    if (font && systemInitialized[SYS_TTF]) TTF_CloseFont(font); // Font was loaded, so clean it up
    if (systemInitialized[SYS_IMG]) IMG_Quit();
    if (systemInitialized[SYS_TTF]) TTF_Quit(); // TTF was init'ed, so also TTF Quit
    if (systemInitialized[SYS_SDL]) SDL_Quit(); // SDL was init'ed, so SDL_Quit
    return status;
}

// All core engine operations run through the EventExecute as an Event processed
// by the unified event system in the order it was enqueued.

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
        case EV_LOAD_MODELS: return SetupGeometry();
        case EV_QUIT: return 1; break;
    }

    return 99; // Something went wrong
}

int main(int argc, char* argv[]) {
    if (argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) { cli_args_print_version(); return 0; }

    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        cli_args_print_help();
        return 0;
    } else if (argc == 3 && strcmp(argv[1], "dump") == 0) { // Log dump as text
        printf("Converting log: %s ...", argv[2]);
        JournalDump(argv[2]);
        printf("DONE!\n");
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Voxen the OpenGL Voxel Lit Rendering Engine\n");
        printf("-----------------------------------------------------------\n");
        printf("        This is a rendering engine designed for optimized focused\n");
        printf("        usage of OpenGL making maximal use of GPU Driven rendering\n");
        printf("        techniques, a unified event system for debugging and log\n");
        printf("        playback, full mod support loading all data from external\n");
        printf("        files and using definition files for what to do with the\n");
        printf("        data.\n\n");
        printf("        This project aims to have minimal overhead, profiling,\n");
        printf("        traceability, robustness, and low level control.\n\n");
        printf("\n");
        printf("Valid arguments:\n");
        printf(" < none >\n    Runs the engine as normal, loading data from \n    neighbor directories (./Textures, ./Models, etc.)\n\n");
        printf("-v, --version\n    Prints version information\n\n");
        printf("play <file>\n    Plays back recorded log from current directory\n\n");
        printf("record <file>\n    Records all engine events to designated log\n    as a .dem file\n\n");
        printf("dump <file.dem>\n    Dumps the specified log into ./log_dump.txt\n    as human readable text.  You must provide full\n    file name with extension\n\n");
        printf("-h, --help\n    Provides this help text.  Neat!\n\n");
        printf("-----------------------------------------------------------\n");
        return 0;
    } else if (argc == 3 && strcmp(argv[1], "dump") == 0) { // Log dump as text
        printf("Converting log: %s ...", argv[2]);
        JournalDump(argv[2]);
        printf("DONE!\n");
        return 0;
    }

    int exitCode = 0;
    globalFrameNum = 0;
    activeLogFile = 0;
    exitCode = EventInit();
    if (exitCode) return ExitCleanup(exitCode);

    if (argc == 3 && strcmp(argv[1], "play") == 0) { // Log playback
        printf("Playing log: %s\n", argv[2]);
        activeLogFile = fopen(argv[2], "rb");
        if (!activeLogFile) {
            printf("Failed to read log: %s\n", argv[2]);
        } else {
            log_playback = true; // Perform log playback.
        }
    } else if (argc == 3 && strcmp(argv[1], "record") == 0) { // Log record
        manualLogName = argv[2];
    }

    EnqueueEvent_Simple(EV_INIT);
    EnqueueEvent_Simple(EV_LOAD_TEXTURES);
    EnqueueEvent_Simple(EV_LOAD_MODELS);
    double accumulator = 0.0;
    last_time = get_time();
    lastJournalWriteTime = get_time();
    while(1) {
        start_frame_time = get_time();
        if (!log_playback) {
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
                        SDL_SetRelativeMouseMode(SDL_TRUE);
                    } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                        window_has_focus = false;
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                    }
                }
            }

            // Enqueue render events in pipeline order
            EnqueueEvent_Simple(EV_CLEAR_FRAME_BUFFERS);
            EnqueueEvent_Simple(EV_RENDER_STATICS);
            EnqueueEvent_Simple(EV_RENDER_UI);
        } else {
            // Log playback controls (pause, fast forward, rewind, quit playback)
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) EnqueueEvent_Simple(EV_QUIT);
                else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        log_playback = false;
                        printf("Exited log playback manually.  Control returned\n");
                    }
                    
                // These aren't really events so just handle them here.
                } else if (event.type == SDL_WINDOWEVENT) {
                    if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                        window_has_focus = true;
                        SDL_SetRelativeMouseMode(SDL_TRUE);
                    } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                        window_has_focus = false;
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                    }
                }
            }
        }
        
        current_time = get_time();
        double frame_time = current_time - last_time;
        accumulator += frame_time;
        while (accumulator >= time_step) {
            if (window_has_focus) ProcessInput();
            accumulator -= time_step;
        }

        // Enqueue all logged events for the current frame.
        if (log_playback) {
            // Read the log file for current frame and enqueue events from log.
            int read_status = ReadActiveLog();
            if (read_status == 2) { // EOF reached, no more events
                printf("Log playback completed.  Control returned.\n");
            } else if (read_status == -1) { // Read error
                printf("Error reading log file, exiting playback\n");
                EnqueueEvent_Simple(EV_QUIT);
            }
        }

        exitCode = EventQueueProcess(); // Do everything
        if (exitCode) break;

        SDL_GL_SwapWindow(window); // Present frame
        last_time = current_time;
        globalFrameNum++;
    }

    // Cleanup
    return ExitCleanup(exitCode);
}
