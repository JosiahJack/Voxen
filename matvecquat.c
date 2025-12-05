// matvecquat.c - Matrices, Vectors, Quaternions
#include <stdint.h>
#include <stdbool.h>
#include "matvecquat.h"
#include "vmath.h"
#include "entity.h"
extern float aspect3D;
extern float rasterPerspectiveProjection[16];
extern float shadowmapsPerspectiveProjection[16];

void normalize_vector(float* x, float* y, float* z) { float len = vsqrtf(*x * *x + *y * *y + *z * *z); if (len > 1e-6f) { *x /= len; *y /= len; *z /= len; } }
Vector3 add_vector3(Vector3 a, Vector3 b) { return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
Vector3 sub_vector3(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
Vector3 scale_vector3(Vector3 v, float s) { Vector3 res = {v.x * s, v.y * s, v.z * s}; return res; }
float magnitude_vector3(const Vector3 v) { return vsqrtf(dot_vector3(v, v)); }
Vector3 min_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x<b.x ? a.x : b.x, a.y<b.y ? a.y : b.y, a.z<b.z ? a.z : b.z }; }
Vector3 max_vector3(Vector3 a, Vector3 b) { return (Vector3){ a.x>b.x ? a.x : b.x, a.y>b.y ? a.y : b.y, a.z>b.z ? a.z : b.z }; }
float dot(float x1, float y1, float z1, float x2, float y2, float z2) { return x1*x2 + y1*y2 + z1*z2; }
float dot_vector3(Vector3 a, Vector3 b) { return dot(a.x,a.y,a.z, b.x,b.y,b.z); }
float dist_sq_vector3(Vector3 a, Vector3 b) { Vector3 d = sub_vector3(a, b); return dot_vector3(d, d); }
Vector3 cross_vector3(Vector3 a, Vector3 b) { return (Vector3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
float length_vector3(Vector3 v) { return vsqrtf(dot_vector3(v, v)); }
Vector3 normalize_vector3(Vector3 v) { float len = length_vector3(v); return len > 0.000001f ? (Vector3){v.x / len, v.y / len, v.z / len} : v; }
Vector3 mul_mat4_vector3(const float* m, Vector3 v) { return (Vector3){m[0]*v.x+m[4]*v.y+m[8]*v.z+m[12], m[1]*v.x+m[5]*v.y+m[9]*v.z+m[13], m[2]*v.x+m[6]*v.y+m[10]*v.z+m[14]}; }
float squareDistance2D(float x1, float z1, float x2, float z2) { float dx = x2 - x1; float dz = z2 - z1; return dx * dx + dz * dz; }
float squareDistance3D(float x1, float y1, float z1, float x2, float y2, float z2) { float dx = x2 - x1; float dy = y2 - y1; float dz = z2 - z1; return dx * dx + dy * dy + dz * dz; }

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
    m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = m[15] = 1.0f;
}

Quaternion conjugate_quaternion(const Quaternion q) { return (Quaternion){q.w, -q.x, -q.y, -q.z}; }
Quaternion axis_angle_quaternion(const Vector3 axis, float angle) { float s = vsinf(angle * 0.5f); return (Quaternion){vcosf(angle * 0.5f), s * axis.x, s * axis.y, s * axis.z}; }
void normalize_quaternion(Quaternion* q) {
    float mag = vsqrtf(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
    if (mag > 1e-6f) {q->w /= mag; q->x /= mag;  q->y /= mag; q->z /= mag; }
    else { q->w = 1.0f; q->x = q->y = q->z = 0.0f; }
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
    Quaternion q_conj = conjugate_quaternion(q);
    Quaternion temp = mul_quaternion(q, vq);
    Quaternion result = mul_quaternion(temp, q_conj);
    return (Vector3){result.x, result.y, result.z};
}

void mul_mat4(float *out, const float *a, const float *b) { // out = a * b
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

void SetUpdatedMatrix(float *mat, float posx, float posy, float posz, Quaternion* quat, float sclx, float scly, float sclz) {
    float rot[16];
    quat_to_matrix(quat,rot);
    mat[0]  = rot[0] * -sclx; mat[1]  = rot[1] * -sclx; mat[2]  = rot[2] * -sclx; mat[3]  = 0.0f;
    mat[4]  = rot[4] * scly; mat[5]  = rot[5] * scly; mat[6]  = rot[6] * scly; mat[7]  = 0.0f;
    mat[8]  = rot[8] * sclz; mat[9]  = rot[9] * sclz; mat[10] = rot[10] * sclz; mat[11] = 0.0f;
    mat[12] = posx;          mat[13] = posy;          mat[14] = posz;          mat[15] = 1.0f;
}

void UpdateInstanceMatrix(int32_t i) {
    if (instances[i].modelIndex >= loadedModels) { dirtyInstances[i] = false; return; } // No model
    if (modelVertexCounts[instances[i].modelIndex] < 1) { dirtyInstances[i] = false; return; } // Empty model

    float mat[16]; // 4x4 matrix
    Quaternion quat = {instances[i].rotation.x, instances[i].rotation.y, instances[i].rotation.z, instances[i].rotation.w};
    SetUpdatedMatrix(mat, instances[i].position.x, instances[i].position.y, instances[i].position.z, &quat,instances[i].scale.x, instances[i].scale.y, instances[i].scale.z);
    memcpy(&modelMatrices[i * 16], mat, 16 * sizeof(float));
    dirtyInstances[i] = false;
}

void UpdateProjectionMatrices(void) {
    float* m;
    m = uiOrthoProjection;
    m[0] = 2.0f / (float)screen_width; m[1] =                           0.0f; m[2] =  0.0f; m[3] = 0.0f;
    m[4] =                       0.0f; m[5] = -2.0f / ((float)screen_height); m[6] =  0.0f; m[7] = 0.0f;
    m[8] =                       0.0f; m[9] =                           0.0f; m[10]= -1.0f; m[11]= 0.0f;
    m[12]=                      -1.0f; m[13]=                           1.0f; m[14]=  0.0f; m[15]= 1.0f;
    
    aspect3D = (float)screen_width / (float)screen_height;
    float f = vcot(settings_FOV * PI / 360.0f);
    m = rasterPerspectiveProjection;
    m[0] = f / aspect3D; m[1] = 0.0f; m[2] =                                                      0.0f; m[3] =  0.0f;
    m[4] =         0.0f; m[5] =    f; m[6] =                                                      0.0f; m[7] =  0.0f;
    m[8] =         0.0f; m[9] = 0.0f; m[10]=      -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE); m[11]= -1.0f;
    m[12]=         0.0f; m[13]= 0.0f; m[14]= -2.0f * FAR_PLANE * NEAR_PLANE / (FAR_PLANE - NEAR_PLANE); m[15]=  0.0f;
    
    float aspectShad = (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE;
    f = 1.0f / vtan(SHADOWMAP_FOV * PI / 360.0f); // vcot introduces skewness causing false "Peter-Panning" from bubble distortion of the shadowmap depths.  Just stick with recip tangent.
    m = shadowmapsPerspectiveProjection;
    m[0] = f / aspectShad; m[1] = 0.0f; m[2] =                                            0.0f; m[3] =  0.0f;
    m[4] =           0.0f; m[5] =    f; m[6] =                                            0.0f; m[7] =  0.0f;
    m[8] =           0.0f; m[9] = 0.0f; m[10]=      -(35.0 + NEAR_PLANE) / (35.0 - NEAR_PLANE); m[11]= -1.0f;
    m[12]=           0.0f; m[13]= 0.0f; m[14]= -2.0f * 35.0 * NEAR_PLANE / (35.0 - NEAR_PLANE); m[15]=  0.0f;
}
