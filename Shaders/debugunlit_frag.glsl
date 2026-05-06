// debugunlit_frag.glsl
#version 430 core
out vec4 FragColor;
layout(location = 1) uniform vec3 color;
void main(){FragColor=vec4(color,1.0);}
