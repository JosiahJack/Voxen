// File: voxen.c
// Description: A realtime OpenGL based application for experimenting with voxel lighting techniques to derive new methods of high speed accurate lighting in resource constrained environements (e.g. embedded).

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

// Window and OpenGL context
SDL_Window *window;
SDL_GLContext gl_context;
int screen_width = 800, screen_height = 600;
GLuint font_list_base;                          // Display list for font
bool window_has_focus = false;

// Camera variables
float cam_x = 0.0f, cam_y = -4.0f, cam_z = 0.0f; // Camera position
float cam_yaw = 0.0f, cam_pitch = -90.0f;         // Camera orientation
float move_speed = 0.1f;
float mouse_sensitivity = 0.1f;                 // Mouse look sensitivity

// Input states
bool keys[256] = {false};
int mouse_x = 0, mouse_y = 0;                   // Mouse position

const double time_step = 1.0 / 60.0; // 60fps
double last_time = 0.0;

// Function to get current time in seconds
double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
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

void setup_font(void) {
//     Font font = XLoadFont(window, "-misc-fixed-medium-r-normal--13-120-75-75-c-70-iso8859-1");
//     font_list_base = glGenLists(256);
//     glXUseXFont(font, 32, 96, font_list_base + 32); // ASCII 32-127
//     XUnloadFont(window, font);
}

// Renders text at x,y coordinates specified using pointer to the string array.
void render_text(float x, float y, const char *text) {
//     glMatrixMode(GL_PROJECTION);
//     glPushMatrix();
//     glLoadIdentity();
//     glOrtho(0, screen_width, 0, screen_height, -1, 1);
//     glMatrixMode(GL_MODELVIEW);
//     glPushMatrix();
//     glLoadIdentity();
//     glRasterPos2f(x, screen_height - y - 10); // Adjust for 10-pixel height
//     glListBase(font_list_base);
//     glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
//     glMatrixMode(GL_PROJECTION);
//     glPopMatrix();
//     glMatrixMode(GL_MODELVIEW);
//     glPopMatrix();
}

// Function to draw a cube
void draw_cube(void) {
    glBegin(GL_QUADS);
    // Front face
    glColor3f(1, 0, 0);
    glVertex3f(-1, -1, 1); glVertex3f(1, -1, 1);
    glVertex3f(1, 1, 1); glVertex3f(-1, 1, 1);
    // Back face
    glColor3f(0, 1, 0);
    glVertex3f(-1, -1, -1); glVertex3f(-1, 1, -1);
    glVertex3f(1, 1, -1); glVertex3f(1, -1, -1);
    // Top face
    glColor3f(0, 0, 1);
    glVertex3f(-1, 1, -1); glVertex3f(-1, 1, 1);
    glVertex3f(1, 1, 1); glVertex3f(1, 1, -1);
    // Bottom face
    glColor3f(1, 1, 0);
    glVertex3f(-1, -1, -1); glVertex3f(1, -1, -1);
    glVertex3f(1, -1, 1); glVertex3f(-1, -1, 1);
    // Right face
    glColor3f(1, 0, 1);
    glVertex3f(1, -1, -1); glVertex3f(1, 1, -1);
    glVertex3f(1, 1, 1); glVertex3f(1, -1, 1);
    // Left face
    glColor3f(0, 1, 1);
    glVertex3f(-1, -1, -1); glVertex3f(-1, -1, 1);
    glVertex3f(-1, 1, 1); glVertex3f(-1, 1, -1);
    glEnd();
}

// Main rendering function
void render(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set up camera
    glRotatef(cam_pitch, 1, 0, 0);
    glRotatef(cam_yaw, 0, 0, 1);
    glTranslatef(-cam_x, -cam_y, -cam_z);

    // Draw cube
    draw_cube();

    // Draw debug text
    char text[64];
    snprintf(text, sizeof(text), "x: %.2f y: %.2f z: %.2f", cam_x, cam_y, cam_z);
    glColor3f(1, 1, 1); // White text
    render_text(10, 10, text); // Top-left corner (10, 10)
    
    SDL_GL_SwapWindow(window);
}

// Update camera based on input
void process_input(void) {
    // WASD movement
    float yaw_rad = cam_yaw * M_PI / 180.0f;
    float pitch_rad = cam_pitch * M_PI / 180.0f;
    float facing_x = sin(yaw_rad) * cos(pitch_rad); // X component of facing vector
    float facing_y = -cos(yaw_rad) * cos(pitch_rad); // Y component (forward, Z-up)

    // Normalize X-Y facing vector (Z=0 for X-Y plane)
    float len = sqrt(facing_x * facing_x + facing_y * facing_y);
    if (len > 0) { facing_x /= len; facing_y /= len; }
    // Perpendicular vector for strafing (cross product with Z-up)
    float strafe_x = -facing_y; // (facing_x, facing_y, 0) x (0, 0, 1) = (-facing_y, facing_x, 0)
    float strafe_y = facing_x;

    if (keys['w']) {
        cam_x += move_speed * facing_x; // Move forward in X-Y plane
        cam_y += move_speed * facing_y;
    } else if (keys['s']) {
        cam_x -= move_speed * facing_x; // Move backward in X-Y plane
        cam_y -= move_speed * facing_y;
    }
    
    if (keys['a']) {
        cam_x += move_speed * strafe_x; // Strafe left in X-Y plane
        cam_y += move_speed * strafe_y;
    } else if (keys['d']) {
        cam_x -= move_speed * strafe_x; // Strafe right in X-Y plane
        cam_y -= move_speed * strafe_y;
    }
    
    if (keys['v']) {
        cam_z += move_speed;
    } else if (keys['c']) {
        cam_z -= move_speed;
    }
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    window = SDL_CreateWindow("Voxen", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL);
    if (!window) { fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError()); return 1; }
    
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) { fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError()); return 1; }
    
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, screen_width, screen_height);
    setup_projection();
    setup_font();
    double accumulator = 0.0;
    last_time = get_time();
    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) quit = true;
                if (event.key.keysym.sym < 256) keys[event.key.keysym.sym] = true;
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym < 256) keys[event.key.keysym.sym] = false;
            } else if (event.type == SDL_MOUSEMOTION && window_has_focus) {
                cam_yaw += event.motion.xrel * mouse_sensitivity;
                cam_pitch += event.motion.yrel * mouse_sensitivity;
                if (cam_pitch < -179.0f) cam_pitch = -179.0f;
                if (cam_pitch > -1.0f) cam_pitch = -1.0f;
                printf("Mouse delta: x=%d, y=%d\n", event.motion.xrel, event.motion.yrel);
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
            if (window_has_focus) process_input();
            accumulator -= time_step;
        }

        render();
    }
    
    glDeleteLists(font_list_base, 256);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
