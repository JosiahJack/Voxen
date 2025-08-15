#ifndef VOXEN_QUATERNION_H
#define VOXEN_QUATERNION_H

typedef struct {
    float w, x, y, z;
} Quaternion;

float deg2rad(float degrees);
float rad2deg(float radians);
void quat_identity(Quaternion* q);
void quat_normalize(Quaternion* q);
void quat_multiply(Quaternion* result, Quaternion* q1, Quaternion* q2);
void quat_from_axis_angle(Quaternion* q, float x, float y, float z, float angle);
void quat_to_euler(Quaternion* q, float* yaw, float* pitch, float* roll);
void mat4_identity(float* m);
void quat_to_matrix(Quaternion* q, float* m); // Convert quaternion to a 4x4 matrix
void quat_from_yaw_pitch(Quaternion* q, float yaw_deg, float pitch_deg);
void quat_from_yaw_pitch_roll(Quaternion* q, float yaw_deg, float pitch_deg, float roll_deg);
void normalize_vector(float* x, float* y, float* z);
void mat4_perspective(float* m, float fov, float aspect, float near, float far);
void mat4_lookat(float* m, float eyeX, float eyeY, float eyeZ, Quaternion* orientation);
void mat4_ortho(float* m, float left, float right, float bottom, float top, float near, float far);
void mat4_inverse(float *out, float *m);
void SetUpdatedMatrix(float *mat, float posx, float posy, float posz, float rotx, float roty, float rotz, float rotw, float sclx, float scly, float sclz);
void UpdateInstanceMatrix(int i);
#endif // VOXEN_QUATERNION_H
