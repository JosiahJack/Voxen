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
layout(rgba8, binding = 4) uniform image2D outputImage;
layout(std430, binding = 13) buffer BlueNoise { float blueNoiseColors[]; };
const int SSR_RES = 4;

uniform float chromaticAberrationStrength = 1.72; // Base strength for chromatic aberration
uniform float aaStrength = 2.0; // Controls the radius of AA sampling (in pixels)
uniform float aaThreshold = 0.2; // Gradient threshold for applying AA
uniform float crtScanlineStrength = 1.0;//0.9; Good value, off for testing.
uniform float lensWarpStrength = 0.3; // Controls lens warp intensity (0.0 to 0.3 for subtle)
uniform float gamma = 1.0; // 0.85 to 1.25 or so seems fine

vec4 unpackColor32(uint color) {
    return vec4(float((color >> 24) & 0xFF) / 255.0,  // r
                float((color >> 16) & 0xFF) / 255.0,  // g
                float((color >>  8) & 0xFF) / 255.0,  // b
                float((color      ) & 0xFF) / 255.0); // a
}

void main() {
    FragColor = texture(tex, TexCoord);
    if (debugValue > 0) return;

    ivec2 pixel = ivec2(TexCoord * vec2(screenWidth/SSR_RES, screenHeight/SSR_RES));
    if (debugView == 0) {
        // Lens Warp
        vec2 uv = TexCoord;
//         vec2 center = vec2(0.5, 0.5);
//         vec2 delta = uv - center;
//         float r2 = dot(delta, delta); // Squared distance from center
//         float warpFactor = 1.0 + lensWarpStrength * r2 * smoothstep(0.7, 0.0, sqrt(r2)); // Barrel distortion with edge falloff
//         uv = center + delta * warpFactor; // Apply warp
//         uv = clamp(uv, vec2(0.0), vec2(1.0)); // Prevent sampling outside texture

        // Chromatic Aberration
//         vec2 uv = TexCoord;
        vec2 pixelSize = vec2(1.0 / float(screenWidth), 1.0 / float(screenHeight));
        float aberrationStrength = chromaticAberrationStrength;

        // Modulate aberration strength based on distance from screen center
        float distFromCenter = length(TexCoord - vec2(0.5)); // Distance from (0.5, 0.5)
        float centerFade = smoothstep(0.0, 0.7, distFromCenter); // 0 at center, 1 at edges
        aberrationStrength *= centerFade; // Reduce strength near center

        // Dither with blue noise
        int blueNoiseTextureWidth = 64;
        int pixelIndex = ((pixel.y & (blueNoiseTextureWidth - 1)) * blueNoiseTextureWidth + (pixel.x & (blueNoiseTextureWidth - 1))) * 3;
        vec4 bluenoise = vec4(blueNoiseColors[pixelIndex], blueNoiseColors[pixelIndex + 1], blueNoiseColors[pixelIndex + 2], 1.0);
        aberrationStrength += ((bluenoise.r * 0.1) - 0.05); // Adjusted to keep dither subtle
        
        // Sample each color channel with a slight offset
        vec2 redOffset = vec2(aberrationStrength, 0.0); // Red shifts right
        vec2 greenOffset = vec2(0.0, 0.0);              // Green stays centered
        vec2 blueOffset = vec2(-aberrationStrength, 0.0); // Blue shifts left
        
        vec3 color;
        color.r = texture(tex, uv + redOffset * pixelSize).r;
        color.g = texture(tex, uv + greenOffset * pixelSize).g;
        color.b = texture(tex, uv + blueOffset * pixelSize).b;

        // Get specular color from G-buffer
        vec4 worldPosPack = texelFetch(inputWorldPos, ivec2(TexCoord * vec2(screenWidth, screenHeight)), 0);
        vec4 specColor = unpackColor32(floatBitsToUint(worldPosPack.a));
        float specSum = specColor.r + specColor.g + specColor.b;

        // Compute blur radius based on specular sum
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
                vec3 sampleColor = imageLoad(outputImage, samplePixel).rgb;
                reflectionColor.rgb += sampleColor * weight;
                totalWeight += weight;
            }
        }

        reflectionColor.rgb /= totalWeight;
        color += reflectionColor.rgb;

        // SMAA-Inspired Edge-Directed Antialiasing
        // Compute luminance for edge detection
        vec3 centerColor = texture(tex, uv).rgb;
        float lumaCenter = dot(centerColor, vec3(0.299, 0.587, 0.114)); // Luminance (Rec. 601)
        vec3 dx = texture(tex, uv + vec2(pixelSize.x, 0.0)).rgb - texture(tex, uv - vec2(pixelSize.x, 0.0)).rgb;
        vec3 dy = texture(tex, uv + vec2(0.0, pixelSize.y)).rgb - texture(tex, uv - vec2(0.0, pixelSize.y)).rgb;
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
            const int sampleCount = 5; // Samples per side (total 11 samples: -5 to +5)
            float maxDist = aaStrength; // Max sampling distance in pixels
            for (int i = -sampleCount; i <= sampleCount; i++) {
                float t = float(i) / float(sampleCount); // Normalized position [-1, 1]
                float dist = t * maxDist; // Distance along edge
                float weight = exp(-abs(t) * 2.0); // Gaussian weight (sigma = 0.5)
                vec2 sampleUV = uv + orthoDir * dist * pixelSize;
                sampleColor += texture(tex, sampleUV).rgb * weight;
                aaWeightSum += weight;
            }
            sampleColor /= aaWeightSum;

            // Dynamic blending based on edge contrast
            float blendFactor = clamp(gradientMag * 0.5, 2.0, 4.0); // Adjust blend based on edge strength
            aaColor = mix(color, sampleColor, blendFactor);
        }

        // CRT Scanline effect
        if (mod(TexCoord.y * float(screenHeight), 2.0) < 1.0) {
            aaColor *= crtScanlineStrength; // Darken by up to 20% (0.8 at strength=0.5)
        }

        aaColor.rgb = pow(aaColor.rgb, vec3(1.0 / gamma));

        // Combine with original alpha
        FragColor = vec4(aaColor, FragColor.a);
    } else if (debugView == 7 || debugView == 10) {
        FragColor = imageLoad(outputImage, pixel);
    }
}
