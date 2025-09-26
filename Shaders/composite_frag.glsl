// composite.glsl
// Full screen quad unlit textured for presenting image buffers such as results
// from compute shaders, image effects, post-processing, etc..
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
const int SSR_RES = 4;

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
        // Get specular color from G-buffer
        vec4 worldPosPack = texelFetch(inputWorldPos, ivec2(TexCoord * vec2(screenWidth, screenHeight)), 0);
        vec4 specColor = unpackColor32(floatBitsToUint(worldPosPack.a));
        float specSum = specColor.r + specColor.g + specColor.b;

        // Compute blur radius based on specular sum
        float maxRadius = 2.0; // For 5x5 kernel at specSum < 0.3
        float minRadius = 0.0; // For 1x1 kernel (no blur) at specSum > 2.4
        float radius = mix(maxRadius, minRadius, smoothstep(0.3, 2.4, specSum));

        // Gaussian weights for up to 9x9 kernel (simplified for dynamic sizing)
        float weights[25] = float[](
            0.027, 0.110, 0.194, 0.110, 0.027,
            0.110, 0.451, 0.794, 0.451, 0.110,
            0.194, 0.794, 1.398, 0.794, 0.194,
            0.110, 0.451, 0.794, 0.451, 0.110,
            0.027, 0.110, 0.194, 0.110, 0.027
        ); // Normalized later

        vec4 reflectionColor = vec4(0.0);
        float totalWeight = 0.0001; // Avoid division by zero
        int kernelSize = int(ceil(radius)) * 2 + 1; // e.g., 9 for radius = 4, 1 for radius = 0
        float weightScale = 1.0 / (1.398 * float(kernelSize * kernelSize)); // Normalize weights

        // Dynamic kernel loop
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
        FragColor += reflectionColor;
    } else if (debugView == 7 || debugView == 10) {
        FragColor = imageLoad(outputImage, pixel);
    }
}
