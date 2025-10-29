// Text shader
#version 430 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D textTexture;  // GL_R8 SDF atlas
uniform uint fontType;              // 0 = SystemShockText, 1 = StopD
uniform vec4 textColor;
uniform vec2 texelSize;         // (1.0 / atlasWidth, 1.0 / atlasHeight)

void main() {
    float sdf = texture(textTexture, TexCoord).r;

    if (sdf >= 0.8) { FragColor = vec4(textColor.rgb,1.0); return; } // Center of glyph (fully filled)

    // Outside glyph: fully transparent
    if (sdf <= 0.00001) {
        FragColor = vec4(0.0);
        return;
    }

    // We're in the border region: 0.0 < sdf < 0.5
    // Sample neighbors to detect edge direction
    float sdfRight  = texture(textTexture, TexCoord + vec2(texelSize.x, 0.0)).r;
    float sdfLeft   = texture(textTexture, TexCoord + vec2(-texelSize.x, 0.0)).r;
    float sdfUp     = texture(textTexture, TexCoord + vec2(0.0, texelSize.y)).r;
    float sdfDown   = texture(textTexture, TexCoord + vec2(0.0, -texelSize.y)).r;

    // Compute approximate gradient direction using central differences
    vec2 grad = vec2(sdfRight - sdfLeft, sdfUp - sdfDown);
    float gradLen = length(grad);
    if (gradLen < 0.001) discard; // No strong direction — fallback or ignore

    vec2 gradNorm = grad / gradLen; // Normalize gradient

    // Determine if on a vertical edge
    float verticalComponent = gradNorm.y; // +1 = pointing up (bottom edge), -1 = pointing down (top edge)
    if (fontType == 1) { // StopD: directional colored borders
        if (abs(verticalComponent) > 0.1) { // Only strong vertical edges
            if (verticalComponent > 0.0) {
                // Top edge: 20% lighter
                vec3 lightColor = mix(textColor.rgb, vec3(1.0), 0.07);
                FragColor = vec4(lightColor, 1.0);
            } else {
                // Bottom edge: 50% darker
                vec3 darkColor = textColor.rgb * 0.5;
                FragColor = vec4(darkColor, 0.5);
            }
        } else {
            // Side edges or weak direction: no border
            FragColor = vec4(textColor.rgb, 1.0);
        }
    } else { // font == 0: original black outline
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
