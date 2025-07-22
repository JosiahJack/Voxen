#include <math.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include "constants.h"
#include "input.h"
#include "quaternion.h"
#include "player.h"
#include "audio.h"
#include "render.h"

// Camera variables
float cam_x = 0.0f, cam_y = -4.0f, cam_z = 0.0f; // Camera position
Quaternion cam_rotation;
float cam_yaw = 180.0f;
float cam_pitch = 90.0f;
float cam_roll = 0.0f;
float cam_fov = 65.0f;
float move_speed = 0.1f;
float mouse_sensitivity = 0.1f;                 // Mouse look sensitivity
bool in_cyberspace = true;
float sprinting = 0.0f;

float testLight_x = 0.0f;
float testLight_y = -0.64f;
float testLight_z = 1.12f;
float testLight_intensity = 4.0f;
float testLight_range = 16.0f;
float testLight_spotAng = 0.0f;
bool noclip = true;

bool keys[SDL_NUM_SCANCODES] = {0}; // SDL_NUM_SCANCODES 512b, covers all keys
int mouse_x = 0, mouse_y = 0; // Mouse position
int debugView = 0;
int debugValue = 0;

void Input_Init(void) {
    quat_identity(&cam_rotation);
    Quaternion pitch_quat;
    quat_from_axis_angle(&pitch_quat, 1.0f, 0.0f, 0.0f, deg2rad(-90.0f)); // Pitch -90° to look toward Y+
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
    
    if (keys[SDL_SCANCODE_R]) {
        debugView++;
        if (debugView > 5) debugView = 0;
    }
    
    if (keys[SDL_SCANCODE_Y]) {
        debugValue++;
        if (debugValue > 5) debugValue = 0;
    }
    
    if (keys[SDL_SCANCODE_O]) {
        testLight_intensity += 8.0f / 256.0f;
        if (testLight_intensity > 8.0f) testLight_intensity = 8.0f;
    } else if (keys[SDL_SCANCODE_P]) {
        testLight_intensity -= 8.0f / 256.0f;
        if (testLight_intensity < 0.01f) testLight_intensity = 0.01f;
    }
    
    if (keys[SDL_SCANCODE_E]) {
        play_wav("./Audio/weapons/wpistol.wav",0.5f);
    }
    return 0;
}

int Input_KeyUp(uint32_t scancode) {
    keys[scancode] = false;
    return 0;
}

int Input_MouseMove(float xrel, float yrel) {
    cam_yaw += xrel * mouse_sensitivity;
    cam_pitch += yrel * mouse_sensitivity;
    if (cam_pitch > 179.0f) cam_pitch = 179.0f;
    if (cam_pitch < 1.0f) cam_pitch = 1.0f;
    Input_MouselookApply();
    return 0;
}

// Update camera position based on input
void ProcessInput(void) {
    if (keys[SDL_SCANCODE_LSHIFT]) sprinting = 1.0f;
    else sprinting = 0.0f;
    
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

    float finalMoveSpeed = (move_speed + (sprinting * move_speed));
    if (keys[SDL_SCANCODE_F]) {
        cam_x += finalMoveSpeed * facing_x; // Move forward
        cam_y += finalMoveSpeed * facing_y;
        cam_z += finalMoveSpeed * facing_z;
    } else if (keys[SDL_SCANCODE_S]) {
        cam_x -= finalMoveSpeed * facing_x; // Move backward
        cam_y -= finalMoveSpeed * facing_y;
        cam_z -= finalMoveSpeed * facing_z;
    }

    if (keys[SDL_SCANCODE_A]) {
        cam_x -= finalMoveSpeed * strafe_x; // Strafe left
        cam_y -= finalMoveSpeed * strafe_y;
        cam_z -= finalMoveSpeed * strafe_z;
    } else if (keys[SDL_SCANCODE_D]) {
        cam_x += finalMoveSpeed * strafe_x; // Strafe right
        cam_y += finalMoveSpeed * strafe_y;
        cam_z += finalMoveSpeed * strafe_z;
    }

    if (noclip) {
        if (keys[SDL_SCANCODE_V]) cam_z += finalMoveSpeed; // Move up
        else if (keys[SDL_SCANCODE_C]) cam_z -= finalMoveSpeed; // Move down
    }
    
    if (keys[SDL_SCANCODE_T]) {
        cam_roll += move_speed * 5.0f; // Move up
        Input_MouselookApply();
    } else if (keys[SDL_SCANCODE_Q]) {
        cam_roll -= move_speed * 5.0f; // Move down
        Input_MouselookApply();
    }
    
    if (keys[SDL_SCANCODE_J]) {
        testLight_x += finalMoveSpeed;
        lightDirty[0] = true;
    } else if (keys[SDL_SCANCODE_K]) {
        testLight_x -= finalMoveSpeed;
        lightDirty[0] = true;
    }
    
    if (keys[SDL_SCANCODE_N]) {
        testLight_y += finalMoveSpeed;
        lightDirty[0] = true;
    } else if (keys[SDL_SCANCODE_M]) {
        testLight_y -= finalMoveSpeed;
        lightDirty[0] = true;
    }
    
    if (keys[SDL_SCANCODE_U]) {
        testLight_z += finalMoveSpeed;
        lightDirty[0] = true;
    } else if (keys[SDL_SCANCODE_I]) {
        testLight_z -= finalMoveSpeed;
        lightDirty[0] = true;
    }
    
    if (keys[SDL_SCANCODE_L]) {
        testLight_range += finalMoveSpeed;
        lightDirty[0] = true;
    } else if (keys[SDL_SCANCODE_SEMICOLON]) {
        testLight_range -= finalMoveSpeed;
        if (testLight_range < 0.0f) testLight_range = 0.0f;
        else lightDirty[0] = true;
    }
    
    if (keys[SDL_SCANCODE_B]) {
        testLight_spotAng += finalMoveSpeed * 2.0f;
        if (testLight_spotAng > 180.0f) testLight_spotAng = 180.0f;
    } else if (keys[SDL_SCANCODE_Z]) {
        testLight_spotAng -= finalMoveSpeed * 2.0f;
        if (testLight_spotAng < 0.0f) testLight_spotAng = 0.0f;
    }
    
    if (keys[SDL_SCANCODE_X]) {
        for (int i=0;i<MAX_VISIBLE_LIGHTS;++i) lightDirty[i] = true;
    }
}
