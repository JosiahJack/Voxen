#ifndef VOXEN_PLAYER_H
#define VOXEN_PLAYER_H

#include <GL/glew.h>
#include <stdbool.h>
#include "quaternion.h"

// Camera variables
extern float cam_x, cam_y, cam_z;
extern Quaternion cam_rotation;
extern float cam_yaw;
extern float cam_pitch;
extern float cam_roll;
extern float cam_fovH;
extern float cam_fovV;
extern bool in_cyberspace;

#endif // VOXEN_PLAYER_H
