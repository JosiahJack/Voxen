// composite.glsl
#version 450 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D tex;
layout(rgba32f, binding = 1) readonly uniform image2D inputWorldPos;
uniform int debugView;
uniform int debugValue;
uniform uint screenWidth;
uniform uint screenHeight;
uniform sampler2D outputImage;
uniform uint reflectionsEnabled;
uniform uint aaEnabled;
uniform uint brightnessSetting;
uniform vec3 camRot;
uniform float timeVal;

const int SSR_RES = 4;
const float PI = 3.14159265359;

uniform float aaThreshold = 0.2; // Gradient threshold for applying AA

// Hash function for pseudo-random noise
float hash(vec2 p) {
    p = fract(p * vec2(123.45, 678.90));
    p += dot(p, p + vec2(45.67, 89.01));
    return fract(p.x * p.y * 43758.5453);
}

// Simple 2D noise for star field
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f); // Smooth interpolation
    return mix(
        mix(hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x),
        mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x),
        u.y
    );
}

// Star field generation
vec3 starField(vec2 uv, float density, float brightness) {
    vec2 grid = uv * 10000.0; // Reduced scale for more visible stars
    vec2 gridId = floor(grid);
    vec2 gridUv = fract(grid);
    float star = 0.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec2 offset = vec2(float(x), float(y));
            vec2 starPos = vec2(hash(gridId + offset));
            float dist = length(gridUv - starPos - offset);
            float starSize = 0.3 * hash(gridId + offset + vec2(0.5)); // Larger stars
            float twinkle = hash(gridId + offset + vec2(timeVal * 0.5)); // Faster twinkle
            star += smoothstep(starSize, 0.0, dist) * brightness * (0.5 + 0.5 * twinkle); // Boost twinkle
        }
    }
    return vec3(star) * density;
}

// Milky Way generation
vec3 milkyWay(vec2 uv) {
    float theta = uv.x * 2.0 * PI; // Longitude
    float phi = uv.y * PI; // Latitude
    float milkyWayAngle = 0.3 * PI; // Tilt of Milky Way
    float phiMilky = abs(phi - (0.5 * PI + sin(theta) * 0.2)); // Wavy band
    float intensity = exp(-phiMilky * phiMilky * 5.0); // Softer Gaussian for wider band
    intensity *= (noise(uv * 5.0) * 0.5 + 0.5); // Softer noise texture
    return vec3(intensity) * 0.1; // Brighter Milky Way
}

void main() {
    vec3 color = texture(tex, TexCoord).rgb;
    // Adjusted clear color detection for transparency
    if (color.r > 0.6 && color.g < 0.25 && color.b > 0.6) { // Hot pink clear color
        // Convert screen-space TexCoord to view direction
        vec2 ndc = TexCoord * 2.0 - 1.0; // Map [0,1] to [-1,1]
        // Assume a perspective projection with 90-degree FOV for simplicity
        float aspect = float(screenWidth) / float(screenHeight);
        vec3 viewDir = normalize(vec3(ndc.x * aspect, ndc.y, -1.0)); // View direction in view space

        // Apply camera rotation (yaw, pitch, roll) to viewDir
        // Convert camRot (yaw, pitch, roll) to a rotation matrix
        float cy = cos(camRot.x); // yaw
        float sy = sin(camRot.x);
        float cp = cos(camRot.y); // pitch
        float sp = sin(camRot.y);
        float cr = cos(camRot.z); // roll
        float sr = sin(camRot.z);
        mat3 yawMatrix = mat3(
            cy, 0.0, sy,
            0.0, 1.0, 0.0,
            -sy, 0.0, cy
        );
        mat3 pitchMatrix = mat3(
            1.0, 0.0, 0.0,
            0.0, cp, -sp,
            0.0, sp, cp
        );
        mat3 rollMatrix = mat3(
            cr, -sr, 0.0,
            sr, cr, 0.0,
            0.0, 0.0, 1.0
        );
        mat3 rotMatrix = yawMatrix * pitchMatrix * rollMatrix;
        viewDir = rotMatrix * viewDir; // Rotate view direction to world space

        // Convert viewDir to equirectangular UVs
        float theta = atan(viewDir.x, viewDir.z);
        float phi = acos(viewDir.y);
        vec2 uv = vec2(theta / (2.0 * PI) + 0.5, phi / PI); // Shift u to [0,1]
        uv = clamp(uv, 0.0, 1.0);

        // Generate procedural sky
        vec3 skyColor = vec3(0.0); // Black background
        skyColor += starField(uv, 8.0, 2.0); // Increased density and brightness
        skyColor += milkyWay(uv); // Milky Way

        // Debug: Visualize UVs or components if needed
        if (debugView == 1) {
            skyColor = vec3(uv, 0.0); // Visualize UVs
        } else if (debugView == 2) {
            skyColor = starField(uv, 1.0, 2.0); // Stars only
        } else if (debugView == 3) {
            skyColor = milkyWay(uv); // Milky Way only
        }

        FragColor = vec4(skyColor, 1.0);
        return;
    }

//     if (debugValue > 0) { FragColor = vec4(color, 1.0); return; }

    ivec2 pixel = ivec2(TexCoord * vec2(screenWidth/SSR_RES, screenHeight/SSR_RES));
//     if (debugView == 0) {
        if (reflectionsEnabled > 0) {
            vec2 sampleUV = (vec2(pixel)) / vec2(screenWidth/SSR_RES, screenHeight/SSR_RES);
            vec3 reflectionColor = texture(outputImage, sampleUV).rgb;
            color += reflectionColor;
        }

        vec3 aaColor = color; // Default to chromatic aberration result
        if (aaEnabled > 0) {
            // SMAA-Inspired Edge-Directed Antialiasing
            // Compute luminance for edge detection
            vec2 pixelSize = vec2(1.0 / float(screenWidth), 1.0 / float(screenHeight));
            vec3 centerColor = texture(tex, TexCoord).rgb;
            float lumaCenter = dot(centerColor, vec3(0.299, 0.587, 0.114)); // Luminance (Rec. 601)
            vec3 dx = texture(tex, TexCoord + vec2(pixelSize.x, 0.0)).rgb - texture(tex, TexCoord - vec2(pixelSize.x, 0.0)).rgb;
            vec3 dy = texture(tex, TexCoord + vec2(0.0, pixelSize.y)).rgb - texture(tex, TexCoord - vec2(0.0, pixelSize.y)).rgb;
            float lumaDx = dot(abs(dx), vec3(0.299, 0.587, 0.114));
            float lumaDy = dot(abs(dy), vec3(0.299, 0.587, 0.114));
            float gradientMag = lumaDx + lumaDy; // Luminance-based gradient magnitude
            if (gradientMag > aaThreshold) {
                // Determine edge direction
                vec2 edgeDir = vec2(lumaDx, lumaDy);
                edgeDir = normalize(edgeDir + 1e-6); // Avoid division by zero
                vec2 orthoDir = vec2(-edgeDir.y, edgeDir.x); // Perpendicular to edge

                // Sample along the edge (up to ±5 pixels)
                vec3 sampleColor = vec3(0.0);
                float aaWeightSum = 0.0;
                const int sampleCount = 10; // Samples per side (total 11 samples: -5 to +5)
                for (int i = -sampleCount; i <= sampleCount; i++) {
                    float t = float(i) / float(sampleCount); // Normalized position [-1, 1]
                    float dist = t * 2.0; // Distance along edge
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
        }

        aaColor.rgb = pow(aaColor.rgb, vec3(1.0 / (float(brightnessSetting) / 100.0)));
        FragColor = vec4(aaColor, 1.0);
//     } else if (debugView == 7 || debugView == 10) {
//         vec2 sampleUV = (vec2(pixel) + 0.5) / vec2(screenWidth/SSR_RES, screenHeight/SSR_RES);
//         FragColor = texture(outputImage, sampleUV);
//     }
}
