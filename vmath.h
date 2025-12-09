#pragma once
#define vabs(x) ((x) < 0 ? -(x) : (x))
#define vmin(a,b) ((a) < (b) ? (a) : (b))
#define vmax(a,b) ((a) > (b) ? (a) : (b))
#define PI 3.14159265f
#define TAU 6.2831853f
static inline float vfloor(float x) { int i = (int)x; return (float)(i > x ? i - 1 : i); }
static inline float vceil(float x) { int i = (int)x; return (float)(i < x ? i + 1 : i); }
static inline float vsqrtf(float x) { union { float f; unsigned int i; } u = { x }; u.i = 0x1fbd1df5 + (u.i >> 1); return 0.5f * (u.f + x / u.f); }
static inline float vsinf(float x) { x -= TAU * vfloor(x / TAU); if (x > PI) { x -= TAU; } float s = (4/PI)*x - (4/(PI*PI))*x*vabs(x); return 0.225f*(s*vabs(s) - s) + s; }
static inline float vcosf(float x) { return vsinf(x + 1.57079632f); }
static inline float vtan(float x) { return vsinf(x) / vcosf(x); }
static inline float vcot(float x) { float x2 = x * x; float t = x + (x2 * x) * 0.33333333f; return 1.0f / t; }
static inline float deg2rad(float degrees) { return degrees * (PI / 180.0f); }
static inline float rad2deg(float radians) { return radians * (180.0f / PI); }
static inline float vlog2f(float x) {
    union { float f; unsigned int i; } v = { x };
    int e = (int)((v.i >> 23) & 255) - 127;
    v.i = (v.i & 0x7FFFFF) | 0x3F800000;   // normalize mantissa to [1,2)
    float m = v.f;
    float p = m - 1.0f;
    float log2m = p * (1.3465558f + p * (-0.33942322f + p * 0.028794660f)); // polynomial approximation of log2(m)
    return (float)e + log2m;
}

static inline float vlog(float x) { return vlog2f(x) * 0.69314718f; }
static inline float vexp2f(float x) {
    float ip = vfloor(x);
    float fp = x - ip;
    float p = 1.0f + fp * (0.69314718f + fp * (0.24022651f + fp * 0.05550411f)); // poly approximation for 2^fp on [0,1]
    int ei = (int)ip + 127;
    unsigned int bits = (unsigned int)(ei << 23);
    union { unsigned int i; float f; } u = { bits };
    return u.f * p;
}

static inline float vexp(float x) { return vexp2f(x * 1.4426950409f); } // 1/ln(2)
static inline float vpow(float a, float b) { return vexp(b * vlog(a)); }
