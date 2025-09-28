// Text shader
#version 450 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D textTexture; // GL_RED8 atlas
uniform vec4 textColor;
uniform vec2 texelSize;

void main() {
    float coverage = texture(textTexture, TexCoord).r;
    if (coverage < 0.6 && coverage > 0.4) { // If it's 0.5 with buffer for float imprecision
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // outline
    } else if (coverage > 0.0) {
        FragColor = vec4(textColor.rgb, coverage * textColor.a); // text
    } else {
        FragColor = vec4(0.0); // transparent
    }
}
