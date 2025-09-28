// Text shader
#version 450 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D textTexture; // GL_RED8 atlas
uniform vec4 textColor;
uniform vec2 texelSize;

void main() {
    float sdf = texture(textTexture, TexCoord).r;
    float alpha = smoothstep(0.5-0.1, 0.5+0.1, sdf); // 0.1 controls smoothness
    FragColor = vec4(textColor.rgb, alpha * textColor.a);
}
