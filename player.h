#ifndef PLAYER_H
#define PLAYER_H

#include "quaternion.h"

// Camera variables
extern float cam_x, cam_y, cam_z;
extern Quaternion cam_rotation;
extern float cam_yaw;
extern float cam_pitch;
extern float cam_roll;
extern bool in_cyberspace;

#endif // PLAYER_H
