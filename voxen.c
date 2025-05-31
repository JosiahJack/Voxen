// File: voxen.c
// Description: A realtime OpenGL based application for experimenting with voxel lighting techniques to derive new methods of high speed accurate lighting in resource constrained environements (e.g. embedded).

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
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
TTF_Font *font = NULL;
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

// Main rendering function
void render(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set up camera
    glRotatef(cam_pitch, 1, 0, 0);
    glRotatef(cam_yaw, 0, 0, 1);
    glTranslatef(-cam_x, -cam_y, -cam_z);
    
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

    // Draw cube
    draw_cube();

    glDisable(GL_LIGHTING); // Disable lighting for text
    
    // Draw debug text
    char text[64];
    snprintf(text, sizeof(text), "x: %.2f y: %.2f z: %.2f", cam_x, cam_y, cam_z);
    SDL_Color textCol = {255, 255, 255, 255}; // White
    render_debug_text(10, 10, text, textCol); // Top-left corner (10, 10)
    
    SDL_GL_SwapWindow(window);
}

// Update camera based on input
void process_input(void) {
    // WASD movement (fixed to match inverted mouselook)
    float yaw_rad = -cam_yaw * M_PI / 180.0f;
    float pitch_rad = cam_pitch * M_PI / 180.0f;
    // Forward direction (Z-up, adjusted for yaw and pitch)
    float facing_x = sin(yaw_rad);
    float facing_y = -cos(yaw_rad);
    // Normalize
    float len = sqrt(facing_x * facing_x + facing_y * facing_y);
    if (len > 0) { facing_x /= len; facing_y /= len; }
    // Strafe direction (perpendicular)
    float strafe_x = -facing_y;
    float strafe_y = facing_x;

    if (keys['f']) {
        cam_x -= move_speed * facing_x; // Move forward (inverted)
        cam_y -= move_speed * facing_y;
    } else if (keys['s']) {
        cam_x += move_speed * facing_x; // Move backward
        cam_y += move_speed * facing_y;
    }
    if (keys['a']) {
        cam_x -= move_speed * strafe_x; // Strafe left
        cam_y -= move_speed * strafe_y;
    } else if (keys['d']) {
        cam_x += move_speed * strafe_x; // Strafe right
        cam_y += move_speed * strafe_y;
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
    
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    window = SDL_CreateWindow("Voxen, the OpenGL Voxel Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }
    
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        exit(1);
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
    
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 10);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        // Continue without font, handled in render_debug_text()
    }

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
    
    // Cleanup
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
