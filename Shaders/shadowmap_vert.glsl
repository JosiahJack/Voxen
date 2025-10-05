// Shadowmap Vertex shader
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 0) uniform mat4 modelMatrix;
layout(location = 1) uniform mat4 viewProjMatrix;
layout(location = 2) uniform vec3 lightPos;

uniform uint texIndex;
uniform uint glowSpecIndex;
uniform uint normInstanceIndex;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

flat out uint TexIndex;
flat out uint GlowIndex;
flat out uint SpecIndex;
flat out uint NormalIndex;
flat out uint InstanceIndex;

void main() {
    FragPos = vec3(modelMatrix * vec4(position, 1.0)); // World-space position
    Normal = mat3(transpose(inverse(modelMatrix))) * aNormal;
    TexCoord = aTexCoord; // Pass along data to each vertex, shared for whole tri's pixels.
    TexIndex = texIndex;
    GlowIndex = glowSpecIndex & 0xFFFFu;
    SpecIndex = (glowSpecIndex >> 16) & 0xFFFFu;
    NormalIndex = normInstanceIndex & 0xFFFFu;
    InstanceIndex = (normInstanceIndex >> 16) & 0xFFFFu;
    gl_Position = viewProjMatrix * modelMatrix * vec4(position, 1.0);
}
