// composite.glsl
#version 450 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D tex;
uniform sampler2D inputWorldPos; // G-buffer texture with specular color in .a
uniform int debugView;
uniform int debugValue;
uniform uint screenWidth;
uniform uint screenHeight;
uniform sampler2D outputImage;
layout(std430, binding = 13) buffer BlueNoise { float blueNoiseColors[]; };
const int SSR_RES = 4;

uniform float aaStrength = 2.0; // Controls the radius of AA sampling (in pixels)
uniform float aaThreshold = 0.2; // Gradient threshold for applying AA
uniform float gamma = 1.0; // 0.85 to 1.25 or so seems fine

vec4 unpackColor32(uint color) {
    return vec4(float((color >> 24) & 0xFF) / 255.0,  // r
                float((color >> 16) & 0xFF) / 255.0,  // g
                float((color >>  8) & 0xFF) / 255.0,  // b
                float((color      ) & 0xFF) / 255.0); // a
}

void main() {
    vec3 color = texture(tex, TexCoord).rgb;
    if (debugValue > 0) { FragColor = vec4(color, 1.0); return; }

    ivec2 pixel = ivec2(TexCoord * vec2(screenWidth/SSR_RES, screenHeight/SSR_RES));
    if (debugView == 0) {
        vec2 pixelSize = vec2(1.0 / float(screenWidth), 1.0 / float(screenHeight));
        // Get specular color from G-buffer
        vec4 worldPosPack = texelFetch(inputWorldPos, ivec2(TexCoord * vec2(screenWidth, screenHeight)), 0);
        vec4 specColor = unpackColor32(floatBitsToUint(worldPosPack.a));
        float specSum = specColor.r + specColor.g + specColor.b;

        // Compute blur radius based on specular sum
        if (debugValue == 0) {
            float maxRadius = 2.0; // For 5x5 kernel at specSum < 0.3
            float minRadius = 0.0; // For 1x1 kernel (no blur) at specSum > 2.2
            float radius = mix(maxRadius, minRadius, smoothstep(0.3, 2.2, specSum));
            vec4 reflectionColor = vec4(0.0);
            float totalWeight = 0.0001; // Avoid division by zero
            int kernelSize = int(ceil(radius)) * 2 + 1; // e.g., 9 for radius = 4, 1 for radius = 0
            float weightScale = 1.0 / (1.398 * float(kernelSize * kernelSize)); // Normalize weights
            for (int y = -int(radius); y <= int(radius); y++) {
                for (int x = -int(radius); x <= int(radius); x++) {
                    ivec2 offset = ivec2(x, y);
                    ivec2 samplePixel = pixel + offset;
                    samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));

                    // Compute Gaussian weight based on distance
                    float dist = length(vec2(x, y) / max(radius, 0.1)); // Avoid division by zero
                    float weight = exp(-dist * dist * 0.5) * weightScale; // Simplified Gaussian

                    vec2 sampleUV = (vec2(samplePixel) + 0.5) / vec2(screenWidth/SSR_RES, screenHeight/SSR_RES);
                    vec3 sampleColor = texture(outputImage, sampleUV).rgb;
                    reflectionColor.rgb += sampleColor * weight;
                    totalWeight += weight;
                }
            }

            reflectionColor.rgb /= totalWeight;
            reflectionColor.rgb = clamp(reflectionColor.rgb, 0.0, 1.0);
            color += reflectionColor.rgb;
        }

        // SMAA-Inspired Edge-Directed Antialiasing
        // Compute luminance for edge detection
        vec3 centerColor = texture(tex, TexCoord).rgb;
        float lumaCenter = dot(centerColor, vec3(0.299, 0.587, 0.114)); // Luminance (Rec. 601)
        vec3 dx = texture(tex, TexCoord + vec2(pixelSize.x, 0.0)).rgb - texture(tex, TexCoord - vec2(pixelSize.x, 0.0)).rgb;
        vec3 dy = texture(tex, TexCoord + vec2(0.0, pixelSize.y)).rgb - texture(tex, TexCoord - vec2(0.0, pixelSize.y)).rgb;
        float lumaDx = dot(abs(dx), vec3(0.299, 0.587, 0.114));
        float lumaDy = dot(abs(dy), vec3(0.299, 0.587, 0.114));
        float gradientMag = lumaDx + lumaDy; // Luminance-based gradient magnitude

        vec3 aaColor = color; // Default to chromatic aberration result
        if (gradientMag > aaThreshold) {
            // Determine edge direction
            vec2 edgeDir = vec2(lumaDx, lumaDy);
            edgeDir = normalize(edgeDir + 1e-6); // Avoid division by zero
            vec2 orthoDir = vec2(-edgeDir.y, edgeDir.x); // Perpendicular to edge

            // Sample along the edge (up to ±5 pixels)
            vec3 sampleColor = vec3(0.0);
            float aaWeightSum = 0.0;
            const int sampleCount = 10; // Samples per side (total 11 samples: -5 to +5)
            float maxDist = aaStrength; // Max sampling distance in pixels
            for (int i = -sampleCount; i <= sampleCount; i++) {
                float t = float(i) / float(sampleCount); // Normalized position [-1, 1]
                float dist = t * maxDist; // Distance along edge
                float weight = exp(-abs(t) * 2.0); // Gaussian weight (sigma = 0.5)
                vec2 sampleUV = TexCoord + orthoDir * dist * pixelSize;
                sampleColor += texture(tex, sampleUV).rgb * weight;
                aaWeightSum += weight;
            }
            sampleColor /= aaWeightSum;

            // Dynamic blending based on edge contrast
            float blendFactor = clamp(gradientMag * 0.5, 2.0, 4.0); // Adjust blend based on edge strength
            aaColor = mix(color, sampleColor, blendFactor);
        }

        aaColor.rgb = pow(aaColor.rgb, vec3(1.0 / gamma));
        FragColor = vec4(aaColor, 1.0);
    } else if (debugView == 7 || debugView == 10) {
        vec2 sampleUV = (vec2(pixel) + 0.5) / vec2(screenWidth/SSR_RES, screenHeight/SSR_RES);
        FragColor = texture(outputImage, sampleUV);
    }
}
