// trail_vert.glsl - Camera-facing ribbon trail vertex shader
layout(location = 0) in vec2 inCorner;
layout(location = 1) in vec2 inUV;
layout(location = 0) uniform mat4 uViewProj;
layout(location = 1) uniform vec3 uCamPos;
layout(location = 2) uniform vec3 uCamRight;
layout(location = 3) uniform vec3 uCamUp;
layout(std430, binding = 11) readonly buffer TrailInstanceBlock { vec4 p0Width[4096]; vec4 p1Width[4096]; uvec4 data[4096]; };
out vec2 vUV; out vec4 vColor;
vec4 unpackColorRGBA8(uint packedColor) { return vec4(float((packedColor >>  0) & 0xFFu) / 255.0,float((packedColor >>  8) & 0xFFu) / 255.0,float((packedColor >> 16) & 0xFFu) / 255.0,float((packedColor >> 24) & 0xFFu) / 255.0);}
void main() {
    int i = gl_InstanceID; vec4 p0=p0Width[i], p1=p1Width[i]; float along=inCorner.x, side=inCorner.y; vec3 center = mix(p0.xyz, p1.xyz, along); float width = mix(p0.w, p1.w, along); 
    vec3 segmentDir = normalize(p1.xyz - p0.xyz); vec3 viewDir = normalize(uCamPos - center); vec3 sideVec = cross(segmentDir, viewDir); float lenSq = dot(sideVec, sideVec); if (lenSq < 1e-8) { sideVec = uCamRight; } else { sideVec = normalize(sideVec); }
    vec3 worldPos = center + sideVec * (side * width); gl_Position = uViewProj * vec4(worldPos, 1.0); vUV = vec2(along, side * 0.5 + 0.5); uvec4 d = data[i]; vColor = mix(unpackColorRGBA8(d.x), unpackColorRGBA8(d.y), along);
}
