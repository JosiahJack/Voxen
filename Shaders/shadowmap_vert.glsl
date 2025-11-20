// Shadowmap Vertex shader
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(location = 3) uniform mat4 modelMatrix;
layout(location = 4) uniform mat4 viewProjMatrix;

out vec3 FragPos;

void main() {
    vec4 matmul = modelMatrix * vec4(position, 1.0);
    FragPos = vec3(matmul); // World-space position
    gl_Position = viewProjMatrix * matmul;
}
