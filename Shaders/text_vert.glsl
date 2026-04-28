// Text shader
#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
const mat4 projection = mat4(
    2.0/1366.0, 0.0,         0.0,  0.0,
    0.0,       -2.0/768.0,   0.0,  0.0,
    0.0,        0.0,        -1.0,  0.0,
   -1.0,        1.0,         0.0,  1.0
);
out vec2 TexCoord;

void main() {
    gl_Position = projection * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
