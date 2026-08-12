// depth_prepass_vert.glsl: vertex shader for depth prepass
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(std430,binding=1) buffer ModelMatrices { mat4 modelMatrices[]; };
layout(location=0) uniform uint instanceIndex;
layout(location=2) uniform mat4 viewProjection;
out vec3 FragPos;
out vec2 TexCoord;
void main() {
    TexCoord = aTexCoord;
    vec4 worldPos = modelMatrices[instanceIndex] * vec4(aPos,1.0);
    gl_Position = viewProjection * worldPos;
    FragPos = vec3(worldPos);
}
