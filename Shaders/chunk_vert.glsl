// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(std430, binding = 11) buffer ModelMatrices { float modelMatrices[]; };

uniform uint instanceIndex;
uniform uint normInstanceIndex;
uniform mat4 viewProjection;
uniform uint isUI;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
flat out uint NormalIndex;

void main() {
    NormalIndex = normInstanceIndex;
    if (isUI > 0) {
        TexCoord = aNormal.xy; // uiImageVAO only has pos and uvs so uvs are at location 1 (2nd)
        FragPos = vec3(aPos);
        gl_Position = viewProjection * vec4(aPos, 1.0);
        Normal = vec3(0.0, 0.0, 1.0);
    } else {
        TexCoord = aTexCoord;
        uint matbase = instanceIndex * 16;
        mat4 matrix = mat4(modelMatrices[matbase + 0], modelMatrices[matbase + 1], modelMatrices[matbase + 2], modelMatrices[matbase + 3],
                           modelMatrices[matbase + 4], modelMatrices[matbase + 5], modelMatrices[matbase + 6], modelMatrices[matbase + 7],
                           modelMatrices[matbase + 8], modelMatrices[matbase + 9], modelMatrices[matbase +10], modelMatrices[matbase +11],
                           modelMatrices[matbase +12], modelMatrices[matbase +13], modelMatrices[matbase +14], modelMatrices[matbase +15]);
        vec4 worldPos = matrix * vec4(aPos, 1.0);
        FragPos = vec3(worldPos);
        gl_Position = viewProjection * worldPos;
        Normal = mat3(transpose(inverse(matrix))) * aNormal;
    }
}
