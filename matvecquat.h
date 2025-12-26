// matvecquat.h - Matrices, Vectors, Quaternions
#pragma once
#include "vmath.h"
typedef struct { float x,y; } Vector2;
typedef struct { float x,y,z; } Vector3;
typedef struct { float x,y,z,w; } Quaternion;
static inline Vector3 Vector3_A_plus_B(Vector3 a, Vector3 b) { return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline Vector3 Vector3_A_minus_B(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline Vector3 scale_vector3(Vector3 v, float s) { Vector3 res = {v.x * s, v.y * s, v.z * s}; return res; }
static inline float dot(float x1, float y1, float z1, float x2, float y2, float z2) { return x1*x2 + y1*y2 + z1*z2; }
static inline float dot_vector3(Vector3 a, Vector3 b) { return dot(a.x,a.y,a.z, b.x,b.y,b.z); }
static inline float magnitude_vector3(const Vector3 v) { return vsqrtf(dot_vector3(v, v)); }
static inline Vector3 min_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x<b.x ? a.x : b.x, a.y<b.y ? a.y : b.y, a.z<b.z ? a.z : b.z }; }
static inline Vector3 max_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x>b.x ? a.x : b.x, a.y>b.y ? a.y : b.y, a.z>b.z ? a.z : b.z }; }
static inline float dist_sq_vector3(Vector3 a, Vector3 b) { Vector3 d = Vector3_A_minus_B(a, b); return dot_vector3(d, d); }
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
Vector3 quat_rotate(Quaternion q, Vector3 v);
void UpdateInstanceMatrix(int32_t i);

static inline Quaternion mul_quaternion(Quaternion a, Quaternion b) {
    return (Quaternion){
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
    };
}

static inline Vector3 rotate_quaternion(Quaternion rotation, Vector3 axis) {
    Vector3 qv = {rotation.x, rotation.y, rotation.z}; // Take only the xyz, not w
    Vector3 uv = cross_vector3(qv, axis);
    return Vector3_A_plus_B(axis,Vector3_A_plus_B(scale_vector3(uv, 2.0f * rotation.w), scale_vector3(cross_vector3(qv, uv), 2.0f)));
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
/*
// --- Ray-Triangle Intersection (Möller-Trumbore) ---
static inline bool RayTriangle(Vector3 origin, Vector3 dir, Vector3 v0, Vector3 v1, Vector3 v2, float* t) {
    Vector3 edge1 = Vector3_A_minus_B(v1, v0);
    Vector3 edge2 = Vector3_A_minus_B(v2 - v0);
    Vector3 h = cross_vector3(dir, edge2);
    float a = dot_vector3(edge1, h);
    if (vabs(a) < 1e-6) return false;

    float f = 1.0 / a;
    Vector3 s = Vector3_A_minus_B(origin, v0);
    float u = f * dot_vector3(s, h);
    if (u < 0.0 || u > 1.0) return false;

    Vector3 q = cross_vector3(s, edge1);
    float v = f * dot_vector3(dir, q);
    if (v < 0.0 || u + v > 1.0) return false;

    *t = f * dot_vector3(edge2, q);
    return *t > 0.001;
}

// --- Trace Ray for Shadow ---
static inline float TraceRay(Vector3 origin, Vector3 dir, float maxDist) {
    for (int i = 0; i < instancesInPVSCount; i++) {
        uint instanceIdx = instancesIndices[i];
        Instance inst = instances[instanceIdx];
        if (inst.texIndex == 881) continue; // Fullbright light

        mat4 invModel = inverse(instanceMatrices[instanceIdx]);
        Vector3 localOrigin = (invModel * vec4(origin, 1.0)).xyz;
        float instanceRadius = bounds[instanceIdx * BOUNDS_ATTRIBUTES_COUNT + 6]; // first 6 are the mins,maxs xyz
        if (length(localOrigin - origin) > (maxDist + instanceRadius)) continue;

        Vector3 localDir = ((invModel * vec4(dir, 0.0)).xyz);
        uint modelIndex = inst.modelIndex;
        uint vertCount = modelVertexCounts[modelIndex];
        if (vertCount < 3) continue;

        uint triCount = vertCount / 3;
        mat4 matrix = instanceMatrices[instanceIdx];
        uint j = 0;
        uint vertexIdx;
        Vector3 v0, v1, v2;
        for (uint tri = 0; tri < triCount; tri++) {
            vertexIdx = (vertexOffsets[modelIndex] * VERTEX_ATTRIBUTES_COUNT) + (tri * VERTEX_ATTRIBUTES_COUNT);
            j = 0;
            v0 = (Vector3){ vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 0], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 1], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 2] };
            j++;
            v1 = (Vector3){ vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 0], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 1], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 2] };
            j++;
            v2 = (Vector3){ vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 0], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 1], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 2] };
            float t; // Output result
            if (RayTriangle(localOrigin, localDir, v0, v1, v2, t) && (t < maxDist)) return 0.0;
        }
    }
    return 1.0;
}*/

//shadow = TraceRay(worldPos + adjustedNormal * 0.01, lightDir, range);
