// debugunlit_frag.glsl
#version 430 core
out vec4 FragColor;

uniform vec3 u_Color = vec3(0.0, 1.0, 0.0); // default bright green

void main() {
    FragColor = vec4(u_Color, 1.0);
}
