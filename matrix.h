#ifndef VOXEN_MATRIX_H
#define VOXEN_MATRIX_H

#include "quaternion.h"

// Matrix helper functions
void mat4_identity(float* m);
void mat4_perspective(float* m, float fov, float aspect, float near, float far);
void mat4_ortho(float* m, float left, float right, float bottom, float top, float near, float far);
void mat4_lookat(float* m, float eyeX, float eyeY, float eyeZ, Quaternion* orientation);

#endif // VOXEN_MATRIX_H
