// ui_vert.glsl: Generic shader for unlit textured surfaces (all world geometry, items, enemies, doors, etc., without transparency for first pass prior to lighting.
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aTex;
const mat4 viewProjection = mat4(2.0/1366.0,0.0,0.0,0.0,  0.0,-2.0/768.0,0.0,0.0,  0.0,0.0,-1.0,0.0,  -1.0,1.0,0.0,1.0);
out vec2 TexCoord;
void main() { TexCoord = aTex.xy; gl_Position = viewProjection * vec4(aPos,1.0); }
