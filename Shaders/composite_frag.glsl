// composite.glsl
#version 430 core
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
uniform float fov;
uniform float timeVal;
uniform uint skyVisible;
uniform uint planetaryBodiesVisible;
uniform uint groveShieldVisible;
uniform uint stationShieldVisible;

const int SSR_RES = 4;
const float PI = 3.14159265359;
uniform float aaThreshold = 0.2;

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

float dither(vec2 uv, float scale, float fac, float finalMultiplier) {
    return fract(snoise(uv * vec2(screenWidth, screenHeight) * 0.5) * 0.025) * finalMultiplier;
}

// Cellular noise for star field with density and size variation
vec3 cellularStar(vec2 uv, float scale, float brightness, float time, float densityMod) {
    vec2 p = uv * scale;
    vec2 i = floor(p);
    vec2 f = fract(p);
    float minDist = 1.0;
    vec2 starPos;
    vec3 starColor = vec3(1.0);
    float sizeMod = 0.1;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));
            vec2 point = vec2(
                snoise(i + neighbor + vec2(0.0, 0.0)),
                snoise(i + neighbor + vec2(1.0, 1.0))
            ) * 0.5 + 0.5;
            vec2 diff = neighbor + point - f;
            float dist = length(diff);
            if (dist < minDist) {
                minDist = dist;
                starPos = point;
                float colorNoise = snoise(i + neighbor + vec2(2.0, 2.0)) * 0.5 + 0.5;
                starColor = mix(vec3(0.8, 0.8, 1.0), vec3(1.0, 0.9, 0.7), colorNoise);
                sizeMod = mix(0.02, 0.15, snoise(i + neighbor + vec2(3.0, 3.0)) * 0.5 + 0.5);
            }
        }
    }
    float star = smoothstep(sizeMod, 0.0, minDist) * brightness * densityMod;
    return starColor * star;
}

// Star field generation with density variation in polar coordinates
vec3 starField(vec3 dir, float density, float brightness) {
    // Convert direction to polar coordinates
    float theta = atan(dir.z, dir.x); // Azimuth [0, 2PI]
    float phi = acos(dir.y); // Elevation [0, PI]
    vec2 noiseUV = vec2(theta, phi); // Use raw polar coords for noise
    // Layered Simplex noise for density variation
    float densityMod = snoise(noiseUV * 0.5) * 0.5 + 0.5; // Primary layer
    densityMod = mix(0.0, 2.5, densityMod); // Stronger variation
    float densityMod2 = snoise(noiseUV * 10.5 + vec2(10.0)) * 0.5 + 0.5; // Higher frequency, phase-shifted
    densityMod = mix(densityMod, 0.0, densityMod2 * 3.2); // Stronger dark patches
    return cellularStar(noiseUV, 40.0, brightness, timeVal * 0.1, densityMod) * density;
}

// Milky Way generation in polar coordinates
vec3 milkyWay(vec3 dir) {
    float phi = acos(dir.y); // Elevation [0, PI]
    float theta = atan(dir.z, dir.x); // Azimuth [0, 2PI]
    // Blend across theta = ±π to avoid seam
    float seamBlend = smoothstep(PI - 0.2, PI, abs(theta)); // Fade near ±π
    vec2 noiseUV = vec2(theta * 0.5, phi); // Scale theta for smoother noise
    float tiltAngle = 60.0 * PI / 180.0;
    float phiTilted = phi - tiltAngle * cos(theta);
    float phiMilky = abs(phiTilted - (0.5 * PI + cos(theta * 2.0) * 0.1));
    float intensity = exp(-phiMilky * phiMilky * 4.0);
    float poleFade = smoothstep(0.0, 2.0, phi) * smoothstep(0.0, 2.0, PI - phi);
    intensity *= (snoise(noiseUV * 0.6) * 0.2 + 0.8); // 2D noise, softer scale
    intensity *= poleFade * (1.0 - seamBlend * 0.5); // Apply seam and pole fade
    float tintNoise = clamp(snoise(noiseUV * 0.3 + vec2(5.0)) * 0.5 + 0.5, 0.0, 1.0);
    vec3 tint = mix(vec3(0.7, 0.85, 0.7), vec3(1.0, 0.95, 0.99), tintNoise);
    float ditherVal = dither(noiseUV, 0.2, 0.015, 0.01);
    return vec3(intensity) * tint * 0.06 + vec3(ditherVal);
}

vec3 lerp(vec3 a, vec3 b, float t) {
    return mix(a, b, clamp(t, 0.0, 1.0));
}

void main() {
    vec4 color = texture(tex, TexCoord).rgba;
    bool isSky = (color.a > 0.19 && color.a < 0.21 && skyVisible > 0); // Sky hack alpha
    float mappedLat = 0.0;
    if (isSky) {
        vec2 ndc = TexCoord * 2.0 - 1.0;
        float aspect = float(screenWidth) / float(screenHeight);
        float fovRad = fov * PI / 180.0; // Convert FOV to radians
        float tanHalfFov = tan(fovRad * 0.5);
        vec3 viewDir = normalize(vec3(ndc.x * tanHalfFov * aspect, ndc.y * tanHalfFov, -1.0));
        float cy = cos(camRot.x + timeVal * 0.05); // Yaw + time-based rotation
        float sy = sin(camRot.x + timeVal * 0.05);
        float cp = cos(camRot.y); // Pitch
        float sp = sin(camRot.y);
        mat3 yawMatrix = mat3(cy, 0.0, sy, 0.0, 1.0, 0.0, -sy, 0.0, cy);
        mat3 pitchMatrix = mat3(1.0, 0.0, 0.0, 0.0, cp, -sp, 0.0, sp, cp);
        mat3 skyRotMatrix = yawMatrix * pitchMatrix; // Combine yaw and pitch
        vec3 skyDir = skyRotMatrix * viewDir; // Yaw and pitch for sky
        vec3 microwaveBackground = vec3(0.034, 0.02, 0.05); // Not really the mbr but sounds cool.
        vec3 shieldColor = vec3(0.0, 0.0, 0.00);
        vec3 saturnCenterWorld = vec3(0.0, -6.0, 456.0);
        vec3 saturnCenter = normalize(vec3(0.0, -0.1, sqrt(1.0 - 0.1*0.1))); // Lower position for below horizon
        if (stationShieldVisible > 0 || groveShieldVisible > 0) {
            vec3 viewDirNorm = normalize(skyDir);
            vec3 sunDir = normalize(-saturnCenter);
            vec3 saturnDir = normalize(saturnCenter);
            vec3 upDir = vec3(0.0, 1.0, 0.0);
            float base = 0.2;
            float sunHighlight = pow(max(dot(viewDirNorm, sunDir), 0.0), 32.0);
            float saturnHighlight = pow(max(dot(viewDirNorm, saturnDir), 0.0), 16.0);
            float fres = pow(1.0 - abs(dot(viewDirNorm, upDir)), 1.5) * 0.15;
            float intensity = base + sunHighlight * 0.5 + saturnHighlight * 1.35 + fres;
            intensity = clamp(intensity, 0.0, 1.0);
            vec3 baseColor = vec3(0.01, 0.08, 0.015);
            vec3 glowColor = vec3(0.2, 0.5, 0.25);
            shieldColor = mix(baseColor, glowColor, intensity) * 0.451;
            microwaveBackground += shieldColor;
        }

        vec3 skyColor = microwaveBackground + starField(skyDir, 0.5, 1.8);
        skyColor.r = clamp(skyColor.r, microwaveBackground.r, 1.0); // Prevent black spots where noise pulls below base color of background.
        skyColor.g = clamp(skyColor.g, microwaveBackground.g, 1.0);
        skyColor.b = clamp(skyColor.b, microwaveBackground.b, 1.0);
        if (planetaryBodiesVisible > 0) { // No milkyway, saturn, rings, or sun for cyberspace... just stars.
            skyColor += milkyWay(skyDir);

            // Procedural Saturn
            vec3 saturnPole = vec3(0.0, 1.0, 0.0);
            float planetRadius = 0.451;
            float cosPlanet = cos(planetRadius);
            float dd = dot(skyDir, saturnCenter);
            vec3 mainColor1 = vec3(0.85, 0.78, 0.6);
            vec3 mainColor2 = vec3(0.82, 0.74, 0.62);
            vec3 darkColor = vec3(0.81, 0.73, 0.55);
            vec3 ringColor = darkColor;
            vec3 tiltedPole = normalize(vec3(saturnPole.x, saturnPole.y - saturnPole.z * 0.5, saturnPole.y * 0.5 + saturnPole.z));
            vec3 rayDir = normalize(skyDir);
            bool pixelLiesOnPlanet = (dd > cosPlanet);
            if (pixelLiesOnPlanet) {
                float t = cosPlanet / max(dot(skyDir, saturnCenter), 0.001);
                vec3 surfacePoint = saturnCenter + rayDir * (cosPlanet / sin(planetRadius)) * t;
                vec3 planetNormal = normalize(surfacePoint - saturnCenter);
                float latitude = asin(clamp(dot(planetNormal, tiltedPole), -0.9999, 0.9999));
                mappedLat = latitude / (PI * 0.5); // Normalize to [-1, 1]
                mappedLat = clamp(mappedLat, -1.0, 1.0);
                float tGrad = (mappedLat + 1.0) * 0.5; // Map [-1, 1] to [0, 1]
                vec3 baseColor = mix(darkColor, mainColor2, smoothstep(0.3, 0.5, tGrad));
                baseColor = mix(baseColor, mainColor1, smoothstep(0.5, 0.7, tGrad));
                vec2 noiseUV1 = vec2(mappedLat * 18.0, 0.0);
                vec2 noiseUV2 = vec2(mappedLat * 24.0, 0.0);
                float noise1 = snoise(noiseUV1) * 0.5 + 0.5;
                float noise2 = snoise(noiseUV2) * 0.5 + 0.5;
                vec3 stripeColor = mix(baseColor, mix(darkColor, mainColor1, smoothstep(0.4, 0.6, noise1)), smoothstep(0.2, 0.8, noise2));
                float concavity = 1.0 - abs(mappedLat);
                vec3 planetColor = stripeColor * mix(0.7, 1.0, concavity);
                float viewLat = dot(planetNormal, tiltedPole);
                float sphericalDarkeningFactor = pow(clamp(dd, 0.0, 1.0), 16.0);
                planetColor *= sphericalDarkeningFactor;
                float alpha = acos(dd);
                float aa_width = 0.0045;
                float edge_dist = planetRadius - alpha;
                float diskMask = smoothstep(0.0, aa_width, edge_dist);
                skyColor = mix(skyColor, planetColor + shieldColor, diskMask);
            } else {
                // Saturn Rings - fixed world plane at y = -300
                float ringPlaneY = -35.0;
                vec3 ringNormal = vec3(0.0, 1.0, 0.0); // world up plane
                vec3 ringU = normalize(vec3(1.0, 0.0, 0.0));
                vec3 ringV = normalize(cross(ringNormal, ringU));

                // intersection of view ray with world plane
                float denom = dot(rayDir, ringNormal);
                if (abs(denom) > 1e-6) {
                    float t = (ringPlaneY) / denom;
                    if (t > 0.0) {
                        vec3 hitPos = rayDir * t;

                        // position relative to Saturn center (for concentric rings)
                        vec3 rel = hitPos - saturnCenterWorld;
                        float u = dot(rel, ringU);
                        float v = dot(rel, ringV);
                        float proj_r = length(vec2(u, v));
                        float a_i = 400.0; // radii in world units
                        float a_o = 580.0;
                        float r_norm = clamp((proj_r - a_i) / (a_o - a_i), 0.0, 1.0);
                        if (proj_r >= a_i && proj_r <= a_o) {
                            float density1 = snoise(vec2(r_norm * 16.0, timeVal * 0.05)) * 0.5 + 0.5;
                            float density2 = snoise(vec2(r_norm * 4.0, timeVal * 0.08)) * 0.5 + 0.5;
                            float density = mix(density1, density2, 0.45);
                            density = smoothstep(0.25, 0.75, density);
                            float foreshort = clamp(0.5 + 0.8 * abs(dot(ringNormal, rayDir)), 0.2, 1.0);
                            vec3 ringBase = mix(ringColor, mainColor1, 0.35);
                            ringBase = mix(ringBase, vec3(0.95, 0.9, 0.8), 0.08);
                            float innerFade = smoothstep(0.0, 0.06, r_norm);
                            float outerFade = 1.0 - smoothstep(0.94, 1.0, r_norm);
                            float ringDensity = density * innerFade * outerFade * foreshort;
                            skyColor += ringBase * ringDensity * 1.05;
                        }
                    }
                }
            }

            // Sun
            vec3 sunDir = normalize(-saturnCenter);
            float sunSize = 0.009;
            float sunDist = acos(dot(skyDir, sunDir));
            float sunMask = smoothstep(sunSize, sunSize * 0.8, sunDist);
            vec3 sunColor = vec3(1.0, 0.97, 0.85);
            float corona = exp(-pow(sunDist / (sunSize * 1.5), 2.0)) * 1.2;
            skyColor += sunColor * (sunMask * 3.0 + corona * 1.5);
        }

        FragColor = vec4((color.rgb * color.a) + skyColor, 1.0); // Add window alpha weighted color tint
    }

//     if (debugValue > 0) { FragColor = vec4(color.rgb, 1.0); return; }

    ivec2 pixel = ivec2(TexCoord * vec2(screenWidth/SSR_RES, screenHeight/SSR_RES));
//     if (debugView == 0) {
        if (reflectionsEnabled > 0) {
            vec2 sampleUV = (vec2(pixel)) / vec2(screenWidth/SSR_RES, screenHeight/SSR_RES);
            vec3 reflectionColor = texture(outputImage, sampleUV).rgb;
            if (isSky) {
                FragColor.rgb += reflectionColor;
                return;
            }

            color.rgb += reflectionColor;
        }

        vec3 aaColor = color.rgb; // Default to chromatic aberration result
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
                aaColor = mix(color.rgb, sampleColor, blendFactor);
            }
        }

        aaColor.rgb = pow(aaColor.rgb, vec3(1.0 / (float(brightnessSetting) / 100.0)));
        FragColor = vec4(aaColor, 1.0);
//     } else if (debugView == 7 || debugView == 10) {
//         vec2 sampleUV = (vec2(pixel) + 0.5) / vec2(screenWidth/SSR_RES, screenHeight/SSR_RES);
//         FragColor = texture(outputImage, sampleUV);
//     }
}
