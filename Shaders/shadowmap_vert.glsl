// Shadowmap Vertex shader
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(std430, binding = 11) buffer ModelMatrices { mat4 modelMatrices[]; };

layout(location = 0) uniform uint instanceIndex;
layout(location = 1) uniform mat4 viewProjMatrix;

out vec3 FragPos;
out vec2 TexCoord;

void main() {
    mat4 matrix = modelMatrices[instanceIndex];
    vec4 matmul = matrix * vec4(position, 1.0);
    FragPos = vec3(matmul); // World-space position
    gl_Position = viewProjMatrix * matmul;
    TexCoord = aTexCoord;
}
