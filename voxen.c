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

// Cube definition: 8 vertices (x, y, z) and per-face colors (r, g, b)
float cube_vertices[8][3] = {
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}, // Front face
    {-1, -1, -1}, {-1,  1, -1}, { 1,  1, -1}, { 1, -1, -1}  // Back face
};
float face_colors[12][3] = {
    {1, 0, 0}, // Front: red
    {1, 0, 0}, // Front: red
    {0, 1, 0}, // Back: green
    {0, 0, 1}, // Top: blue
    {1, 1, 0}, // Bottom: yellow
    {1, 0, 1}, // Right: magenta
    {0, 1, 1}, // Left: cyan
    {0.5, 0, 0}, // Left: cyan
    {0, 0.5, 0}, // Left: cyan
    {0, 0, 0.5}, // Left: cyan
    {0.5, 0.5, 0}, // Left: cyan
    {0.5, 0, 0.5}, // Left: cyan
};
int cube_triangles[12][3] = {
    // Front: 0,1,2 and 0,2,3
    {0, 1, 2}, {0, 2, 3},
    // Back: 4,5,6 and 4,6,7
    {4, 5, 6}, {4, 6, 7},
    // Top: 3,2,6 and 3,6,5
    {3, 2, 6}, {3, 6, 5},
    // Bottom: 4,7,1 and 4,1,0
    {4, 7, 1}, {4, 1, 0},
    // Right: 7,6,2 and 7,2,1
    {7, 6, 2}, {7, 2, 1},
    // Left: 4,0,3 and 4,3,5
    {4, 0, 3}, {4, 3, 5}
};

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

// Barycentric coordinate check for triangle rasterization
bool is_inside_triangle(float x, float y, float v0x, float v0y, float v1x, float v1y, float v2x, float v2y, float *u, float *v, float *w) {
    float area = 0.5f * (-v1y * v2x + v0y * (-v1x + v2x) + v0x * (v1y - v2y) + v1x * v2y);
    float s = (v0y * v2x - v0x * v2y + (v2y - v0y) * x + (v0x - v2x) * y) / (2 * area);
    float t = (v0x * v1y - v0y * v1x + (v0y - v1y) * x + (v1x - v0x) * y) / (2 * area);
    *u = s; *v = t; *w = 1.0f - s - t;
    return *u >= 0 && *v >= 0 && *w >= 0;
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

// Rasterize a triangle
void rasterize_triangle(float x0, float pitch0, float dist0,
                        float x1, float pitch1, float dist1,
                        float x2, float pitch2, float dist2,
                        float r, float g, float b) {
    float y0, y1, y2;
    y0 = screen_height - pitch0;
    y1 = screen_height - pitch1;
    y2 = screen_height - pitch2;
    uint32_t r8 = (uint32_t)(r * 255);
    uint32_t g8 = (uint32_t)(g * 255);
    uint32_t b8 = (uint32_t)(b * 255);
    uint32_t r8_c = (uint32_t)(r * 0.7 * 255);
    uint32_t g8_c = (uint32_t)(g * 0.7 * 255);
    uint32_t b8_c = (uint32_t)(b * 0.7 * 255);
    uint32_t r8_f = (uint32_t)(r * 0.4 * 255);
    uint32_t g8_f = (uint32_t)(g * 0.4 * 255);
    uint32_t b8_f = (uint32_t)(b * 0.4 * 255);

    // Bounding box
    int min_x = (int)fmax(0, fmin(x0, fmin(x1,x2)));
    int max_x = (int)fmin(screen_width - 1, fmax(x0, fmax(x1,x2)));
    int min_y = (int)fmax(0, fmin(y0, fmin(y1,y2)));
    int max_y = (int)fmin(screen_height - 1, fmax(y0, fmax(y1,y2)));
    for (int y = min_y; y <= max_y; y+=4) {
        for (int x = min_x; x <= max_x; x+=4) {
            float u, v, w;
            if (is_inside_triangle(x + 0.5f, y + 0.5f,
                                   x0, y0,
                                   x1, y1,
                                   x2, y2,
                                   &u, &v, &w)) {

                // Interpolate depth
                float depth = u * dist0 + v * dist1 + w * dist2;
                int idx = (screen_height - 1 - y) * screen_width + x;
                if (depth > 0 && depth < depth_buffer[idx]) {
                    depth_buffer[idx] = depth;
                    // Convert color to 8-bit and pack into RGBA

                    int idx_up = idx - screen_width;
                    int idx_dn = idx + screen_width;
                    int idx_rt = idx + 1;
                    int idx_lf = idx - 1;
                    int idx_nw = idx - screen_width - 1;
                    int idx_ne = idx - screen_width + 1;
                    int idx_sw = idx + screen_width - 1;
                    int idx_se = idx + screen_width + 1;
                    framebuffer[idx] = (0xFF << 24) | (b8 << 16) | (g8 << 8) | r8;
                    framebuffer[idx_up] = (0xFF << 24) | (b8_c << 16) | (g8_c << 8) | r8_c;
                    framebuffer[idx_dn] = (0xFF << 24) | (b8_c << 16) | (g8_c << 8) | r8_c;
                    framebuffer[idx_lf] = (0xFF << 24) | (b8_c << 16) | (g8_c << 8) | r8_c;
                    framebuffer[idx_rt] = (0xFF << 24) | (b8_c << 16) | (g8_c << 8) | r8_c;
                    framebuffer[idx_nw] = (0xFF << 24) | (b8_f << 16) | (g8_f << 8) | r8_f;
                    framebuffer[idx_ne] = (0xFF << 24) | (b8_f << 16) | (g8_f << 8) | r8_f;
                    framebuffer[idx_sw] = (0xFF << 24) | (b8_f << 16) | (g8_f << 8) | r8_f;
                    framebuffer[idx_se] = (0xFF << 24) | (b8_f << 16) | (g8_f << 8) | r8_f;
                }
            }
        }
    }
}

// Software render function
void software_render(void) {
    // Clear buffers
    memset(framebuffer, 0, screen_width * screen_height * sizeof(uint32_t));
    for (int i = 0; i < screen_width * screen_height; i++) depth_buffer[i] = 1e10;

    // Generate list of "spots" which are splats projected into 2d screen x,y
    float spots[5][2] = {{0, 0}, {0, 0}};
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
    for (int s = 0; s<5;s++) {
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
    for (int y = 0; y<screen_height - 1; y++) {
        for (int x = 0; x< screen_width - 1; x++) {
            int idx = (screen_height - 1 - y) * screen_width + x;
            for (int s = 0; s < 5; s++) {
                rad = splats[s][3];
                rad *= rad;
                dx = (spots[s][0] - x);
                dy = (spots[s][1] - y);
                dist = (dx * dx) + (dy * dy);
                uint32_t r8 = (uint32_t)(r * ((rad - dist) / rad) * 255);
                uint32_t g8 = (uint32_t)(g * ((rad - dist) / rad) * 255);
                uint32_t b8 = (uint32_t)(b * ((rad - dist) / rad) * 255);
                if (dist < rad) {
                    packedCol = (0xFF << 24) | (b8 << 16) | (g8 << 8) | r8;
                    packedCol += framebuffer[idx];
                    framebuffer[idx] = packedCol;
                }
            }
        }
    }
    /*
    float normX = normPointX;
    float normY = normPointY;
    float normZ = normPointZ;
    normalize(&normX, &normY, &normZ);
    get_cam_dir(cam_yaw, cam_pitch, &camdir_x, &camdir_y, &camdir_z);
    if (dot(camdir_x, camdir_y, camdir_z, normX, normY, normZ) < 0) rasterize_triangle(testVertX, testVertY, testVertZ  , 128, 128, 3  , 128, 160, 3  , 0.7, 0.0, 0.0);
    rasterize_triangle(62, 128, 3.1, 158, 128, 3.1, 158, 160, 3.1, 0.0, 0.0, 0.7);
    return;

    float cam_yaw_min = cam_yaw - CAMERA_FOV_HORIZONTAL_HALF;
    float cam_yaw_max= cam_yaw + CAMERA_FOV_HORIZONTAL_HALF;
    float cam_pitch_min = cam_pitch - CAMERA_FOV_VERTICAL_HALF;
    float cam_pitch_max = cam_pitch + CAMERA_FOV_VERTICAL_HALF;

    // Process each triangle
    for (int i = 0; i < 12; i++) {
        int idx0 = cube_triangles[i][0];
        int idx1 = cube_triangles[i][1];
        int idx2 = cube_triangles[i][2];
        float v0[3] = {cube_vertices[idx0][0], cube_vertices[idx0][1], cube_vertices[idx0][2]};
        float v1[3] = {cube_vertices[idx1][0], cube_vertices[idx1][1], cube_vertices[idx1][2]};
        float v2[3] = {cube_vertices[idx2][0], cube_vertices[idx2][1], cube_vertices[idx2][2]};

        // Transform to view space in polar coordinates where x maps to yaw, y maps to pitch, and z is the distance to camera.
        float dx = v0[0] - cam_x;
        float dy = v0[1] - cam_y;
        float dz = v0[2] - cam_z;
        float dist_v0 = sqrt((dx * dx) + (dy * dy) + (dz * dz));
        float yaw_v0 = rad2deg(atan2f(dy, dx)) - cam_yaw;
        while (yaw_v0 > 180.0f) yaw_v0 -= 360.0f;
        while (yaw_v0 < -180.0f) yaw_v0 += 360.0f;
        yaw_v0 = (yaw_v0 / DEGREES_PER_PIXEL_X) + (screen_width / 2.0f);
        float dist_xy_v0 = sqrt(dx * dx + dy * dy);
        float pitch_v0 = dist_xy_v0 < FLT_EPSILON ? 0 : rad2deg(atan2f(dz, dist_xy_v0));
        pitch_v0 = (pitch_v0 - cam_pitch) / DEGREES_PER_PIXEL_Y + (screen_height / 2.0f);

        dx = v1[0] - cam_x;
        dy = v1[1] - cam_y;
        dz = v1[2] - cam_z;
        float dist_v1 = sqrt((dx * dx) + (dy * dy) + (dz * dz));
        float yaw_v1 = rad2deg(atan2f(dy, dx)) - cam_yaw;
        while (yaw_v1 > 180.0f) yaw_v1 -= 360.0f;
        while (yaw_v1 < -180.0f) yaw_v1 += 360.0f;
        yaw_v1 = (yaw_v1 / DEGREES_PER_PIXEL_X) + (screen_width / 2.0f);
        float dist_xy_v1 = sqrt(dx * dx + dy * dy);
        float pitch_v1 = dist_xy_v1 < FLT_EPSILON ? 0 : rad2deg(atan2f(dz, dist_xy_v1));
        pitch_v1 = (pitch_v1 - cam_pitch) / DEGREES_PER_PIXEL_Y + (screen_height / 2.0f);

        dx = v2[0] - cam_x;
        dy = v2[1] - cam_y;
        dz = v2[2] - cam_z;
        float dist_v2 = sqrt((dx * dx) + (dy * dy) + (dz * dz));
        float yaw_v2 = rad2deg(atan2f(dy, dx)) - cam_yaw;
        while (yaw_v2 > 180.0f) yaw_v2 -= 360.0f;
        while (yaw_v2 < -180.0f) yaw_v2 += 360.0f;
        yaw_v2 = (yaw_v2 / DEGREES_PER_PIXEL_X) + (screen_width / 2.0f);
        float dist_xy_v2 = sqrt(dx * dx + dy * dy);
        float pitch_v2 = dist_xy_v2 < FLT_EPSILON ? 0 : rad2deg(atan2f(dz, dist_xy_v2));
        pitch_v2 = (pitch_v2 - cam_pitch) / DEGREES_PER_PIXEL_Y + (screen_height / 2.0f);

        // Rasterize
        float r = face_colors[i][0];
        float g = face_colors[i][1];
        float b = face_colors[i][2];
        rasterize_triangle(yaw_v0, pitch_v0, dist_v0, yaw_v1, pitch_v1, dist_v1, yaw_v2, pitch_v2, dist_v2, r, g, b);
    }
    */
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
