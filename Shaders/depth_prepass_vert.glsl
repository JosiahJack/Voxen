// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(std430, binding = 11) buffer ModelMatrices { mat4 modelMatrices[]; };

layout(location = 0) uniform uint instanceIndex;
layout(location = 2) uniform mat4 viewProjection;

void main() {
    mat4 matrix = modelMatrices[instanceIndex];
    vec4 worldPos = matrix * vec4(aPos, 1.0);
    gl_Position = viewProjection * worldPos;
}
