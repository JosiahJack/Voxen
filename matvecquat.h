// matvecquat.h - Math, Vectors, Quaternions
#pragma once
#define PI 3.14159265f
#define TAU 6.2831853f
#define INV_TAU (1.0f / TAU) // Precomputed for fast multiplication
#define INV_PI  (1.0f / PI)
#define vabs(x) ((x) < 0 ? -(x) : (x))
#define vmin(a,b) ((a) < (b) ? (a) : (b))
#define vmax(a,b) ((a) > (b) ? (a) : (b))
#define vclamp(x,a,b) vmin(vmax(x,a),b)
#define vsqrtf(x) __builtin_sqrtf(x)
#define NEAR_PLANE (0.02f)
INLINE float vinvsqtf(float v) { __m128 x = (__m128){v,0.0f,0.0f,0.0f}; __m128 r=__builtin_ia32_rsqrtss(x); float y = r[0]; float x2 = v * 0.5f; return y * (1.5f - x2 * y * y); }
INLINE float vfloor(float x) { return __builtin_floorf(x); }
INLINE float vceil(float x) { return __builtin_ceilf(x); }
INLINE float vsinf(float x) { x -= TAU * vfloor(x * INV_TAU); if (x > PI) { x -= TAU; } float s = (4/PI)*x - (4/(PI*PI))*x*vabs(x); return 0.225f*(s*vabs(s) - s) + s; }
INLINE float vcosf(float x) { return vsinf(x + 1.57079632f); }
INLINE float vacosf(float x) { float negate = (x < 0.0f) ? 1.0f : 0.0f; x=vabs(x); float ret=(-0.0187293f*x + 0.0742610f)*x - 0.2121144f; ret=(ret*x + 1.5707288f)*vsqrtf(1.0f - x); ret=ret - 2.0f*negate*ret; return negate*PI + (1.0f - 2.0f*negate) * ret; }
INLINE float vtan(float x) { return vsinf(x) / vcosf(x); }
INLINE float vcot(float x) { float x2 = x * x; float t = x + (x2 * x) * 0.33333333f; return 1.0f / t; }
INLINE float deg2rad(float degrees) { return degrees * (PI / 180.0f); }
INLINE float vexp2f(float x){float ip=vfloor(x); float fp=x - ip; float p=1.0f + fp*(0.69314718f + fp*(0.24022651f + fp*0.05550411f)); /*poly approx 2^fp on [0,1]*/ int ei=(int)ip + 127; u32 bits=(u32)(ei << 23); union{u32 i; float f;}u={bits}; return u.f*p;}
INLINE float vexp(float x) { return vexp2f(x * 1.4426950409f); } // 1/ln(2)
INLINE i32 clamp(i32 val, i32 min, i32 max) { return (val > max) ? max : ((val < min) ? min : val); }
INLINE float vround(float val) { return (val >= 0.0f) ? (float)(int)(val + 0.5f) : (float)(int)(val - 0.5f); }
INLINE V3 V3_AplusB(V3 a, V3 b) { return (V3){a.x + b.x, a.y + b.y, a.z + b.z}; }
INLINE V3 V3_AsubB(V3 a, V3 b) { return (V3){a.x - b.x, a.y - b.y, a.z - b.z}; }
INLINE V3 V3_ScaleByF(V3 v, float s) { return (V3){v.x * s, v.y * s, v.z * s}; }
INLINE V3 mul_v3_v3_elementwise(V3 v, V3 w) { return (V3){v.x * w.x, v.y * w.y, v.z * w.z}; }
INLINE float dot(float x1, float y1, float z1, float x2, float y2, float z2) { return x1*x2 + y1*y2 + z1*z2; }
INLINE float V3_dot(V3 a, V3 b) { return dot(a.x,a.y,a.z, b.x,b.y,b.z); }
INLINE float quat_dot(Quaternion a, Quaternion b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
INLINE float V3_Mag(const V3 v) { return vsqrtf(V3_dot(v,v)); }
INLINE float V3_SqDist(V3 a, V3 b) { V3 d = V3_AsubB(a,b); return V3_dot(d,d); }
INLINE float V3_Dist(V3 a, V3 b) { return V3_Mag(V3_AsubB(a,b)); }
INLINE V3 V3_Cross(V3 a, V3 b) { return (V3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
INLINE V3 V3_Normalize(V3 v) { float len_sq = V3_dot(v,v); if (len_sq < 0.000001f){return v;} float inv_len = vinvsqtf(len_sq); return (V3){v.x * inv_len, v.y * inv_len, v.z * inv_len}; }
INLINE Quaternion quat_multiply(Quaternion q1, Quaternion q2){float aw=q1.w,ax=q1.x,ay=q1.y,az=q1.z,bw=q2.w,bx=q2.x,by=q2.y,bz=q2.z; return (Quaternion){aw*bx+ax*bw+ay*bz-az*by,aw*by-ax*bz+ay*bw+az*bx,aw*bz+ax*by-ay*bx+az*bw,aw*bw-ax*bx-ay*by-az*bz};}
INLINE V3 quat_rot_v3(Quaternion q, V3 v) {float x=q.x,y=q.y,z=q.z,w=q.w; float vx=v.x,vy=v.y,vz=v.z; float tx=2.0f*(y*vz-z*vy); float ty=2.0f*(z*vx-x*vz); float tz=2.0f*(x*vy-y*vx); return (V3){vx+w*tx+(y*tz-z*ty),vy+w*ty+(z*tx-x*tz),vz+w*tz+(x*ty-y*tx)};}
INLINE u8 hardware14fromConstdex(u16 c) { return clamp(c - 21,0,14); }
