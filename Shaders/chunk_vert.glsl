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

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Barycentric;
flat out uint TexIndex;
flat out uint GlowIndex;
flat out uint SpecIndex;
flat out uint NormalIndex;

void main() {
    vec4 worldPos = matrix * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    gl_Position = viewProjection * worldPos;
    Normal = mat3(transpose(inverse(matrix))) * aNormal;
    TexCoord = aTexCoord; // Pass along data to each vertex, shared for whole tri's pixels.
    TexIndex = texIndex;
    GlowIndex = glowSpecIndex & 0xFFFFu;
    SpecIndex = (glowSpecIndex >> 16);
    NormalIndex = normInstanceIndex;

    Barycentric = vec3(0.0);
    if (gl_VertexID % 3 == 0) Barycentric.x = 1.0;
    else if (gl_VertexID % 3 == 1) Barycentric.y = 1.0;
    else Barycentric.z = 1.0;
}
