#ifndef VOXEN_QUATERNION_H
#define VOXEN_QUATERNION_H

// Quaternion structure
typedef struct {
    float w, x, y, z;
} Quaternion;

float deg2rad(float degrees); // Not in math.h, who knew!
float rad2deg(float radians);

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

void quat_from_yaw_pitch(Quaternion* q, float yaw_deg, float pitch_deg);
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg);

#endif // VOXEN_QUATERNION_H
