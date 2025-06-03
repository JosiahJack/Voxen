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

// Camera variables
float cam_x = 0.0f, cam_y = -4.0f, cam_z = 0.0f; // Camera position
float cam_yaw = 0.0f, cam_pitch = -90.0f;         // Camera orientation
float move_speed = 0.1f;
float mouse_sensitivity = 0.1f;                   // Mouse look sensitivity

// Input states
bool keys[256] = {false};
int mouse_x = 0, mouse_y = 0;                   // Mouse position

const double time_step = 1.0 / 60.0; // 60fps
double last_time = 0.0;

// Software rasterizer buffers
uint32_t *framebuffer = NULL; // RGBA pixel buffer
float *depth_buffer = NULL;   // Depth buffer

// Cube definition: 8 vertices (x, y, z) and per-face colors (r, g, b)
float cube_vertices[8][3] = {
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}, // Front face
    {-1, -1, -1}, {-1,  1, -1}, { 1,  1, -1}, { 1, -1, -1}  // Back face
};
float face_colors[12][3] = {
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
    {0, 0.5, 0.5}  // Left: cyan
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

// Function to get current time in seconds
double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// Vector math helpers
typedef struct { float x, y, z; } Vec3;
typedef struct { float x, y, w; } Vec2;

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float vec3_length(Vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 vec3_normalize(Vec3 v) {
    float len = vec3_length(v);
    if (len > 0) return (Vec3){v.x / len, v.y / len, v.z / len};
    return v;
}

void matrix_multiply(float *m, float *v, float *result) {
    result[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12] * v[3];
    result[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13] * v[3];
    result[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
    result[3] = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
}

// Setup projection and view matrices
void setup_matrices(float *proj, float *view) {
    // Projection matrix (same as original)
    float aspect = (float)screen_width / screen_height;
    float fov = 90.0f, near = 0.1f, far = 100.0f;
    float f = 1.0f / tan(fov * M_PI / 360.0f);
    proj[0] = f / aspect; proj[5] = f; proj[10] = (far + near) / (near - far);
    proj[11] = -1; proj[14] = (2 * far * near) / (near - far);
    proj[1] = proj[2] = proj[3] = proj[4] = proj[6] = proj[7] = proj[8] = proj[9] = proj[12] = proj[13] = proj[15] = 0;

    // View matrix (camera transformation)
    float yaw_rad = -cam_yaw * M_PI / 180.0f;
    float pitch_rad = cam_pitch * M_PI / 180.0f;
    Vec3 forward = {sin(yaw_rad), -cos(yaw_rad), sin(pitch_rad)};
    Vec3 up = {0, 0, 1};
    Vec3 right = vec3_cross(forward, up);
    up = vec3_cross(right, forward);
    forward = vec3_normalize(forward);
    right = vec3_normalize(right);
    up = vec3_normalize(up);
    view[0] = right.x; view[4] = right.y; view[8] = right.z; view[12] = -vec3_dot(right, (Vec3){cam_x, cam_y, cam_z});
    view[1] = up.x; view[5] = up.y; view[9] = up.z; view[13] = -vec3_dot(up, (Vec3){cam_x, cam_y, cam_z});
    view[2] = -forward.x; view[6] = -forward.y; view[10] = -forward.z; view[14] = vec3_dot(forward, (Vec3){cam_x, cam_y, cam_z});
    view[3] = view[7] = view[11] = 0; view[15] = 1;
}

// Barycentric coordinate check for triangle rasterization
bool is_inside_triangle(float x, float y, Vec2 v0, Vec2 v1, Vec2 v2, float *u, float *v, float *w) {
    float area = 0.5f * (-v1.y * v2.x + v0.y * (-v1.x + v2.x) + v0.x * (v1.y - v2.y) + v1.x * v2.y);
    float s = (v0.y * v2.x - v0.x * v2.y + (v2.y - v0.y) * x + (v0.x - v2.x) * y) / (2 * area);
    float t = (v0.x * v1.y - v0.y * v1.x + (v0.y - v1.y) * x + (v1.x - v0.x) * y) / (2 * area);
    *u = s; *v = t; *w = 1.0f - s - t;
    return *u >= 0 && *v >= 0 && *w >= 0;
}

// Rasterize a triangle
void rasterize_triangle(Vec2 v0, Vec2 v1, Vec2 v2, float z0, float z1, float z2, float r, float g, float b) {
    // Bounding box
    int min_x = (int)fmax(0, fmin(v0.x, fmin(v1.x, v2.x)));
    int max_x = (int)fmin(screen_width - 1, fmax(v0.x, fmax(v1.x, v2.x)));
    int min_y = (int)fmax(0, fmin(v0.y, fmin(v1.y, v2.y)));
    int max_y = (int)fmin(screen_height - 1, fmax(v0.y, fmax(v1.y, v2.y)));

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            float u, v, w;
            if (is_inside_triangle(x + 0.5f, y + 0.5f, v0, v1, v2, &u, &v, &w)) {
                // Interpolate depth
                float depth = u * z0 + v * z1 + w * z2;
                int idx = (screen_height - 1 - y) * screen_width + x;
                if (depth > 0 && depth < depth_buffer[idx]) {
                    depth_buffer[idx] = depth;
                    // Convert color to 8-bit and pack into RGBA
                    uint8_t r8 = (uint8_t)(r * 255);
                    uint8_t g8 = (uint8_t)(g * 255);
                    uint8_t b8 = (uint8_t)(b * 255);
                    framebuffer[idx] = (r8 << 24) | (g8 << 16) | (b8 << 8) | 0xFF;
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

    // Setup matrices
    float proj[16] = {0}, view[16] = {0};
    setup_matrices(proj, view);

    // Process each triangle
    for (int i = 0; i < 12; i++) {
        // Get vertices
        int idx0 = cube_triangles[i][0], idx1 = cube_triangles[i][1], idx2 = cube_triangles[i][2];
        float v0[4] = {cube_vertices[idx0][0], cube_vertices[idx0][1], cube_vertices[idx0][2], 1};
        float v1[4] = {cube_vertices[idx1][0], cube_vertices[idx1][1], cube_vertices[idx1][2], 1};
        float v2[4] = {cube_vertices[idx2][0], cube_vertices[idx2][1], cube_vertices[idx2][2], 1};

        // Transform to view space
        float v0_view[4], v1_view[4], v2_view[4];
        matrix_multiply(view, v0, v0_view);
        matrix_multiply(view, v1, v1_view);
        matrix_multiply(view, v2, v2_view);

        // Project to screen space
        float v0_proj[4], v1_proj[4], v2_proj[4];
        matrix_multiply(proj, v0_view, v0_proj);
        matrix_multiply(proj, v1_view, v1_proj);
        matrix_multiply(proj, v2_view, v2_proj);

        // Perspective divide and viewport transform
        Vec2 s0, s1, s2;
        float z0, z1, z2;
        if (v0_proj[3] != 0) {
            s0.x = (v0_proj[0] / v0_proj[3] + 1) * screen_width / 2;
            s0.y = (v0_proj[1] / v0_proj[3] + 1) * screen_height / 2;
            z0 = v0_proj[2] / v0_proj[3];
        } else continue;
        if (v1_proj[3] != 0) {
            s1.x = (v1_proj[0] / v1_proj[3] + 1) * screen_width / 2;
            s1.y = (v1_proj[1] / v1_proj[3] + 1) * screen_height / 2;
            z1 = v1_proj[2] / v1_proj[3];
        } else continue;
        if (v2_proj[3] != 0) {
            s2.x = (v2_proj[0] / v2_proj[3] + 1) * screen_width / 2;
            s2.y = (v2_proj[1] / v2_proj[3] + 1) * screen_height / 2;
            z2 = v2_proj[2] / v2_proj[3];
        } else continue;

        // Backface culling
        Vec2 e1 = {s1.x - s0.x, s1.y - s0.y};
        Vec2 e2 = {s2.x - s0.x, s2.y - s0.y};
        float cross = e1.x * e2.y - e1.y * e2.x;
        if (cross > 0) continue; // Skip back-facing triangles

        // Rasterize
        float r = face_colors[i][0];
        float g = face_colors[i][1];
        float b = face_colors[i][2];
        rasterize_triangle(s0, s1, s2, z0, z1, z2, r, g, b);
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
    SDL_Color textCol = {255, 255, 255, 255};
    render_debug_text(10, 10, text, textCol);

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
