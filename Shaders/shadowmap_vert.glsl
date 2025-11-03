// Shadowmap Vertex shader
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 0) uniform mat4 modelMatrix;
layout(location = 1) uniform mat4 viewProjMatrix;
layout(location = 2) uniform vec3 lightPos;

out vec3 FragPos;

void main() {
    vec4 matmul = modelMatrix * vec4(position, 1.0);
    FragPos = vec3(matmul); // World-space position
    gl_Position = viewProjMatrix * matmul;
}
