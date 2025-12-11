// matvecquat.h - Matrices, Vectors, Quaternions
#pragma once
#include "vmath.h"
typedef struct { float x,y; } Vector2;
typedef struct { float x,y,z; } Vector3;
typedef struct { float x,y,z,w; } Quaternion;
static inline Vector3 add_vector3(Vector3 a, Vector3 b) { return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline Vector3 sub_vector3(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline Vector3 scale_vector3(Vector3 v, float s) { Vector3 res = {v.x * s, v.y * s, v.z * s}; return res; }
static inline float dot(float x1, float y1, float z1, float x2, float y2, float z2) { return x1*x2 + y1*y2 + z1*z2; }
static inline float dot_vector3(Vector3 a, Vector3 b) { return dot(a.x,a.y,a.z, b.x,b.y,b.z); }
static inline float magnitude_vector3(const Vector3 v) { return vsqrtf(dot_vector3(v, v)); }
static inline Vector3 min_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x<b.x ? a.x : b.x, a.y<b.y ? a.y : b.y, a.z<b.z ? a.z : b.z }; }
static inline Vector3 max_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x>b.x ? a.x : b.x, a.y>b.y ? a.y : b.y, a.z>b.z ? a.z : b.z }; }
static inline float dist_sq_vector3(Vector3 a, Vector3 b) { Vector3 d = sub_vector3(a, b); return dot_vector3(d, d); }
static inline Vector3 cross_vector3(Vector3 a, Vector3 b) { return (Vector3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
static inline Vector3 normalize_vector3(Vector3 v) { float len = magnitude_vector3(v); return len > 0.000001f ? (Vector3){v.x / len, v.y / len, v.z / len} : v; }
static inline float squareDistance2D(float x1, float z1, float x2, float z2) { float dx = x2 - x1; float dz = z2 - z1; return dx * dx + dz * dz; }
static inline float squareDistance3D(float x1, float y1, float z1, float x2, float y2, float z2) { float dx = x2 - x1; float dy = y2 - y1; float dz = z2 - z1; return dx * dx + dy * dy + dz * dz; }
void normalize_vector(float* x, float* y, float* z);
__attribute__((pure)) Vector3 mul_mat4_vector3(const float* m, Vector3 v);
void quat_to_matrix(Quaternion* q, float* m);
Quaternion conjugate_quaternion(const Quaternion q);
Quaternion axis_angle_quaternion(const Vector3 axis, float angle);
void normalize_quaternion(Quaternion* q);
void UpdateInstanceMatrix(int32_t i);

static inline Quaternion mul_quaternion(Quaternion a, Quaternion b) {
    return (Quaternion){
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
    };
}

static inline Vector3 rotate_quaternion(Quaternion q, Vector3 v) {
    Vector3 qv = {q.x, q.y, q.z};
    Vector3 uv = cross_vector3(qv, v);
    Vector3 uuv = cross_vector3(qv, uv);
    return add_vector3(v, add_vector3(scale_vector3(uv, 2.0f * q.w), scale_vector3(uuv, 2.0f)));
}

static inline void mul_mat4(float *out, const float *a, const float *b) { // out = a * b
    float result[16];
    for (int32_t col = 0; col < 4; ++col) {
        for (int32_t row = 0; row < 4; ++row) {
            result[col*4 + row] =
                a[0*4 + row] * b[col*4 + 0] +
                a[1*4 + row] * b[col*4 + 1] +
                a[2*4 + row] * b[col*4 + 2] +
                a[3*4 + row] * b[col*4 + 3];
        }
    }
   
    for (int32_t i = 0; i < 16; i++) out[i] = result[i]; // copy back
}
