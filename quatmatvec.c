#include <math.h>
#include <string.h>
#include "constants.h"
#include "quatmatvec.h"
#include "instance.h"
#include "data_models.h"

float deg2rad(float degrees) {
    return degrees * (M_PI / 180.0f);
}

float rad2deg(float radians) {
    return radians * (180.0f / M_PI);
}

// Initialize a quaternion (identity: no rotation)
void quat_identity(Quaternion* q) {
    q->w = 1.0f;
    q->x = 0.0f;
    q->y = 0.0f;
    q->z = 0.0f;
}

// Normalize a quaternion
void quat_normalize(Quaternion* q) {
    float len = sqrt(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
    if (len > 1e-6f) {
        q->w /= len;
        q->x /= len;
        q->y /= len;
        q->z /= len;
    } else {
        quat_identity(q);
    }
}

// Multiply two quaternions: q = q1 * q2
void quat_multiply(Quaternion* result, Quaternion* q1, Quaternion* q2) {
    float w = q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z;
    float x = q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y;
    float y = q1->w * q2->y + q1->y * q2->w + q1->z * q2->x - q1->x * q2->z;
    float z = q1->w * q2->z + q1->z * q2->w + q1->x * q2->y - q1->y * q2->x;
    result->w = w;
    result->x = x;
    result->y = y;
    result->z = z;
    quat_normalize(result);
}

// Create a quaternion from an axis and angle (in radians)
void quat_from_axis_angle(Quaternion* q, float x, float y, float z, float angle) {
    float half_angle = (-angle) * 0.5f;  // Negate for left-handed
    float s = sinf(half_angle);
    q->x = x * s;  // X-axis (right)
    q->y = y * s;  // Y-axis (up)
    q->z = z * s;  // Z-axis (forward)
    q->w = cosf(half_angle);
    quat_normalize(q);
}

void quat_to_euler(Quaternion* q, float* yaw, float* pitch, float* roll) {
    // Pitch (X-axis rotation, same as original)
    float sinp = 2.0f * (q->w * q->x + q->y * q->z);
    float cosp = 1.0f - 2.0f * (q->x * q->x + q->y * q->y);
    *pitch = atan2(sinp, cosp) * 180.0f / M_PI;

    // Yaw (Y-axis rotation, original Z-axis)
    float siny = 2.0f * (q->w * q->y - q->x * q->z); // Note: Negated for left-handed
    float cosy = 1.0f - 2.0f * (q->y * q->y + q->z * q->z);
    *yaw = atan2(siny, cosy) * 180.0f / M_PI;

    // Roll (Z-axis rotation, original Y-axis)
    float sinr = 2.0f * (q->w * q->z + q->x * q->y); // Note: Adjusted for Z-axis
    *roll = asin(sinr) * 180.0f / M_PI;
}

void mat4_identity(float* m) {
    for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}

// Convert quaternion to a 4x4 matrix
// Converts a normalized quaternion to a 4x4 column-major matrix (left-handed, Y-up, Z-forward)
void quat_to_matrix(Quaternion* q, float* m) {
    float x = q->x, y = q->y, z = q->z, w = q->w;

    float xx = x * x;
    float xy = x * y;
    float xz = x * z;
    float xw = x * w;
    float yy = y * y;
    float yz = y * z;
    float yw = y * w;
    float zz = z * z;
    float zw = z * w;

    mat4_identity(m);

    // X axis (right)
    m[0] = 1.0f - 2.0f * (yy + zz);
    m[1] = 2.0f * (xy + zw);
    m[2] = 2.0f * (xz - yw);

    // Y axis (up)
    m[4] = 2.0f * (xy - zw);
    m[5] = 1.0f - 2.0f * (xx + zz);
    m[6] = 2.0f * (yz + xw);

    // Z axis (forward)
    m[8] = 2.0f * (xz + yw);
    m[9] = 2.0f * (yz - xw);
    m[10] = 1.0f - 2.0f * (xx + yy);

    m[15] = 1.0f;
}

// Create a quaternion from yaw (around Z) and pitch (around X) in degrees
void quat_from_yaw_pitch(Quaternion* q, float yaw_deg, float pitch_deg) {
    float yaw = yaw_deg * (M_PI / 180.0f);
    float pitch = pitch_deg * (M_PI / 180.0f);

    float half_yaw = yaw * 0.5f;
    float half_pitch = pitch * 0.5f;

    float cy = cosf(half_yaw);
    float sy = sinf(half_yaw);
    float cp = cosf(half_pitch);
    float sp = sinf(half_pitch);

    q->w = cy * cp;
    q->x = sp * cy;
    q->y = sy * cp;
    q->z = sp * sy;
    quat_normalize(q);
}

// Create a quaternion from yaw (around Z), pitch (around X), and roll (around Y) in degrees
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) {
    float yaw = yaw_deg * M_PI / 180.0f;
    float pitch = pitch_deg * M_PI / 180.0f;
    float roll = roll_deg * M_PI / 180.0f;

    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);
    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);
    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);

    q->w = cy * cp * cr + sy * sp * sr;
    q->x = cy * sp * cr + sy * cp * sr;
    q->y = sy * cp * cr - cy * sp * sr;
    q->z = cy * cp * sr - sy * sp * cr;
    quat_normalize(q);
}

void normalize_vector(float* x, float* y, float* z) {
    float len = sqrtf(*x * *x + *y * *y + *z * *z);
    if (len > 1e-6f) { // Avoid division by zero
        *x /= len;
        *y /= len;
        *z /= len;
    }
}

// Generates Projection Matrix4x4 for Geometry Rasterizer Pass.
void mat4_perspective(float* m, float fov, float aspect, float near, float far) {
    float f = 1.0f / tan(fov * M_PI / 360.0f);
    mat4_identity(m);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = -(far + near) / (far - near); // Fixed: Negative for correct depth
    m[11] = -1.0f;
    m[14] = -2.0f * far * near / (far - near);
    // Generates perspective projection matrix for OpenGL (Z in [-1, 1]).
    // For DirectX (Z in [0, 1]), use:
    //     m[14] = -far * near / (far - near); // Adjusted for Z in [0, 1]

    m[15] = 0.0f;
}

// Generates View Matrix4x4 for Geometry Rasterizer Pass
// Constructs a View matrix from camera world position + orientation
void mat4_lookat(float* m, float eyeX, float eyeY, float eyeZ, Quaternion* orientation) {
    float rotation[16];
    quat_to_matrix(orientation, rotation); // Rotation matrix (camera orientation)

    // Extract basis vectors (camera space axes from rotation matrix)
    float right[3]   = { rotation[0], rotation[1], rotation[2] };    // local X
    float up[3]      = { rotation[4], rotation[5], rotation[6] };    // local Y
    float forward[3] = { rotation[8], rotation[9], rotation[10] };   // local Z

    // Invert the camera transform: transpose rotation, translate by -eye
    m[0]  = right[0];    m[1]  = right[1];    m[2]  = right[2];    m[3]  = 0.0f;
    m[4]  = up[0];       m[5]  = up[1];       m[6]  = up[2];       m[7]  = 0.0f;
    m[8]  = -forward[0]; m[9]  = -forward[1]; m[10] = -forward[2]; m[11] = 0.0f;

    m[12] = -(right[0] * eyeX + right[1] * eyeY + right[2] * eyeZ);
    m[13] = -(up[0]    * eyeX + up[1]    * eyeY + up[2]    * eyeZ);
    m[14] =  (forward[0] * eyeX + forward[1] * eyeY + forward[2] * eyeZ);
    m[15] = 1.0f;
}

// Matrix helper for 2D orthographic projection for text/UI
void mat4_ortho(float* m, float left, float right, float bottom, float top, float near, float far) {
    mat4_identity(m);
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (far - near);
    // Generates orthographic projection matrix for OpenGL (Z in [-1, 1]).
    // For DirectX (Z in [0, 1]), use:
    //    m[10] = -1.0f / (far - near), m[14] = -near / (far - near).
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
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

void SetUpdatedMatrix(float *mat, float posx, float posy, float posz, float rotx, float roty, float rotz, float rotw, float sclx, float scly, float sclz) {
    float x = rotx;
    float y = roty;
    float z = rotz;
    float w = rotw;

    // Compute rotation matrix (matches updated quat_to_matrix for Y+ up, left-handed)
    float xx = x * x;
    float xy = x * y;
    float xz = x * z;
    float xw = x * w;
    float yy = y * y;
    float yz = y * z;
    float yw = y * w;
    float zz = z * z;
    float zw = z * w;

    float rot[16];
    mat4_identity(rot);
    // X (right) = Original X (right), negate Z component for left-handed
    rot[0] = 1.0f - 2.0f * (yy + zz); // Xx
    rot[1] = 2.0f * (xy + zw);        // Xy
    rot[2] = -2.0f * (xz - yw);       // Xz (negated)

    // Y (up) = Original Z (up)
    rot[4] = 2.0f * (xz + yw);        // Zx
    rot[5] = 2.0f * (yz - xw);        // Zy
    rot[6] = 1.0f - 2.0f * (xx + yy); // Zz

    // Z (forward) = Original Y (forward), negate X and Z components
    rot[8] = -2.0f * (xy - zw);       // Yx (negated)
    rot[9] = 1.0f - 2.0f * (xx + zz); // Yy
    rot[10] = -2.0f * (yz + xw);      // Yz (negated)

    // Build final matrix with scale and translation
    mat4_identity(mat);
    mat[0]  = rot[0] * sclx; mat[1]  = rot[1] * scly; mat[2]  = rot[2]  * sclz; mat[3]  = 0.0f;
    mat[4]  = rot[4] * sclx; mat[5]  = rot[5] * scly; mat[6]  = rot[6]  * sclz; mat[7]  = 0.0f;
    mat[8]  = rot[8] * sclx; mat[9]  = rot[9] * scly; mat[10] = rot[10] * sclz; mat[11] = 0.0f;
    mat[12] = posx;          mat[13] = posy;          mat[14] = posz;           mat[15] = 1.0f;
}

void UpdateInstanceMatrix(int i) {
    if (instances[i].modelIndex >= MODEL_COUNT) { dirtyInstances[i] = false; return; }
    if (modelVertexCounts[instances[i].modelIndex] < 1) { dirtyInstances[i] = false; return; } // Empty model
    if (instances[i].modelIndex < 0) return; // Culled

    float mat[16]; // 4x4 matrix
    SetUpdatedMatrix(mat, instances[i].posx, instances[i].posy, instances[i].posz, instances[i].rotx, instances[i].roty, instances[i].rotz, instances[i].rotw, instances[i].sclx, instances[i].scly, instances[i].sclz);
    memcpy(&modelMatrices[i * 16], mat, 16 * sizeof(float));
    dirtyInstances[i] = false;
}
