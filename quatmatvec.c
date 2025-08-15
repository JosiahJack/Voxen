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
    // Remap axis: original (x, y, z) → Unity (x, z, y)
    float unity_x = x;  // X (right) stays X
    float unity_y = z;  // Z (up) → Y (up)
    float unity_z = y;  // Y (forward) → Z (forward)

    // Negate angle for left-handed rotation
    float half_angle = (-angle) * 0.5f;  // Negate for left-handed
    float s = sinf(half_angle);
    q->x = unity_x * s;  // X-axis (right)
    q->y = unity_y * s;  // Y-axis (up)
    q->z = unity_z * s;  // Z-axis (forward)
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
void quat_to_matrix(Quaternion* q, float* m) {
    float xx = q->x * q->x;
    float xy = q->x * q->y;
    float xz = q->x * q->z;
    float xw = q->x * q->w;
    float yy = q->y * q->y;
    float yz = q->y * q->z;
    float yw = q->y * q->w;
    float zz = q->z * q->z;
    float zw = q->z * q->w;

    mat4_identity(m);
    // Unity X (right) = Current X (right), negate Z component for left-handed
    m[0]  = 1.0f - 2.0f * (yy + zz); // Xx
    m[1]  = 2.0f * (xy + zw);        // Xy
    m[2]  = -2.0f * (xz - yw);       // Xz (negated for left-handed)

    // Unity Y (up) = Current Z (up)
    m[4]  = 2.0f * (xz + yw);        // Zx
    m[5]  = 2.0f * (yz - xw);        // Zy
    m[6]  = 1.0f - 2.0f * (xx + yy); // Zz

    // Unity Z (forward) = Current Y (forward), negate X component for left-handed
    m[8]  = -2.0f * (xy - zw);       // Yx (negated for left-handed)
    m[9]  = 1.0f - 2.0f * (xx + zz); // Yy
    m[10] = -2.0f * (yz + xw);       // Yz (negated for consistency)

    m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}

// Create a quaternion from yaw (around Z) and pitch (around X) in degrees
void quat_from_yaw_pitch(Quaternion* q, float yaw_deg, float pitch_deg) {
    // Convert to radians
    float yaw = yaw_deg * M_PI / 180.0f;
    float pitch = pitch_deg * M_PI / 180.0f;

    // Compute half angles
    float half_yaw = yaw * 0.5f;
    float half_pitch = pitch * 0.5f;

    // Precompute sines and cosines
    float cy = cos(half_yaw);
    float sy = sin(half_yaw);
    float cp = cos(half_pitch);
    float sp = sin(half_pitch);

    // Compute quaternion for Y-X order (yaw around Y, pitch around X)
    // Unity: yaw (Y), pitch (X), no roll
    q->w = cy * cp;
    q->x = cy * sp;  // X-axis (pitch)
    q->y = sy * cp;  // Y-axis (yaw)
    q->z = -sy * sp; // Z-axis (no roll, negated for left-handed)
    quat_normalize(q);
}

// Create a quaternion from yaw (around Z), pitch (around X), and roll (around Y) in degrees
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg) {
    // Convert to radians
    float yaw = yaw_deg * M_PI / 180.0f;
    float pitch = pitch_deg * M_PI / 180.0f;
    float roll = roll_deg * M_PI / 180.0f;

    // Compute half angles
    float half_yaw = yaw * 0.5f;
    float half_pitch = pitch * 0.5f;
    float half_roll = roll * 0.5f;

    // Precompute sines and cosines
    float cy = cos(half_yaw);
    float sy = sin(half_yaw);
    float cp = cos(half_pitch);
    float sp = sin(half_pitch);
    float cr = cos(half_roll);
    float sr = sin(half_roll);

    // Compute quaternion for Y-X-Z order (yaw around Y, pitch around X, roll around Z)
    q->w = cy * cp * cr - sy * sp * sr; // Negated sr for left-handed
    q->x = cy * sp * cr - sy * cp * sr; // X-axis (pitch)
    q->y = sy * cp * cr + cy * sp * sr; // Y-axis (yaw)
    q->z = cy * cp * sr + sy * sp * cr; // Z-axis (roll)
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
void mat4_lookat(float* m, float eyeX, float eyeY, float eyeZ, Quaternion* orientation) {
    // Remap eye position: (X, Y, Z) → (X, Z, Y) for Y+ up
    float unity_eyeX = eyeX;  // X stays X
    float unity_eyeY = eyeZ;  // Z → Y (up)
    float unity_eyeZ = eyeY;  // Y → Z (forward)

    // Get rotation matrix (Y+ up, left-handed)
    float rotation[16];
    quat_to_matrix(orientation, rotation);

    // Extract right, up, forward vectors
    float right[3] = { rotation[0], rotation[1], rotation[2] };   // X-axis (right)
    float up[3] = { rotation[4], rotation[5], rotation[6] };      // Y-axis (up)
    float forward[3] = { rotation[8], rotation[9], rotation[10] }; // Z-axis (forward)

    // Normalize vectors to ensure orthogonality
    normalize_vector(&right[0], &right[1], &right[2]);
    normalize_vector(&up[0], &up[1], &up[2]);
    normalize_vector(&forward[0], &forward[1], &forward[2]);

    // Build view matrix (right-handed view space: Z- forward)
    mat4_identity(m);
    m[0] = right[0];   m[4] = right[1];   m[8]  = right[2];   m[12] = -(right[0] * unity_eyeX + right[1] * unity_eyeY + right[2] * unity_eyeZ);
    m[1] = up[0];      m[5] = up[1];      m[9]  = up[2];      m[13] = -(up[0] * unity_eyeX + up[1] * unity_eyeY + up[2] * unity_eyeZ);
    m[2] = -forward[0]; m[6] = -forward[1]; m[10] = -forward[2]; m[14] = (forward[0] * unity_eyeX + forward[1] * unity_eyeY + forward[2] * unity_eyeZ);
    m[3] = 0.0f;       m[7] = 0.0f;       m[11] = 0.0f;       m[15] = 1.0f;
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
    // Unity X (right) = Original X (right), negate Z component for left-handed
    rot[0] = 1.0f - 2.0f * (yy + zz); // Xx
    rot[1] = 2.0f * (xy + zw);        // Xy
    rot[2] = -2.0f * (xz - yw);       // Xz (negated)

    // Unity Y (up) = Original Z (up)
    rot[4] = 2.0f * (xz + yw);        // Zx
    rot[5] = 2.0f * (yz - xw);        // Zy
    rot[6] = 1.0f - 2.0f * (xx + yy); // Zz

    // Unity Z (forward) = Original Y (forward), negate X and Z components
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
