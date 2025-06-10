#ifndef QUATERNION_H
#define QUATERNION_H

// Quaternion structure
typedef struct {
    float w, x, y, z;
} Quaternion;

// Initialize a quaternion (identity: no rotation)
void quat_identity(Quaternion* q);

// Normalize a quaternion
void quat_normalize(Quaternion* q);

// Multiply two quaternions: q = q1 * q2
void quat_multiply(Quaternion* result, Quaternion* q1, Quaternion* q2);

// Create a quaternion from an axis and angle (in radians)
void quat_from_axis_angle(Quaternion* q, float x, float y, float z, float angle);

// Convert quaternion to a 4x4 matrix
void quat_to_matrix(Quaternion* q, float* m);

void quat_to_euler(Quaternion* q, float* yaw, float* pitch, float* roll);

#endif // QUATERNION_H
