// Shadowmap Vertex shader
layout(location=0) in vec3 position;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(std430,binding=1) buffer ModelMatrices { mat4 modelMatrices[]; };
layout(location=0) uniform uint instanceIndex;
layout(location=1) uniform mat4 viewProjMatrix;
out vec3 FragPos;
out vec2 TexCoord;
void main() {
    vec4 matmul = modelMatrices[instanceIndex] * vec4(position, 1.0);
    FragPos = matmul.xyz; // World-space position
    gl_Position = viewProjMatrix * matmul;
    TexCoord = aTexCoord;
}
