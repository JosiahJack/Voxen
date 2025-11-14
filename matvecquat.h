#ifndef VOXEN_MATVECQUAT_H
#define VOXEN_MATVECQUAT_H
typedef struct { float x,y; } Vector2;
typedef struct { float x,y,z; } Vector3;
typedef struct { float x,y,z,w; } Quaternion;
void normalize_vector(float* x, float* y, float* z);
Vector3 add_vector3(Vector3 a, Vector3 b);
Vector3 sub_vector3(Vector3 a, Vector3 b);
Vector3 scale_vector3(Vector3 v, float s);
float magnitude_vector3(const Vector3 v);
Vector3 min_vector3(Vector3 a, Vector3 b);
Vector3 max_vector3(Vector3 a, Vector3 b);
float dot(float x1, float y1, float z1, float x2, float y2, float z2);
float dot_vector3(Vector3 a, Vector3 b);
float dist_sq_vector3(Vector3 a, Vector3 b);
Vector3 cross_vector3(Vector3 a, Vector3 b);
float length_vector3(Vector3 v);
Vector3 normalize_vector3(Vector3 v);
Vector3 mul_mat4_vector3(const float* mat, Vector3 v);
void quat_to_matrix(Quaternion* q, float* m);
Quaternion conjugate_quaternion(const Quaternion q);
Quaternion axis_angle_quaternion(const Vector3 axis, float angle);
void normalize_quaternion(Quaternion* q);
#endif // VOXEN_MATVECQUAT_H
