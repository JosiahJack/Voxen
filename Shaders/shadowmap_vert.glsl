// Shadowmap Vertex shader
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 0) uniform mat4 modelMatrix;
layout(location = 1) uniform mat4 viewProjMatrix;
layout(location = 2) uniform vec3 lightPos;

uniform uint texIndex;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

flat out uint TexIndex;

void main() {
    FragPos = vec3(modelMatrix * vec4(position, 1.0)); // World-space position
    Normal = mat3(transpose(inverse(modelMatrix))) * aNormal;
    TexCoord = aTexCoord; // Pass along data to each vertex, shared for whole tri's pixels.
    TexIndex = texIndex;
    gl_Position = viewProjMatrix * modelMatrix * vec4(position, 1.0);
}
