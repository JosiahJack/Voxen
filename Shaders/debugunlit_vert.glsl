// debugunlit_vert.glsl - Wireline Vertex Shader
// One shader for both wireframe debug lines and the door laser quads. Both are already-expanded geometry in
// world space, so there is no thickness uniform: the laser builder bakes width into the corner offsets.
layout(location=0) in vec3 aPos;
layout(location=1) in vec4 aColor;
layout(location=0) uniform mat4 u_ViewProj;
out vec4 v_Color;
void main(){ v_Color=aColor; gl_Position=u_ViewProj*vec4(aPos,1.0); }