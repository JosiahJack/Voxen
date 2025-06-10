#include <math.h>
#include "constants.h"
#include "matrix.h"
#include "quaternion.h"

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
    if (len > 0.0f) {
        q->w /= len;
        q->x /= len;
        q->y /= len;
        q->z /= len;
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
    float half_angle = angle * 0.5f;
    float s = sin(half_angle);
    q->w = cos(half_angle);
    q->x = x * s;
    q->y = y * s;
    q->z = z * s;
    quat_normalize(q);
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
    m[0] = 1.0f - 2.0f * (yy + zz);  // Right X
    m[1] = 2.0f * (xy + zw);          // Right Y
    m[2] = 2.0f * (xz - yw);          // Right Z
    m[4] = 2.0f * (xy - zw);          // Up X
    m[5] = 1.0f - 2.0f * (xx + zz);  // Up Y
    m[6] = 2.0f * (yz + xw);          // Up Z
    m[8] = 2.0f * (xz + yw);          // Forward X
    m[9] = 2.0f * (yz - xw);          // Forward Y
    m[10] = 1.0f - 2.0f * (xx + yy); // Forward Z
    m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}

void quat_to_euler(Quaternion* q, float* yaw, float* pitch, float* roll) {
    // Pitch (X-axis rotation)
    float sinp = 2.0f * (q->w * q->x + q->y * q->z);
    float cosp = 1.0f - 2.0f * (q->x * q->x + q->y * q->y);
    *pitch = atan2(sinp, cosp) * 180.0f / M_PI;

    // Yaw (Z-axis rotation)
    float siny = 2.0f * (q->w * q->z + q->x * q->y);
    float cosy = 1.0f - 2.0f * (q->y * q->y + q->z * q->z);
    *yaw = atan2(siny, cosy) * 180.0f / M_PI;

    // Roll (Y-axis rotation, not used here)
    float sinr = 2.0f * (q->w * q->y - q->z * q->x);
    *roll = asin(sinr) * 180.0f / M_PI;
}
