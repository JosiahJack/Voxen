// Text shader
#version 450 core

in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D textTexture;
uniform vec4 textColor;
uniform vec2 texelSize;

void main() {
    vec4 sampled = texture(textTexture, TexCoord);
    bool isOutline = false;

    // Check if current pixel is transparent
    if (sampled.a == 0.0) {
        vec2 offsets[8] = vec2[](
            vec2(-1.0, -1.0), vec2(0.0, -1.0), vec2(1.0, -1.0),
            vec2(-1.0,  0.0),                  vec2(1.0,  0.0),
            vec2(-1.0,  1.0), vec2(0.0,  1.0), vec2(1.0,  1.0)
        );
        
        for (int i = 0; i < 8; i++) { // Sample 8 neighboring pixels
            vec4 neighbor = texture(textTexture, TexCoord + offsets[i] * texelSize);
            if (neighbor.a > 0.0) { isOutline = true; break; }
        }
    }

    if (isOutline) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black outline
    } else if (sampled.a > 0.0) {
        FragColor = vec4(textColor.rgb, sampled.a * textColor.a); // Text color
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0); // Transparent background
    }
}
