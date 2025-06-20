#include <math.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include "constants.h"
#include "input.h"
#include "quaternion.h"
#include "player.h"

// Camera variables
float cam_x = 0.0f, cam_y = -4.0f, cam_z = 3.0f; // Camera position
// float cam_yaw = 0.0f, cam_pitch = 0.0f;         // Camera orientation
Quaternion cam_rotation;
float cam_yaw = 0.0f;
float cam_pitch = 90.0f;
float cam_roll = 0.0f;
float cam_fovH = 90.0f;
float cam_fovV = 65.0f;
float move_speed = 0.1f;
float mouse_sensitivity = 0.1f;                 // Mouse look sensitivity
bool in_cyberspace = true;

bool keys[SDL_NUM_SCANCODES] = {0}; // SDL_NUM_SCANCODES 512b, covers all keys
int mouse_x = 0, mouse_y = 0; // Mouse position

void Input_Init(void) {
    quat_identity(&cam_rotation);
    Quaternion pitch_quat;
    quat_from_axis_angle(&pitch_quat, 1.0f, 0.0f, 0.0f,deg2rad(-90.0f)); // Pitch -90° to look toward Y+
    quat_multiply(&cam_rotation, &pitch_quat, &cam_rotation);
    Input_MouselookApply();
}
    
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
    cam_pitch -= yrel * mouse_sensitivity;
    if (cam_pitch > 179.0f) cam_pitch = 179.0f;
    if (cam_pitch < 1.0f) cam_pitch = 1.0f;
    Input_MouselookApply();
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
        cam_x -= move_speed * facing_x; // Move forward
        cam_y -= move_speed * facing_y;
        cam_z -= move_speed * facing_z;
    } else if (keys[SDL_SCANCODE_S]) {
        cam_x += move_speed * facing_x; // Move backward
        cam_y += move_speed * facing_y;
        cam_z += move_speed * facing_z;
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
