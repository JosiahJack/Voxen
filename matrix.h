#ifndef VOXEN_MATRIX_H
#define VOXEN_MATRIX_H

#include "quaternion.h"

// Matrix helper functions
void mat4_identity(float* m);
void mat4_perspective(float* m, float fov, float aspect, float near, float far);
void mat4_ortho(float* m, float left, float right, float bottom, float top, float near, float far);
void mat4_from_quat(float *out, float x, float y, float z, float w);
void mat4_multiply(float* result, float* a, float* b);
void mat4_scale(float *m, float sx, float sy, float sz);
void mat4_inverse(float* out, float* m);
void mat4_lookat(float* m, float eyeX, float eyeY, float eyeZ, Quaternion* orientation);
void mat4_lookat_vec(float *m, float eye[3], float target[3], float up[3]);
void mat4_rotate_x(float *m, float angle);
void mat4_translate(float *m, float x, float y, float z);
void mat4_compose(float *out, float tx, float ty, float tz, float rx, float ry, float rz, float rw, float sx, float sy, float sz);
void mat4_transform_vec3(const float *mat, float x, float y, float z, float *out);

#endif // VOXEN_MATRIX_H
