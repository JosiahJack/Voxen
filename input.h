#ifndef INPUT_H
#define INPUT_H

#include "quaternion.h"

// Camera variables
extern float cam_x, cam_y, cam_z;
extern Quaternion cam_rotation;
extern float cam_yaw;
extern float cam_pitch;
extern float cam_roll;
extern float move_speed;
extern float mouse_sensitivity;
extern bool in_cyberspace;
extern bool keys[];
extern int mouse_x, mouse_y;

void Input_MouselookApply();
int Input_KeyDown(uint32_t scancode);
int Input_KeyUp(uint32_t scancode);
int Input_MouseMove(float xrel, float yrel);
void ProcessInput(void);

#endif // INPUT_H
