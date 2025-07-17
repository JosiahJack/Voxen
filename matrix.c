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

void mat4_multiply(float* result, float* a, float* b) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; k++) {
                result[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
            }
        }
    }
}

void mat4_scale(float *m, float sx, float sy, float sz) {
    float t[16];
    memcpy(t, m, 16 * sizeof(float));
    m[0] = t[0] * sx; m[4] = t[4] * sx; m[8] = t[8] * sx; m[12] = t[12] * sx;
    m[1] = t[1] * sy; m[5] = t[5] * sy; m[9] = t[9] * sy; m[13] = t[13] * sy;
    m[2] = t[2] * sz; m[6] = t[6] * sz; m[10] = t[10] * sz; m[14] = t[14] * sz;
}

void mat4_inverse(float *out, float *m) {
    float tmp[16];
    tmp[0]  =  m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    tmp[4]  = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    tmp[8]  =  m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    tmp[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    tmp[1]  = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    tmp[5]  =  m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    tmp[9]  = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    tmp[13] =  m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    tmp[2]  =  m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    tmp[6]  = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    tmp[10] =  m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    tmp[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    tmp[3]  = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    tmp[7]  =  m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    tmp[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    tmp[15] =  m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    float det = m[0] * tmp[0] + m[1] * tmp[4] + m[2] * tmp[8] + m[3] * tmp[12];
    if (fabs(det) < 1e-10) {
        for (int i = 0; i < 16; i++) out[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        return;
    }

    det = 1.0f / det;
    for (int i = 0; i < 16; i++) out[i] = tmp[i] * det;
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

void mat4_lookat_vec(float *m, float eye[3], float target[3], float up[3]) {
    float f[3] = {target[0] - eye[0], target[1] - eye[1], target[2] - eye[2]};
    float len = sqrtf(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    if (len > 0.0f) { f[0] /= len; f[1] /= len; f[2] /= len; }

    float s[3];
    s[0] = f[1] * up[2] - f[2] * up[1];
    s[1] = f[2] * up[0] - f[0] * up[2];
    s[2] = f[0] * up[1] - f[1] * up[0];
    len = sqrtf(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
    if (len > 0.0f) { s[0] /= len; s[1] /= len; s[2] /= len; }

    float u[3];
    u[0] = s[1] * f[2] - s[2] * f[1];
    u[1] = s[2] * f[0] - s[0] * f[2];
    u[2] = s[0] * f[1] - s[1] * f[0];

    mat4_identity(m);
    m[0] = s[0]; m[4] = s[1]; m[8] = s[2];
    m[1] = u[0]; m[5] = u[1]; m[9] = u[2];
    m[2] = -f[0]; m[6] = -f[1]; m[10] = -f[2];
    m[12] = -(s[0] * eye[0] + s[1] * eye[1] + s[2] * eye[2]);
    m[13] = -(u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2]);
    m[14] = (f[0] * eye[0] + f[1] * eye[1] + f[2] * eye[2]);
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

void mat4_transform_vec3(const float *mat, float x, float y, float z, float *out) {
    out[0] = mat[0] * x + mat[4] * y + mat[8] *  z + mat[12];
    out[1] = mat[1] * x + mat[5] * y + mat[9] *  z + mat[13];
    out[2] = mat[2] * x + mat[6] * y + mat[10] * z + mat[14];
}
