// matvecquat.c - Matrices, Vectors, Quaternions
#include <stdint.h>
#include <stdbool.h>
#include "voxen.h"
#include "entity.h"
extern uint16_t loadedModelsMaxIndex;
extern float aspect3D;
extern float rasterPerspectiveProjection[16];
extern float shadowmapsPerspectiveProjection[16];

void normalize_vector(float* x, float* y, float* z) { float len = vsqrtf(*x * *x + *y * *y + *z * *z); if (len > 1e-6f) { *x /= len; *y /= len; *z /= len; } }
__attribute__((pure)) Vector3 mul_mat4_vector3(const float* m, Vector3 v) { return (Vector3){m[0]*v.x+m[4]*v.y+m[8]*v.z+m[12], m[1]*v.x+m[5]*v.y+m[9]*v.z+m[13], m[2]*v.x+m[6]*v.y+m[10]*v.z+m[14]}; }

// Construct rotation matrix (column-major: X+ right, Y+ up, Z+ forward)
void quat_to_matrix(Quaternion* q, float* m) {
    float x = q->x, y = q->y, z = q->z, w = q->w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;
    m[0]  = 1.0f - 2.0f * (y2 + z2); // Right X
    m[1]  = 2.0f * (xy + wz);        // Right Y
    m[2]  = 2.0f * (xz - wy);        // Right Z
    m[4]  = 2.0f * (xy - wz);        // Up X
    m[5]  = 1.0f - 2.0f * (x2 + z2); // Up Y
    m[6]  = 2.0f * (yz + wx);        // Up Z
    m[8]  = 2.0f * (xz + wy);        // Forward X
    m[9]  = 2.0f * (yz - wx);        // Forward Y
    m[10] = 1.0f - 2.0f * (x2 + y2); // Forward Z
    m[3] = m[7] = m[11] = 0.0f;
    m[12] = m[13] = m[14] = 0.0f;
    m[15] = 1.0f;
}

Quaternion conjugate_quaternion(const Quaternion q) { return (Quaternion){q.w, -q.x, -q.y, -q.z}; }
Quaternion axis_angle_quaternion(const Vector3 axis, float angle) { float s = vsinf(angle * 0.5f); return (Quaternion){vcosf(angle * 0.5f), s * axis.x, s * axis.y, s * axis.z}; }
void normalize_quaternion(Quaternion* q) {
    float mag = vsqrtf(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
    if (mag > 1e-6f) {q->w /= mag; q->x /= mag;  q->y /= mag; q->z /= mag; }
    else { q->w = 1.0f; q->x = q->y = q->z = 0.0f; }
}

Vector3 quat_rotate(Quaternion q, Vector3 v) {
    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;
    float xx2 = q.x * x2;
    float yy2 = q.y * y2;
    float zz2 = q.z * z2;
    float xy2 = q.x * y2;
    float xz2 = q.x * z2;
    float yz2 = q.y * z2;
    float wx2 = q.w * x2;
    float wy2 = q.w * y2;
    float wz2 = q.w * z2;
    return (Vector3){
        v.x * (1.0f - yy2 - zz2) + v.y * (xy2 - wz2) + v.z * (xz2 + wy2),
        v.x * (xy2 + wz2) + v.y * (1.0f - xx2 - zz2) + v.z * (yz2 - wx2),
        v.x * (xz2 - wy2) + v.y * (yz2 + wx2) + v.z * (1.0f - xx2 - yy2)
    };
}

void SetUpdatedMatrix(float *mat, float posx, float posy, float posz, Quaternion* quat, float sclx, float scly, float sclz) {
    float rot[16];
    quat_to_matrix(quat,rot);
    mat[0]  = rot[0] * -sclx; mat[1]  = rot[1] * -sclx; mat[2]  = rot[2] * -sclx; mat[3]  = 0.0f; // Necessary -x for blender right to left handed coordinate conversion.
    mat[4]  = rot[4] * scly; mat[5]  = rot[5] * scly; mat[6]  = rot[6] * scly; mat[7]  = 0.0f;
    mat[8]  = rot[8] * sclz; mat[9]  = rot[9] * sclz; mat[10] = rot[10] * sclz; mat[11] = 0.0f;
    mat[12] = posx;          mat[13] = posy;          mat[14] = posz;          mat[15] = 1.0f;
}

void UpdateInstanceMatrix(int32_t i) {
    if (instances[i].modelIndex >= loadedModelsMaxIndex) { dirtyInstances[i] = false; return; } // No model
    if (modelVertexCounts[instances[i].modelIndex] < 1) { dirtyInstances[i] = false; return; } // Empty model

    float mat[16]; // 4x4 matrix
    Quaternion quat = {instances[i].rotation.x, instances[i].rotation.y, instances[i].rotation.z, instances[i].rotation.w};
    SetUpdatedMatrix(mat, instances[i].position.x, instances[i].position.y, instances[i].position.z, &quat,instances[i].scale.x, instances[i].scale.y, instances[i].scale.z);
    memcpy(&modelMatrices[i * 16], mat, 16 * sizeof(float));
    dirtyInstances[i] = false;
}
