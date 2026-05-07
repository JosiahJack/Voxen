// debugunlit_vert.glsl
#version 430 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec4 aColor;
layout(location=0) uniform mat4 u_ViewProj;
out vec4 v_Color;
void main(){ v_Color=aColor; gl_Position=u_ViewProj*vec4(aPos,1.0); }
