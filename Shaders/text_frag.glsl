// Text shader
#version 430 core
in vec2 TexCoord;
out vec4 FragColor;

layout(location = 0) uniform mat4 projection; // vert shader uniform only
layout(location = 1) uniform sampler2D textTexture;  // GL_R8 SDF atlas
layout(location = 2) uniform uint fontType; // 0 = Josiah's handpainted SystemShockText, 1 = StopD
layout(location = 3) uniform vec4 textColor;
layout(location = 4) uniform vec2 texelSize;

void main() {
    float sdf = texture(textTexture, TexCoord).r;
    if (fontType == 1 && sdf >= 0.99) { FragColor = vec4(textColor.rgb,1.0); return; }
    if (fontType == 0 && sdf >= 0.8) { FragColor = vec4(textColor.rgb,1.0); return; } // Center of character, solid color
    if (sdf <= 0.00001) { FragColor = vec4(0.0); return; }           // Outside character, fully transparent

    // We're in the border region: 0.0 < sdf < 0.5
    // Sample neighbors to detect edge direction
    float sdfRight  = texture(textTexture, TexCoord + vec2(texelSize.x, 0.0)).r;
    float sdfLeft   = texture(textTexture, TexCoord + vec2(-texelSize.x, 0.0)).r;
    float sdfUp     = texture(textTexture, TexCoord + vec2(0.0, texelSize.y)).r;
    float sdfDown   = texture(textTexture, TexCoord + vec2(0.0, -texelSize.y)).r;
    vec2 grad = vec2(sdfRight - sdfLeft, sdfUp - sdfDown);
    float gradLen = length(grad);
    if (gradLen < 0.001) discard;

    vec2 gradNorm = grad / gradLen;
    float verticalComponent = gradNorm.y; // +1 = pointing up (bottom edge), -1 = pointing down (top edge)
    vec2 screenTexSize = fwidth(TexCoord);
    float screenPxRange = max(0.5 * length(screenTexSize) * 2.0, 0.0001);
    float softness     = screenPxRange * 5.0;

    float opacity = smoothstep(0.5 - softness, 0.5 + softness, sdf);
    if (fontType == 1) { // StopD: directional colored borders + softened edges
        float sdfRight = texture(textTexture, TexCoord + vec2(texelSize.x, 0.0)).r;
        float sdfLeft  = texture(textTexture, TexCoord + vec2(-texelSize.x, 0.0)).r;
        float sdfUp    = texture(textTexture, TexCoord + vec2(0.0, texelSize.y)).r;
        float sdfDown  = texture(textTexture, TexCoord + vec2(0.0, -texelSize.y)).r;
        vec2 grad     = vec2(sdfRight - sdfLeft, sdfUp - sdfDown);
        float gradLen = length(grad);
        vec3 finalColor = textColor.rgb;
        float finalAlpha = opacity;
        if (gradLen > 0.02) {
            vec2 gradNorm = grad / gradLen;
            float vertical = gradNorm.y;
            if (abs(vertical) > 0.15) {
                if (vertical > 0.0) {
                    finalColor = mix(textColor.rgb, vec3(1.0), 0.12);
                    finalAlpha *= 0.85;
                } else {
                    finalColor = textColor.rgb * 0.55;
                    finalAlpha *= 0.85;
                }
            }
        }

        FragColor = vec4(finalColor, finalAlpha);
    } else { // font == 0: original black outline
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
