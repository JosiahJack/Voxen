// Text shader
#version 430 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D textTexture; // GL_RED8 atlas
uniform vec4 textColor;
uniform vec2 texelSize;

void main() {
    float sdf = texture(textTexture, TexCoord).r;
    if (sdf >= 0.9) {
        FragColor = vec4(textColor.rgb, 1.0);
    } else if (sdf >= 0.05) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
