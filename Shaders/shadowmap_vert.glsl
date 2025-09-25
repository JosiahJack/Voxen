// Shadowmap Vertex shader
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 0) uniform mat4 modelMatrix;
layout(location = 1) uniform mat4 viewProjMatrix;

void main() {
    gl_Position = viewProjMatrix * modelMatrix * vec4(position, 1.0);
}
