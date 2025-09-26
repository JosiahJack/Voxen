// Shadowmap Fragment Shader
#version 430 core
in vec3 WorldPos;
uniform vec3 lightPos;

layout(location = 0) out float outDepth; // Single-channel output

void main() {
    float dist = length(WorldPos - lightPos); // Linear distance in world space
    outDepth = dist; // Output linear depth
}
