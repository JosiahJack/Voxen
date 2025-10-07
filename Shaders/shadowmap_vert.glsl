// Shadowmap Vertex shader
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 0) uniform mat4 modelMatrix;
layout(location = 1) uniform mat4 viewProjMatrix;
layout(location = 2) uniform vec3 lightPos;

out vec3 FragPos;

void main() {
    FragPos = vec3(modelMatrix * vec4(position, 1.0)); // World-space position
    gl_Position = viewProjMatrix * modelMatrix * vec4(position, 1.0);
}
