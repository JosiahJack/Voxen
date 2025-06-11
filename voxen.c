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
#include "quaternion.h"
#include "matrix.h"
#include "constants.h"
#include "shaders.glsl.h"
#include "input.h"
#include "data_textures.h"
#include "data_models.h"

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

// OpenGL
SDL_GLContext gl_context;
GLuint shaderProgram;
GLuint textShaderProgram;
GLuint vao, vbo; // Vertex Array Object and Vertex Buffer Object

TTF_Font *font = NULL;
GLuint textVAO, textVBO;

bool use_bindless_textures = false;

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

// Input states
bool keys[SDL_NUM_SCANCODES] = {0}; // SDL_NUM_SCANCODES 512b, covers all keys
int mouse_x = 0, mouse_y = 0; // Mouse position
bool log_playback = false;

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

int ClearFrameBuffers(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return 0;
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

// int RenderStaticMeshes(void) {
//     glUseProgram(shaderProgram);
//     // Set up matrices
//     float view[16], projection[16];
//     float fov = 65.0f;
//     mat4_perspective(projection, fov, (float)screen_width / screen_height, 0.1f, 100.0f);
//     mat4_lookat(view, cam_x, cam_y, cam_z, &cam_rotation);
//     GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
//     GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
//     glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
//     glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
//     if (use_bindless_textures) {
//         GLint texLoc = glGetUniformLocation(shaderProgram, "uTextures");
//         glUniformHandleui64vARB(texLoc, TEXTURE_COUNT, textureHandles);
//     } else {
//         for (int i = 0; i < TEXTURE_COUNT; i++) {
//             glActiveTexture(GL_TEXTURE0 + i);
//             glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
//             char uniformName[32];
//             snprintf(uniformName, sizeof(uniformName), "uTextures[%d]", i);
//             GLint texLoc = glGetUniformLocation(shaderProgram, uniformName);
//             glUniform1i(texLoc, i);
//         }
//     }
/*
    // Render cube
    glBindVertexArray(vao);
    glDrawArrays(GL_QUADS, 0, 24); // 24 vertices for 6 quads
    glBindVertexArray(0);
    glUseProgram(0);
    return 0;
}*/

int RenderStaticMeshes(void) {
    glUseProgram(shaderProgram);

    // Set up view and projection matrices
    float view[16], projection[16];
    float fov = 65.0f;
    mat4_perspective(projection, fov, (float)screen_width / screen_height, 0.1f, 100.0f);
    mat4_lookat(view, cam_x, cam_y, cam_z, &cam_rotation);
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);

    // Set textures
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

    glBindVertexArray(vao);

    // Render cube (first 24 vertices)
    float model[16];
    mat4_identity(model);
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);
    glDrawArrays(GL_QUADS, 0, 24);

    // Render med1_1.fbx instance at (0, 1.28f, 0)
    mat4_translate(model, 0.0f, 1.28f, 0.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);
    glDrawArrays(GL_TRIANGLES, vbo_offsets[1], modelVertexCounts[0]);

    glBindVertexArray(0);
    glUseProgram(0);
    return 0;
}

int RenderUI(double deltaTime) {
    glDisable(GL_LIGHTING); // Disable lighting for text
    SDL_Color textCol = {255, 255, 255, 255}; // White

    // Draw debug text
    char text0[128];
    snprintf(text0, sizeof(text0), "Frame time: %.6f", deltaTime * 1000.0);
    render_debug_text(10, 10, text0, textCol); // Top-left corner (10, 10)

    char text1[128];
    snprintf(text1, sizeof(text1), "x: %.2f, y: %.2f, z: %.2f", cam_x, cam_y, cam_z);
    render_debug_text(10, 25, text1, textCol); // Top-left corner (10, 10)

    float cam_quat_yaw = 0.0f;
    float cam_quat_pitch = 0.0f;
    float cam_quat_roll = 0.0f;
    quat_to_euler(&cam_rotation,&cam_quat_yaw,&cam_quat_pitch,&cam_quat_roll);
    char text2[128];
    snprintf(text2, sizeof(text2), "cam yaw: %.2f, cam pitch: %.2f, cam roll: %.2f", cam_yaw, cam_pitch, cam_roll);
    render_debug_text(10, 40, text2, textCol);

    char text3[128];
    snprintf(text3, sizeof(text3), "cam quat yaw: %.2f, cam quat pitch: %.2f, cam quat roll: %.2f", cam_quat_yaw, cam_quat_pitch, cam_quat_roll);
    render_debug_text(10, 55, text3, textCol);

    char text4[128];
    snprintf(text4, sizeof(text4), "Peak frame queue count: %.2d", maxEventCount_debug);
    render_debug_text(10, 70, text4, textCol);
    return 0;
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
    if (activeLogFile) fclose(activeLogFile); // Close log playback file.
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
        case EV_LOAD_MODELS: return SetupGeometry();
        case EV_QUIT: return 1; break;
    }

    return 99; // Something went wrong
}

int main(int argc, char* argv[]) {
    if (argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
        printf("-----------------------------------------------------------\n");
        printf("Voxen v0.01.00 6/10/2025\nthe OpenGL Voxel Lit Rendering Engine\n\nby W. Josiah Jack\nMIT licensed\n\n\n");
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
        } else log_playback = true; // Perform log playback.
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
        globalFrameNum++;
    }

    // Cleanup
    return ExitCleanup(exitCode);
}
