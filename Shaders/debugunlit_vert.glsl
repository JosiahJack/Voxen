// debugunlit_vert.glsl
#version 430 core
layout(location=0) in vec3 aPos;
layout(location=0) uniform mat4 u_ViewProj;
void main(){gl_Position=u_ViewProj*vec4(aPos,1.0);}
