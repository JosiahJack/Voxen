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

// Simplex noise implementation (2D)
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x * 34.0) + 1.0) * x); }

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod289(i);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0));
    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

// Cellular noise for star field
float cellularStar(vec2 uv, float scale, float brightness, float time) {
    vec2 p = uv * scale; // Scale to control star density
    vec2 i = floor(p);
    vec2 f = fract(p);
    float minDist = 1.0; // Minimum distance to nearest star
    vec2 starPos;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));
            vec2 point = vec2(snoise(i + neighbor)) * 0.5 + 0.5; // Random point in cell
            vec2 diff = neighbor + point - f;
            float dist = length(diff);
            if (dist < minDist) {
                minDist = dist;
                starPos = point;
            }
        }
    }
    // Circular star shape with radial falloff
    float star = smoothstep(0.1, 0.0, minDist) * brightness;
    // Twinkle using Simplex noise
    float twinkle = snoise(i * 0.5 + vec2(time * 0.5));
    twinkle = 0.5 + 0.5 * twinkle;
    star *= twinkle;
    return star;
}

// Star field generation
vec3 starField(vec2 uv, float density, float brightness) {
    vec2 spherical = uv * vec2(2.0 * PI, PI); // Map to [0,2PI] x [0,PI]
    float star = cellularStar(spherical, 20.0, brightness, timeVal); // Scale for density
    return vec3(star) * density;
}

// Milky Way generation
vec3 milkyWay(vec2 uv) {
    float theta = uv.x * 2.0 * PI;
    float phi = uv.y * PI;
    float milkyWayAngle = 0.3 * PI;
    // Use cos for smooth periodic wave to avoid seam
    float phiMilky = abs(phi - (0.5 * PI + cos(theta) * 0.2));
    float intensity = exp(-phiMilky * phiMilky * 5.0);
    intensity *= (snoise(uv * 5.0) * 0.5 + 0.5);
    return vec3(intensity) * 0.15; // Adjusted for balance
}

void main() {
    vec3 color = texture(tex, TexCoord).rgb;
    if (color.r > 0.6 && color.g < 0.25 && color.b > 0.6) { // Hot pink clear color
        // Convert screen-space TexCoord to view direction
        vec2 ndc = TexCoord * 2.0 - 1.0;
        float aspect = float(screenWidth) / float(screenHeight);
        vec3 viewDir = normalize(vec3(ndc.x * aspect, ndc.y, -1.0));

        // Apply camera rotation
        float cy = cos(camRot.x);
        float sy = sin(camRot.x);
        float cp = cos(camRot.y);
        float sp = sin(camRot.y);
        float cr = cos(camRot.z);
        float sr = sin(camRot.z);
        mat3 yawMatrix = mat3(cy, 0.0, sy, 0.0, 1.0, 0.0, -sy, 0.0, cy);
        mat3 pitchMatrix = mat3(1.0, 0.0, 0.0, 0.0, cp, -sp, 0.0, sp, cp);
        mat3 rollMatrix = mat3(cr, -sr, 0.0, sr, cr, 0.0, 0.0, 0.0, 1.0);
        mat3 rotMatrix = yawMatrix * pitchMatrix * rollMatrix;
        viewDir = rotMatrix * viewDir;

        // Convert viewDir to equirectangular UVs
        float theta = atan(viewDir.x, viewDir.z);
        float phi = acos(viewDir.y);
        vec2 uv = vec2(theta / (2.0 * PI) + 0.5, phi / PI);
        uv = clamp(uv, 0.0, 1.0);

        // Generate procedural sky
        vec3 skyColor = vec3(0.0);
        skyColor += starField(uv, 0.5, 2.0);
        skyColor += milkyWay(uv);

        // Debug modes
        if (debugView == 1) {
            skyColor = vec3(uv, 0.0); // Visualize UVs
        } else if (debugView == 2) {
            skyColor = starField(uv, 0.5, 2.0); // Stars only
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
