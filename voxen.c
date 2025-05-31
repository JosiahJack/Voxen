// File: voxen.c
// Description: A realtime OpenGL based application for experimenting with voxel lighting techniques to derive new methods of high speed accurate lighting in resource constrained environements (e.g. embedded).

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <X11/extensions/XInput2.h>

// Window and OpenGL context
Display *display;
Window window;
GLXContext gl_context;
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
    Font font = XLoadFont(display, "-misc-fixed-medium-r-normal--13-120-75-75-c-70-iso8859-1");
    font_list_base = glGenLists(256);
    glXUseXFont(font, 32, 96, font_list_base + 32); // ASCII 32-127
    XUnloadFont(display, font);
}

// Renders text at x,y coordinates specified using pointer to the string array.
void render_text(float x, float y, const char *text) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screen_width, 0, screen_height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2f(x, screen_height - y - 10); // Adjust for 10-pixel height
    glListBase(font_list_base);
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
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
    
    glXSwapBuffers(display, window);
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
    // Initialize X11
    display = XOpenDisplay(NULL);
    if (!display) { fprintf(stderr, "Cannot open display\n"); exit(1); }
    int s = DefaultScreen(display);

    // Create window
    XSetWindowAttributes attr = {0};
    attr.event_mask = KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | FocusChangeMask;
    window = XCreateWindow(display, RootWindow(display, s), 0, 0, screen_width, screen_height,
                           0, CopyFromParent, InputOutput, CopyFromParent, CWEventMask, &attr);

    // Set up OpenGL context
    int glx_attribs[] = { GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None };
    XVisualInfo *vi = glXChooseVisual(display, s, glx_attribs);
    if (!vi) { fprintf(stderr, "No suitable visual\n"); exit(1); }
    gl_context = glXCreateContext(display, vi, NULL, GL_TRUE);
    glXMakeCurrent(display, window, gl_context);

    // Map window and center mouse
    XMapWindow(display, window);
    mouse_x = screen_width / 2;
    mouse_y = screen_height / 2;

    // Set up OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, screen_width, screen_height);
    setup_projection();
    setup_font();
    
    window_has_focus = true;

    // Main loop with fixed time step
    double accumulator = 0.0;
    last_time = get_time();
    
    XEvent event;
    
    int xi_opcode, xi_event, xi_error;
    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &xi_event, &xi_error)) {
        fprintf(stderr, "XInput2 not available\n"); exit(1);
    }
    
    XIEventMask eventmask;
    unsigned char mask[2] = {0, 0};
    XISetMask(mask, XI_RawMotion);
    eventmask.deviceid = XIAllMasterDevices;
    eventmask.mask_len = sizeof(mask);
    eventmask.mask = mask;
    XISelectEvents(display, DefaultRootWindow(display), &eventmask, 1);
    while (1) {
        // Handle events
        double raw_dx = 0.0, raw_dy = 0.0;
        bool has_motion = false;
        while (XPending(display)) {
            XNextEvent(display, &event);
            if (event.type == GenericEvent && event.xcookie.extension == xi_opcode && XGetEventData(display, &event.xcookie)) {
                printf("event.type was GenericEvent\n");
                if (event.xcookie.evtype == XI_RawMotion) {
                    printf("XI_RawMotion event received via xcookie\n");
                    XIRawEvent *raw = (XIRawEvent *)event.xcookie.data;
                    if (raw->valuators.mask_len) {
                        double *values = raw->raw_values;
                        if (XIMaskIsSet(raw->valuators.mask, 0)) {
                            printf("raw_dx changed\n");
                            raw_dx += *values; // Accumulate X delta
                            values++;
                        }
                        if (XIMaskIsSet(raw->valuators.mask, 1)) {
                            printf("raw_dy changed\n");
                            raw_dy += *values; // Accumulate Y delta
                        }
                    }
                    has_motion = true;
                }
                XFreeEventData(display, &event.xcookie);
            }

            if (event.type == KeyPress) {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                if (key == XK_Escape) goto cleanup;
                keys[key & 0xFF] = true;
            } else if (event.type == KeyRelease) {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                keys[key & 0xFF] = false;
            } else if (event.type == FocusIn) {
                window_has_focus = true;
            } else if (event.type == FocusOut) {
                window_has_focus = false;
            }
        }
        
        if (has_motion && window_has_focus) {
            cam_yaw -= raw_dx * mouse_sensitivity;
            cam_pitch -= raw_dy * mouse_sensitivity;
            if (cam_pitch > 89.0f) cam_pitch = 89.0f;
            if (cam_pitch < -89.0f) cam_pitch = -89.0f;
        }

        // Fixed time step update
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

cleanup:
    glDeleteLists(font_list_base, 256);
    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, gl_context);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
