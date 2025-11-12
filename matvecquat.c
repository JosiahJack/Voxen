// matvecquat.c - Matrices, Vectors, Quaternions
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "matvecquat.h"

void normalize_vector(float* x, float* y, float* z) {
    float len = sqrtf(*x * *x + *y * *y + *z * *z);
    if (len > 1e-6f) { *x /= len; *y /= len; *z /= len; } // Length check to avoid division by zero.
}

Vector3 add_vector3(Vector3 a, Vector3 b) {
    Vector3 res = {a.x + b.x, a.y + b.y, a.z + b.z};
    return res;
}

Vector3 sub_vector3(Vector3 a, Vector3 b) {
    Vector3 res = {a.x - b.x, a.y - b.y, a.z - b.z};
    return res;
}


Vector3 scale_vector3(Vector3 v, float s) {
    Vector3 res = {v.x * s, v.y * s, v.z * s};
    return res;
}

float dot(float x1, float y1, float z1, float x2, float y2, float z2) {
    return x1 * x2 + y1 * y2 + z1 * z2;
}

float dot_vector3(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float dist_sq_vector3(Vector3 a, Vector3 b) {
    Vector3 d = sub_vector3(a, b);
    return dot_vector3(d, d);
}

Vector3 cross_vector3(Vector3 a, Vector3 b) {
    Vector3 res;
    res.x = a.y * b.z - a.z * b.y;
    res.y = a.z * b.x - a.x * b.z;
    res.z = a.x * b.y - a.y * b.x;
    return res;
}

float length_vector3(Vector3 v) {
    return sqrtf(dot_vector3(v, v));
}

Vector3 normalize_vector3(Vector3 v) {
    float len = length_vector3(v);
    if (len > 1e-6f) {
        v.x /= len; v.y /= len; v.z /= len;
    }
    return v;
}

Vector3 mul_mat4_vector3(const float* mat, Vector3 v) {
    // Assume homogeneous, w=1
    Vector3 res;
    res.x = mat[0] * v.x + mat[4] * v.y + mat[8] * v.z + mat[12];
    res.y = mat[1] * v.x + mat[5] * v.y + mat[9] * v.z + mat[13];
    res.z = mat[2] * v.x + mat[6] * v.y + mat[10] * v.z + mat[14];
    return res;
}

// Construct rotation matrix (column-major, Unity: X+ right, Y+ up, Z+ forward)
void quat_to_matrix(Quaternion* q, float* m) {
    float x = q->x, y = q->y, z = q->z, w = q->w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;

    // Column-major rotation matrix for Unity (Y+ up, Z+ forward, X+ right)
    m[0]  = 1.0f - 2.0f * (y2 + z2); // Right X
    m[1]  = 2.0f * (xy + wz);        // Right Y
    m[2]  = 2.0f * (xz - wy);        // Right Z
    m[3]  = 0.0f;
    m[4]  = 2.0f * (xy - wz);        // Up X
    m[5]  = 1.0f - 2.0f * (x2 + z2); // Up Y
    m[6]  = 2.0f * (yz + wx);        // Up Z
    m[7]  = 0.0f;
    m[8]  = 2.0f * (xz + wy);        // Forward X
    m[9]  = 2.0f * (yz - wx);        // Forward Y
    m[10] = 1.0f - 2.0f * (x2 + y2); // Forward Z
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}
