// Text shader
#version 430 core
in vec2 TexCoord;
out vec4 FragColor;

layout(location = 0) uniform mat4 projection; // vert shader uniform only
layout(location = 1) uniform sampler2D textTexture;  // GL_R8 SDF atlas
layout(location = 2) uniform uint fontType; // 0 = Josiah's handpainted SystemShockText, 1 = StopD
layout(location = 3) uniform vec4 textColor;
layout(location = 4) uniform vec2 texelSize;

const float outlineThicknessPixels = 3.2;   // ← how thick the black outline should be (in screen pixels)
const float outlineSoftnessPixels  = 1.1;   // ← how softly it fades out (anti-aliasing)

float DistanceInScreenPixelsFromSDFEdge(float sdf) {
    vec2 unitRange = texelSize * 4.0;
    vec2 screenPxRange = fwidth(TexCoord) / unitRange; // how many SDF units = 1 screen pixel
    float pxRange = max(screenPxRange.x, screenPxRange.y);
    return (sdf - 0.5) * pxRange;                     // positive = inside, negative = outside
}

void main() {
    float sdf = texture(textTexture, TexCoord).r;
    float dist = DistanceInScreenPixelsFromSDFEdge(sdf);
    float alpha = smoothstep(-0.5, 0.5, dist);        // very soft transition (~1 pixel wide)
    if (alpha < 0.01 && fontType == 1) { FragColor = vec4(0.0); return; }

    vec3 color = textColor.rgb;
    if (fontType == 1) { // StopD – Soft directional bevel
        float sR = texture(textTexture, TexCoord + vec2(texelSize.x, 0.0)).r;  // Sample 4 neighbors once
        float sL = texture(textTexture, TexCoord + vec2(-texelSize.x, 0.0)).r;
        float sU = texture(textTexture, TexCoord + vec2(0.0, texelSize.y)).r;
        float sD = texture(textTexture, TexCoord + vec2(0.0, -texelSize.y)).r;
        vec2 grad = vec2(sR - sL, sU - sD);
        float gradLen = length(grad);
        if (gradLen > 0.015) {
            vec2 n = grad / gradLen;
            float vertical = n.y; // +1 = bottom edge (highlight), -1 = top edge (shadow)

            if (abs(vertical) > 0.12) {
                if (vertical > 0.0) { // soft highlight on bottom edges
                   
                    color = mix(color, vec3(1.0), 0.18);
                    alpha *= 0.92;
                } else { // soft shadow on top edges
                    color *= 0.58;
                    alpha *= 0.90;
                }
            }
        }

        FragColor = vec4(color, alpha * textColor.a);
    } else {
        if (sdf >= 0.5) { FragColor = vec4(color, textColor.a); return; }

        bool hasNearbyGlyph = false;
        for (float x = -4.5; x <= 4.5; x += 1.0) {
            for (float y = -5.5; y <= 5.5; y += 1.0) {
                vec2 offset = vec2(x, y) * texelSize;
                float sampleSDF = texture(textTexture, TexCoord + offset).r;
                if (sampleSDF >= 0.5) { hasNearbyGlyph = true; break; }
            }
            if (hasNearbyGlyph) break;
        }

        if (hasNearbyGlyph) FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        else                FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
