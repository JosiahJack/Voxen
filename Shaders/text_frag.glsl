// Text shader
in vec2 TexCoord;
layout(location=0) out vec4 outUI; // GL_COLOR_ATTACHMENT0
layout(location = 1) uniform sampler2D textTexture;  // GL_R8 SDF atlas
layout(location = 2) uniform uint fontType; // 0 = Josiah's handpainted SystemShockText, 1 = StopD
layout(location = 3) uniform vec4 textColor;
layout(location = 4) uniform vec2 texelSize;
float DistanceInScreenPixelsFromSDFEdge(float sdf) {
    vec2 unitRange = texelSize * 4.0;
    vec2 screenPxRange = fwidth(TexCoord) / unitRange;
    float pxRange = max(screenPxRange.x, screenPxRange.y);
    return (sdf - 0.5) * pxRange; // positive = inside, negative = outside
}

void main() {
    float sdf = texture(textTexture,TexCoord).r;
    float dist = DistanceInScreenPixelsFromSDFEdge(sdf);
    float alpha = smoothstep(-0.5,0.5,dist);
    if (alpha < 0.01 && fontType == 1) { outUI = vec4(0.0); return; }

    vec3 color = textColor.rgb;
    if (fontType == 1) { // StopD – Soft directional bevel
        float sR = texture(textTexture,TexCoord + vec2(texelSize.x,0.0)).r;  // Sample 4 neighbors once
        float sL = texture(textTexture,TexCoord + vec2(-texelSize.x,0.0)).r;
        float sU = texture(textTexture,TexCoord + vec2(0.0,texelSize.y)).r;
        float sD = texture(textTexture,TexCoord + vec2(0.0,-texelSize.y)).r;
        vec2 grad = vec2(sR - sL, sU - sD);
        float gradLen = length(grad);
        if (gradLen > 0.015) {
            vec2 n = grad / gradLen;
            float vertical = n.y; // +1 = bottom edge (highlight), -1 = top edge (shadow)
            if (abs(vertical) > 0.12) {
                if (vertical > 0.0) { color = mix(color,vec3(1.0),0.18); alpha *= 0.92; } // soft highlight on top edges
                else { color *= 0.58; alpha *= 0.90; } // soft shadow on bottom edges
            }
        }

        outUI = vec4(color,alpha * textColor.a);
    } else {
        if (sdf >= 0.5) { outUI = vec4(color, textColor.a); return; }

        bool hasNearbyGlyph = false;
        for (float x = -4.5; x <= 4.5; x += 1.0) {
            for (float y = -5.5; y <= 5.5; y += 1.0) {
                vec2 offset = vec2(x, y) * texelSize;
                float sampleSDF = texture(textTexture, TexCoord + offset).r;
                if (sampleSDF >= 0.5) { hasNearbyGlyph = true; break; }
            }
            if (hasNearbyGlyph) break;
        }

        outUI = vec4(0.0,0.0,0.0,hasNearbyGlyph ? 1.0 : 0.0);
    }
}
