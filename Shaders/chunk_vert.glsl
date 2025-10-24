// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform uint texIndex;
uniform uint glowSpecIndex;
uniform uint normInstanceIndex;
uniform mat4 matrix;
uniform mat4 viewProjection;
uniform uint isUI;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
flat out uint TexIndex;
flat out uint GlowIndex;
flat out uint SpecIndex;
flat out uint NormalIndex;

void main() {
    TexIndex = texIndex;
    GlowIndex = glowSpecIndex & 0xFFFFu;
    SpecIndex = (glowSpecIndex >> 16);
    NormalIndex = normInstanceIndex;
    if (isUI > 0) {
        TexCoord = aNormal.xy; // uiImageVAO only has pos and uvs so uvs are at location 1 (2nd)
        FragPos = vec3(aPos);
        gl_Position = viewProjection * vec4(aPos, 1.0);
        Normal = vec3(0.0, 0.0, 1.0);
    } else {
        TexCoord = aTexCoord;
        vec4 worldPos = matrix * vec4(aPos, 1.0);
        FragPos = vec3(worldPos);
        gl_Position = viewProjection * worldPos;
        Normal = mat3(transpose(inverse(matrix))) * aNormal;
    }
}
