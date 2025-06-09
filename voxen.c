// File: voxen.c
// Description: A simple unlit software rasterizer for a cube, using SDL2 for window/input and OpenGL for texture display.

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>

// Window and OpenGL context
SDL_Window *window;
SDL_GLContext gl_context;
int screen_width = 800, screen_height = 600;
TTF_Font *font = NULL;
bool window_has_focus = false;
SDL_Color textCol = {255, 255, 255, 255};

// Camera variables
float cam_x = 0.0f, cam_y = -4.0f, cam_z = 0.0f; // Camera position
float cam_yaw = 0.0f, cam_pitch = -90.0f;         // Camera orientation
float camdir_x, camdir_y, camdir_z;
float move_speed = 0.1f;
float normPointX = 0.0f;
float normPointY = -1.0f;
float normPointZ = 0.0f;
float mouse_sensitivity = 0.1f;                   // Mouse look sensitivity
float CAMERA_NEAR;
float CAMERA_FAR;
float CAMERA_FOV_HORIZONTAL;
float CAMERA_FOV_VERTICAL;
float DEGREES_PER_PIXEL_X;
float DEGREES_PER_PIXEL_Y;
float CAMERA_FOV_HORIZONTAL_HALF;
float CAMERA_FOV_VERTICAL_HALF;

float testVertX = 32.0f;
float testVertY = 128.0f;
float testVertZ = 3.0f;

// Input states
bool keys[256] = {false};
int mouse_x = 0, mouse_y = 0;                   // Mouse position

const double time_step = 1.0 / 60.0; // 60fps
double last_time = 0.0;

// Software rasterizer buffers
uint32_t *framebuffer = NULL; // RGBA pixel buffer
float *depth_buffer = NULL;   // Depth buffer

#define RAD2DEG (180.0 / M_PI)
#define DEG2RAD (M_PI / 180.0)

double rad2deg(double radians) {
    return radians * RAD2DEG;
}

double deg2rad(double degrees) {
    return degrees * DEG2RAD;
}

// TODO: Define the colors for splats
// TODO: Use individual arrays (Structure of Arrays pattern) for each element that comprises a splat: x, y, z, radius, color_r, color_g, color_b, norm_x, norm_y, norm_z

// TODO: Define a splat definition file .splat and put the splat data in there as readable ASCII 8bit text and load that at runtime in main() prior to the main loop.

// x, y, z, radius
float splats[5][4] = {
    {0, 0, 0, 20}, {1, 0, 0, 10}, {1, 1, 0, 10}, {0, 1, 0, 20}, {0.5, 0.5, 0.5, 30}
};

// Function to get current time in seconds
double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void get_cam_dir(float cam_yaw, float cam_pitch, float *dir_x, float *dir_y, float *dir_z) {
    // Convert to radians
    float yaw_rad = deg2rad(cam_yaw);
    float pitch_rad = deg2rad(cam_pitch);

    // Compute direction vector (Z-up coordinate system)
    float cosfrad = cosf(pitch_rad);
    *dir_x = sinf(yaw_rad) * cosfrad;
    *dir_y = -cosf(yaw_rad) * cosfrad;
    *dir_z = sinf(pitch_rad);

    // Normalize the result
    float magnitude = sqrtf((*dir_x) * (*dir_x) + (*dir_y) * (*dir_y) + (*dir_z) * (*dir_z));
    if (magnitude > 0.0f) {
        *dir_x /= magnitude;
        *dir_y /= magnitude;
        *dir_z /= magnitude;
    }
}

void normalize(float *x, float *y, float *z) {
    float magnitude = sqrtf((*x) * (*x) + (*y) * (*y) + (*z) * (*z));
    if (magnitude > 0.0f) {
        *x /= magnitude;
        *y /= magnitude;
        *z /= magnitude;
    }
}

float dot(float x0, float y0, float z0, float x1, float y1, float z1) {
    return (x0 * x1) + (y0 * y1) + (z0 * z1);
}

// Software render function
void software_render(void) {
    // Clear buffers
    memset(framebuffer, 0, screen_width * screen_height * sizeof(uint32_t));
    for (int i = 0; i < screen_width * screen_height; i++) depth_buffer[i] = 1e10;

    // Generate list of "spots" which are splats projected into 2d screen x,y
    float spots[5][2] = {{0, 0}, {0, 0}};

    // Temporarily force some sort of projection to test splat rendering
    spots[0][0] = 400;
    spots[0][1] = 400;
    spots[1][0] = 800;
    spots[1][1] = 400;
    spots[2][0] = 420;
    spots[2][1] = 406;
    spots[3][0] = 780;
    spots[3][1] = 406;
    spots[4][0] = 480;
    spots[4][1] = 480;

    // Apply spherical coordinate projection (not standard perspective projection via matrix4x4's, treats x,y as yaw,pitch respectively with equiangular spacing between pixels).
    for (int s = 0; s<5;s++) {
        // TODO: Actually do the spherical projection (e.g. atan2() for the yaw relative to cam_x,cam_y then subtract out the camera's min yaw)
//         spots[s][0] = 400;
//         spots[s][1] = 400;
    }

    float dist = 0;
    float dx = 0;
    float dy = 0;
    float rad = 0;
    float r = 0.7f;
    float g = 0.0f;
    float b = 0.2f;
    uint32_t packedCol;

    // TODO: Implement tiled rendering with each tile on a separate pthread.
    // Iterate over all pixels in the screen
    for (int y = 0; y<screen_height - 1; y++) {
        for (int x = 0; x< screen_width - 1; x++) {
            int idx = (screen_height - 1 - y) * screen_width + x; // Index into 1D framebuffer array. (or for depth buffer later.)
            // Iterate over every splat (TODO: Cull based on bounds in a preprocessing loop prior to the "Iterate over all pixels in the screen" loops.
            for (int s = 0; s < 5; s++) {
                // Very basic spherical splat rendering
                rad = splats[s][3]; // Get radius for the current splat.
                rad *= rad;
                // TODO: Early bounds check using manhattan distance.
                dx = (spots[s][0] - x);
                dy = (spots[s][1] - y);
                dist = (dx * dx) + (dy * dy); // Just squared distance for the check for performance.
                // TODO: Adjust the brightness based on lambertion cos() angle between splat normal and the camdir_x, camdir_y, camdir_z facing vector.  This is the "backface" culling.
                // TODO: Early return if splat normal facing away via dot()?
                uint32_t r8 = (uint32_t)(r * ((rad - dist) / rad) * 255); // Linear falloff for color based on distance from splat's center
                uint32_t g8 = (uint32_t)(g * ((rad - dist) / rad) * 255);
                uint32_t b8 = (uint32_t)(b * ((rad - dist) / rad) * 255);
                if (dist < rad) {
                    packedCol = (0xFF << 24) | (b8 << 16) | (g8 << 8) | r8; // Stupid blending for now, just to not have hard edges. TODO: Use alpha channel with luminance to not overbrighten existing colors
                    packedCol += framebuffer[idx];
                    framebuffer[idx] = packedCol;
                }
            }
        }
    }
}

// Render debug text (unchanged from original)
void render_debug_text(float x, float y, const char *text, SDL_Color color) {
    if (!font || !text) { fprintf(stderr, "Font or text is NULL\n"); return; }
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
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(x, screen_height - y - rgba_surface->h);
    glTexCoord2f(1, 1); glVertex2f(x + rgba_surface->w, screen_height - y - rgba_surface->h);
    glTexCoord2f(1, 0); glVertex2f(x + rgba_surface->w, screen_height - y);
    glTexCoord2f(0, 0); glVertex2f(x, screen_height - y);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(rgba_surface);
}

// Main rendering function
void render(void) {
    uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t start, end;
    double render_time = 0;
    start = SDL_GetPerformanceCounter();
    // TODO: Calculate the globals for min yaw, max yaw, min pitch, max pitch of the camera using cam_yaw and cam_pitch prior to software_render so it can perform projection correctly with the mouselook.

    // Perform software rendering
    software_render();

    // Upload framebuffer to OpenGL texture
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, framebuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Render texture to screen
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screen_width, 0, screen_height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(screen_width, 0);
    glTexCoord2f(1, 1); glVertex2f(screen_width, screen_height);
    glTexCoord2f(0, 1); glVertex2f(0, screen_height);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDeleteTextures(1, &texture);

    // Draw debug text
    char text[64];
    snprintf(text, sizeof(text), "x: %.2f y: %.2f z: %.2f", cam_x, cam_y, cam_z);
    render_debug_text(10, 25, text, textCol);

    char text2[64];
    snprintf(text2, sizeof(text2), "x deg: %.2f y deg: %.2f", cam_yaw, cam_pitch);
    render_debug_text(10, 40, text2, textCol);

    char text3[64];
    snprintf(text3, sizeof(text3), "testVertX: %.2f testVertY: %.2f", testVertX, testVertY);
    render_debug_text(10, 55, text3, textCol);

    char text4[64];
    snprintf(text4, sizeof(text4), "testVertZ: %.2f", testVertZ);
    render_debug_text(10, 70, text4, textCol);

    end = SDL_GetPerformanceCounter();
    render_time = (end - start) * 1000.0 / (double)freq;

    char text5[64];
    snprintf(text5, sizeof(text5), "frame time: %.4f", render_time);
    render_debug_text(10, 10, text5, textCol);

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
//         cam_x -= move_speed * facing_x; // Move forward (inverted)
//         cam_y -= move_speed * facing_y;
        testVertY += move_speed * 2.0f;
    } else if (keys['s']) {
//         cam_x += move_speed * facing_x; // Move backward
//         cam_y += move_speed * facing_y;
        testVertY -= move_speed * 2.0f;
    }
    if (keys['a']) {
//         cam_x -= move_speed * strafe_x; // Strafe left
//         cam_y -= move_speed * strafe_y;
        testVertX -= move_speed * 2.0f;
    } else if (keys['d']) {
//         cam_x += move_speed * strafe_x; // Strafe right
//         cam_y += move_speed * strafe_y;
        testVertX += move_speed * 2.0f;
    }
    if (keys['v']) {
//         cam_z += move_speed;
//         normPointY += move_speed * 2.0f;
        testVertZ += move_speed * 2.0f;
    } else if (keys['c']) {
//         cam_z -= move_speed;
//         normPointY -= move_speed * 2.0f;
        testVertZ -= move_speed * 2.0f;
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
    window = SDL_CreateWindow("Voxen Software Rasterizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, screen_width, screen_height);

    // Allocate buffers
    framebuffer = (uint32_t *)malloc(screen_width * screen_height * sizeof(uint32_t));
    depth_buffer = (float *)malloc(screen_width * screen_height * sizeof(float));
    if (!framebuffer || !depth_buffer) {
        fprintf(stderr, "Failed to allocate buffers\n");
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 10);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
    }

    double accumulator = 0.0;
    last_time = get_time();
    bool quit = false;

    CAMERA_NEAR = 0.02f;
    CAMERA_FAR = 20.0f;
    CAMERA_FOV_HORIZONTAL = 90.0f;
    CAMERA_FOV_VERTICAL = 60.0f;
    DEGREES_PER_PIXEL_X = CAMERA_FOV_HORIZONTAL / screen_width; // 90 / 800 = 0.1125 degrees
    DEGREES_PER_PIXEL_Y = CAMERA_FOV_VERTICAL / screen_height;  // 90 / 600 = 0.1000 degrees
    CAMERA_FOV_HORIZONTAL_HALF = CAMERA_FOV_HORIZONTAL / 2.0f;
    CAMERA_FOV_VERTICAL_HALF = CAMERA_FOV_VERTICAL / 2.0f;
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
    free(framebuffer);
    free(depth_buffer);
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
