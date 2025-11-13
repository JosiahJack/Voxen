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

float magnitude_vector3(const Vector3 v) {
    return sqrtf(dot_vector3(v, v));
}

Vector3 min_vector3(Vector3 a, Vector3 b) {
    return (Vector3){
        a.x < b.x ? a.x : b.x,
        a.y < b.y ? a.y : b.y,
        a.z < b.z ? a.z : b.z
    };
}

Vector3 max_vector3(Vector3 a, Vector3 b) {
    return (Vector3){
        a.x > b.x ? a.x : b.x,
        a.y > b.y ? a.y : b.y,
        a.z > b.z ? a.z : b.z
    };
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

Quaternion conjugate_quaternion(const Quaternion q) {
    Quaternion res = {q.w, -q.x, -q.y, -q.z};
    return res;
}

Quaternion mul_quaternion(const Quaternion a, const Quaternion b) {
    Quaternion res;
    res.w = a.w * b.w - dot_vector3((Vector3){a.x, a.y, a.z}, (Vector3){b.x, b.y, b.z});
    Vector3 cross_part = cross_vector3((Vector3){a.x, a.y, a.z}, (Vector3){b.x, b.y, b.z});
    Vector3 w_parts = scale_vector3((Vector3){a.x, a.y, a.z}, b.w);
    Vector3 wb_parts = scale_vector3((Vector3){b.x, b.y, b.z}, a.w);
    Vector3 xyz = add_vector3(add_vector3(cross_part, w_parts), wb_parts);
    res.x = xyz.x; res.y = xyz.y; res.z = xyz.z;
    return res;
}

Vector3 rotate_quaternion(const Quaternion q, const Vector3 v) {
    Quaternion vq = {0.0f, v.x, v.y, v.z};
    Quaternion temp = mul_quaternion(vq, conjugate_quaternion(q));
    Quaternion result = mul_quaternion(q, temp);
    return (Vector3){result.x, result.y, result.z};
}

Quaternion axis_angle_quaternion(const Vector3 axis, float angle) {
    float half = angle * 0.5f;
    float s = sinf(half);
    Quaternion res = {cosf(half), s * axis.x, s * axis.y, s * axis.z};
    return res;
}

void normalize_quaternion(Quaternion* q) {
    float mag = sqrtf(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
    if (mag > 1e-6f) {
        q->w /= mag; q->x /= mag; q->y /= mag; q->z /= mag;
    } else {
        q->w = 1.0f; q->x = q->y = q->z = 0.0f;
    }
}

