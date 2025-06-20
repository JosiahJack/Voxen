#ifndef VOXEN_MATRIX_H
#define VOXEN_MATRIX_H

#include "quaternion.h"

// Matrix helper functions
void mat4_identity(float* m);
void mat4_ortho(float* m, float left, float right, float bottom, float top, float near, float far);
void mat4_lookat(float* m, float eyeX, float eyeY, float eyeZ, Quaternion* orientation);
void mat4_rotate_x(float *m, float angle);
void mat4_translate(float *m, float x, float y, float z);

#endif // VOXEN_MATRIX_H
