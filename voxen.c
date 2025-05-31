// File: cube.c
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

// Window and OpenGL context
Display *display;
Window window;
GLXContext gl_context;
int screen_width = 320, screen_height = 200;

// Camera variables
float cam_x = 0.0f, cam_y = 0.0f, cam_z = 5.0f; // Camera position
float cam_yaw = 0.0f, cam_pitch = 0.0f;         // Camera orientation
float move_speed = 0.005f;

// Input states
bool keys[256] = {false};

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
    glRotatef(cam_yaw, 0, 1, 0);
    glTranslatef(-cam_x, -cam_y, -cam_z);

    // Draw cube
    draw_cube();

    glXSwapBuffers(display, window);
}

// Update camera based on input
void update_camera(void) {
    // WASD movement
    if (keys['w']) cam_z -= move_speed * cos(cam_yaw * M_PI / 180.0f);
    if (keys['s']) cam_z += move_speed * cos(cam_yaw * M_PI / 180.0f);
    if (keys['a']) cam_x -= move_speed * sin(cam_yaw * M_PI / 180.0f);
    if (keys['d']) cam_x += move_speed * sin(cam_yaw * M_PI / 180.0f);
}

int main(void) {
    // Initialize X11
    display = XOpenDisplay(NULL);
    if (!display) { fprintf(stderr, "Cannot open display\n"); exit(1); }
    int s = DefaultScreen(display);

    // Create window
    XSetWindowAttributes attr = {0};
    attr.event_mask = KeyPressMask | KeyReleaseMask;
    window = XCreateWindow(display, RootWindow(display, s), 0, 0, screen_width, screen_height,
                           0, CopyFromParent, InputOutput, CopyFromParent, CWEventMask, &attr);

    // Set up OpenGL context
    int glx_attribs[] = { GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None };
    XVisualInfo *vi = glXChooseVisual(display, s, glx_attribs);
    if (!vi) { fprintf(stderr, "No suitable visual\n"); exit(1); }
    gl_context = glXCreateContext(display, vi, NULL, GL_TRUE);
    glXMakeCurrent(display, window, gl_context);

    // Map window
    XMapWindow(display, window);

    // Set up OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, screen_width, screen_height);
    setup_projection();

    // Main loop
    XEvent event;
    while (1) {
        while (XPending(display)) {
            XNextEvent(display, &event);
            if (event.type == KeyPress) {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                if (key == XK_Escape) goto cleanup;
                keys[key & 0xFF] = true;
            } else if (event.type == KeyRelease) {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                keys[key & 0xFF] = false;
            }
        }
        update_camera();
        render();
    }

cleanup:
    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, gl_context);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
