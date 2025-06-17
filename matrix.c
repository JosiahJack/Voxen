#include <math.h>
#include <string.h>
#include "constants.h"
#include "matrix.h"

// Matrix helper functions
void mat4_identity(float* m) {
    for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}

void mat4_perspective(float* m, float fov, float aspect, float near, float far) {
    float f = 1.0f / tan(fov * M_PI / 360.0f);
    mat4_identity(m);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = -(far + near) / (far - near); // Fixed: Negative for correct depth
    m[11] = -1.0f;
    m[14] = -2.0f * far * near / (far - near);
    m[15] = 0.0f;
}

// Matrix helper for 2D orthographic projection for text/UI
void mat4_ortho(float* m, float left, float right, float bottom, float top, float near, float far) {
    mat4_identity(m);
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (far - near);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
}

void mat4_lookat(float* m, float eyeX, float eyeY, float eyeZ, Quaternion* orientation) {
    // Convert quaternion to rotation matrix
    float rotation[16];
    quat_to_matrix(orientation, rotation);

    // Extract forward, right, and up vectors from rotation matrix
    float right[3] = { rotation[0], rotation[1], rotation[2] };
    float up[3] = { rotation[4], rotation[5], rotation[6] };
    float forward[3] = { rotation[8], rotation[9], rotation[10] };

    // Build view matrix
    mat4_identity(m);
    m[0] = right[0];  m[4] = right[1];  m[8]  = right[2];  m[12] = -(right[0] * eyeX + right[1] * eyeY + right[2] * eyeZ);
    m[1] = up[0];     m[5] = up[1];     m[9]  = up[2];     m[13] = -(up[0] * eyeX + up[1] * eyeY + up[2] * eyeZ);
    m[2] = -forward[0]; m[6] = -forward[1]; m[10] = -forward[2]; m[14] = -(-forward[0] * eyeX + -forward[1] * eyeY + -forward[2] * eyeZ);
    m[3] = 0.0f;      m[7] = 0.0f;      m[11] = 0.0f;      m[15] = 1.0f;
}

void mat4_rotate_x(float *m, float angle) {
    float c = cosf(angle), s = sinf(angle);
    float t[16];
    memcpy(t, m, 16 * sizeof(float));
    m[4] = t[4] * c + t[8] * s;
    m[5] = t[5] * c + t[9] * s;
    m[6] = t[6] * c + t[10] * s;
    m[7] = t[7] * c + t[11] * s;
    m[8] = t[8] * c - t[4] * s;
    m[9] = t[9] * c - t[5] * s;
    m[10] = t[10] * c - t[6] * s;
    m[11] = t[11] * c - t[7] * s;
}

void mat4_translate(float *m, float x, float y, float z) {
    mat4_identity(m);
    m[12] = x;
    m[13] = y;
    m[14] = z;
}
