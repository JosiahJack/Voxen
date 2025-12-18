// Shadowmap Vertex shader
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(std430, binding = 11) buffer ModelMatrices { mat4 modelMatrices[]; };

layout(location = 0) uniform uint instanceIndex;
layout(location = 1) uniform mat4 viewProjMatrix;
// layout(location = 2) uniform uint face; // Start fragment uniforms
// layout(location = 3) uniform uint lightIndex;
// layout(location = 4) uniform uint shadowmapSize;
// layout(location = 5) uniform uint shadowmapIndirection;

out vec3 FragPos;
out vec2 TexCoord;

void main() {
    mat4 matrix = modelMatrices[instanceIndex];
    vec4 matmul = matrix * vec4(position, 1.0);
    FragPos = vec3(matmul); // World-space position
    TexCoord = aTexCoord;
    gl_Position = viewProjMatrix * matmul;
}
