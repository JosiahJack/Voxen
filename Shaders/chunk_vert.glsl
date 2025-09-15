// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec2 aTexCoordLightmap;

uniform uint texIndex;
uniform uint glowSpecIndex;
uniform uint normInstanceIndex;
uniform mat4 matrix;
uniform mat4 viewProjection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec2 TexCoordLightmap;

flat out uint TexIndex;
flat out uint GlowIndex;
flat out uint SpecIndex;
flat out uint NormalIndex;
flat out uint InstanceIndex;

void main() {
    FragPos = vec3(matrix * vec4(aPos, 1.0)); // Convert vertex from the model's local space into world space
    Normal = mat3(transpose(inverse(matrix))) * aNormal;
    TexCoord = aTexCoord; // Pass along data to each vertex, shared for whole tri's pixels.
    TexCoordLightmap = aTexCoordLightmap; // Lightmap UVs
    TexIndex = texIndex;
    GlowIndex = glowSpecIndex & 0xFFFFu;
    SpecIndex = (glowSpecIndex >> 16) & 0xFFFFu;
    NormalIndex = normInstanceIndex & 0xFFFFu;
    InstanceIndex = (normInstanceIndex >> 16) & 0xFFFFu;
    gl_Position = viewProjection * vec4(FragPos, 1.0);
}
