// particle_vert.glsl - Instanced particle billboard vertex shader
layout(location = 0) in vec2 inCorner;
layout(location = 1) in vec2 inUV;
layout(location = 0) uniform mat4 uViewProj;
layout(location = 1) uniform vec3 uCamPos;
layout(location = 2) uniform vec3 uCamRight;
layout(location = 3) uniform vec3 uCamUp;
layout(location = 4) uniform vec3 uCamForward;
layout(location = 5) uniform int uInstanceOffset;
struct ParticleInstance { vec4 posSize; uvec4 data; };
layout(std430, binding = 10) readonly buffer ParticleInstanceBlock { ParticleInstance instances[20480]; };
out vec2 vUV; out vec4 vColor; out float vViewDist; flat out uint vFlags; flat out uint vTexIndex; out float vSoftness;
vec4 unpackColorRGBA8(uint packedColor) {return vec4( float((packedColor >>  0) & 0xFFu) / 255.0,float((packedColor >>  8) & 0xFFu) / 255.0,float((packedColor >> 16) & 0xFFu) / 255.0,float((packedColor >> 24) & 0xFFu) / 255.0); }
void main() {
    int idx = gl_InstanceID + uInstanceOffset; vec4 pSize = instances[idx].posSize; float size = pSize.w; if (size <= 0.0) { gl_Position = vec4(0.0, 0.0, 0.0, 0.0); return; }
    uvec4 d = instances[idx].data; float angle = float(d.y&0xFFu) * (6.28318530718 / 255.0); float c=cos(angle), s=sin(angle); vec2 corner = inCorner; vec2 rotated = vec2(corner.x * c - corner.y * s,corner.x * s + corner.y * c);
    vec3 worldPos = pSize.xyz + uCamRight * (rotated.x * size) + uCamUp * (rotated.y * size); gl_Position = uViewProj * vec4(worldPos, 1.0); vUV = inUV; vColor = unpackColorRGBA8(d.x); vFlags = ((d.y >> 24) & 0xFFu); vTexIndex = ((d.y >> 8) & 0xFFFFu); vSoftness = float(d.z & 0xFFu) / 16.0; vViewDist = dot(worldPos - uCamPos, uCamForward);
}
