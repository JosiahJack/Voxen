#ifndef VOXEN_PLAYER_H
#define VOXEN_PLAYER_H

#include "quaternion.h"

// Camera variables
extern float cam_x, cam_y, cam_z;
extern Quaternion cam_rotation;
extern float cam_yaw;
extern float cam_pitch;
extern float cam_roll;
extern bool in_cyberspace;

extern float testLight_x;
extern float testLight_y;
extern float testLight_z;
extern float testLight_intensity;
extern float testLight_range;
extern float testLight_spotAng;

#endif // VOXEN_PLAYER_H
