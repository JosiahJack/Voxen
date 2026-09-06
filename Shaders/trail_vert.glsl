// trail_vert.glsl - Camera-facing ribbon trail vertex shader (welded shared edges)
layout(location = 0) in vec2 inCorner;
layout(location = 1) in vec2 inUV;
layout(location = 0) uniform mat4 uViewProj;
struct TrailInstance { vec4 p0; vec4 p1; vec4 c00; vec4 c01; vec4 c10; vec4 c11; uvec4 data; vec4 vtxExtra; };
layout(std430, binding = 11) readonly buffer TrailInstanceBlock { TrailInstance instances[4096]; };
out vec2 vUV; out vec4 vColor; flat out uint vTexIndex;
vec4 unpackColorRGBA8(uint packedColor) { return vec4(float((packedColor >>  0) & 0xFFu) / 255.0,float((packedColor >>  8) & 0xFFu) / 255.0,float((packedColor >> 16) & 0xFFu) / 255.0,float((packedColor >> 24) & 0xFFu) / 255.0);}
void main() {
    int i = gl_InstanceID; float along = inCorner.x, side = inCorner.y; TrailInstance it = instances[i];
    vec4 corn = (along < 0.0) ? ((side < 0.0) ? it.c00 : it.c01) : ((side < 0.0) ? it.c10 : it.c11);
    float uv = along * 0.5 + 0.5;
    gl_Position = uViewProj * vec4(corn.xyz, 1.0); vUV = vec2(uv, side * 0.5 + 0.5); vColor = mix(unpackColorRGBA8(it.data.x), unpackColorRGBA8(it.data.y), uv); vTexIndex = it.data.z & 0xFFFFu;
}