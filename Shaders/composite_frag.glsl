// composite.glsl
#version 430 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D tex;
layout(rgba32f, binding = 1) readonly uniform image2D inputWorldPos;
layout(std430, binding = 5)  buffer ShadowMaps { uint shadowMaps[]; };
layout(std430, binding = 19) buffer LightIndices { float lights[]; };
layout(std430, binding = 26) buffer VoxelLightListIndices { uint voxelLightListIndices[]; };
layout(std430, binding = 27) buffer UniqueLightLists { uint uniqueLightLists[]; };

uniform int debugView;
uniform int debugValue;
uniform uint screenWidth;
uniform uint screenHeight;
uniform float worldMin_x;
uniform float worldMin_z;
uniform sampler2D outputImage;
uniform uint reflectionsEnabled;
uniform uint aaEnabled;
uniform float berserkTimeRemaining;
uniform float berserkSeedTimestamp;
uniform uint brightnessSetting;
uniform vec3 camRot;
uniform vec3 camPos;
uniform float fov;
uniform float timeVal;
uniform float aspect;
uniform uint skyVisible;
uniform uint planetaryBodiesVisible;
uniform uint groveShieldVisible;
uniform uint stationShieldVisible;
uniform uint empEffectActive;
uniform uint  shadowsEnabled;
uniform float shadowmapSize;
uniform mat4 viewProjection;
uniform mat3 invViewRot;

const int SSR_RES = 4; // 1/SSR_RES = scale factor, e.g. 1/4 = 25% resolution vs main screen.

const float vhsBlurAmount = 0.5; // Cannot be overstated just how magical and impactful this setting is.  DO NOT EVER TURN OFF EVER!!  I recant my former statement about avoiding blur at all costs in all scenarios.
const float vhsRadiusMax = 3.0; // in pixels

const float staticIntensity = 0.0;      // 0.0 .. 1.0
const float staticBandThickness = 0.005;
const float staticScrollSpeed = 200.0;
const vec3  staticColor = vec3(1.0, 0.0, 0.0);

const uint  volumetricFogEnabled = 0;   // 0 = off, 1 = on TODO Fix volumetric fog to behave correctly
const float fogDensity = 1.0;             // density (e.g. 0.02)
const float fogStepSize = 0.08;            // world-space step length
const uint  fogMaxSteps = 24;            // max ray-march steps (e.g. 80)
const float fogScatteringAniso = 0.0;     // Henyey-Greenstein g (-1..1), 0 = isotropic
const vec3  fogBaseColor = vec3(0.0,0.0,0.0);           // colour of the fog when no light hits it
const float fogLightIntensity = 2.0;      // multiplier for light contribution inside fog
const int LIGHT_DATA_SIZE = 13;
const int LIGHT_DATA_OFFSET_POSX = 0;
const int LIGHT_DATA_OFFSET_POSY = 1;
const int LIGHT_DATA_OFFSET_POSZ = 2;
const int LIGHT_DATA_OFFSET_INTENSITY = 3;
const int LIGHT_DATA_OFFSET_RANGE = 4;
const int LIGHT_DATA_OFFSET_SPOTANG = 5;
const int LIGHT_DATA_OFFSET_SPOTDIRX = 6;
const int LIGHT_DATA_OFFSET_SPOTDIRY = 7;
const int LIGHT_DATA_OFFSET_SPOTDIRZ = 8;
const int LIGHT_DATA_OFFSET_SPOTDIRW = 9;
const int LIGHT_DATA_OFFSET_R = 10;
const int LIGHT_DATA_OFFSET_G = 11;
const int LIGHT_DATA_OFFSET_B = 12;
const float WORLDCELL_WIDTH_F = 2.56;
const float VOXEL_SIZE = 0.32;

const float PI = 3.14159265359;

const float aaThreshold = 0.2;

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
    for (int x = -1; x <= 1; x++) {
        vec2 neighbor = vec2(float(x), float(-1));
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

    for (int x = -1; x <= 1; x++) {
        vec2 neighbor = vec2(float(x), float(0));
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

    for (int x = -1; x <= 1; x++) {
        vec2 neighbor = vec2(float(x), float(1));
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

vec3 vhsBlur(vec2 uv) { // Subtly soften everything for more realism.  Subtle but really does help.
    const float r = vhsRadiusMax * vhsBlurAmount;
    if (r < 0.5) return texture(tex, uv).rgb;

    vec2 px = 1.0 / vec2(screenWidth, screenHeight);
    vec3 acc = vec3(0.0);
    float wsum = 0.0;

    // 7-tap horizontal (same weights as original)
    const float w[7] = float[](0.05,0.12,0.20,0.26,0.20,0.12,0.05);
    for (int i=0;i<7;i++) {
        float o = (i-3) * (r/3.0);
        vec3 s = texture(tex, uv + vec2(o*px.x,0.0)).rgb;
        acc += s * w[i];
        wsum += w[i];
    }
    acc /= wsum;

    // 3-tap vertical (same as original)
    acc += texture(tex, uv + vec2(0.0,-px.y)).rgb * 0.25;
    acc += texture(tex, uv + vec2(0.0, px.y)).rgb * 0.25;
    return acc * (1.0/1.5); // renormalize
}

vec3 bandedStatic(vec2 uv) {
    float baseThickness = clamp(staticBandThickness, 0.001, 1.0);
    float screenY = uv.y * float(screenHeight);
    float scroll = timeVal * staticScrollSpeed;
    float thicknessPx = baseThickness * float(screenHeight);
    float bandCoord = (screenY + scroll) / thicknessPx;
    float bandIndex = floor(bandCoord);
    float cluster = snoise(vec2(bandIndex * 0.05, timeVal * 5.2));
    float minGap = 0.1; // min empty space fraction
    float maxGap = 0.9; // max empty space fraction
    float gapFrac = mix(maxGap, minGap, staticIntensity); 
    cluster = smoothstep(0.0, 1.0, cluster);
    cluster = step(gapFrac, cluster); // band visible if cluster > gapFrac
    float speck = snoise(uv * 200.0 + timeVal * 10.0) * 0.5 + 0.5;
    float bandNoiseVal = snoise(vec2(bandIndex * 0.3, uv.x * 5.0 + timeVal * 0.1));
    float intensity = mix(bandNoiseVal, speck, 0.6) * cluster;
    return staticColor * intensity;
}

uint GetVoxelIndex(vec3 worldPos) {
    float offsetX = worldPos.x - worldMin_x + (VOXEL_SIZE * 0.5);
    float offsetZ = worldPos.z - worldMin_z + (VOXEL_SIZE * 0.5);
    uint cellX = uint(offsetX / WORLDCELL_WIDTH_F);
    uint cellZ = uint(offsetZ / WORLDCELL_WIDTH_F);
    float localX = mod(offsetX, WORLDCELL_WIDTH_F);
    float localZ = mod(offsetZ, WORLDCELL_WIDTH_F);
    uint voxelX = uint(localX / VOXEL_SIZE);
    uint voxelZ = uint(localZ / VOXEL_SIZE);
    uint cellIndex = cellZ * 64 + cellX;
    uint voxelIndexInCell = voxelZ * 8 + voxelX;
    return cellIndex * 64 + voxelIndexInCell;
}

vec4 unpackColor32(uint color) {
    return vec4(float((color >> 24) & 0xFFu) / 255.0,  // r
                float((color >> 16) & 0xFFu) / 255.0,  // g
                float((color >>  8) & 0xFFu) / 255.0,  // b
                float((color      ) & 0xFFu) / 255.0); // a
}

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
    vec3 rgb = abs(mod(c.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0;
    rgb = clamp(rgb, 0.0, 1.0);
    return c.z * mix(vec3(1.0), rgb, c.y);
}

vec3 applyBerserk(vec3 worldPos, vec3 base) {
    if (berserkTimeRemaining <= 0.0) return base;

    float prog = clamp(1.0 - berserkTimeRemaining, 0.0, 1.0);
    float seed = fract(sin(berserkSeedTimestamp * 91.7) * 43758.5453);
    float hueBase = mix(0.15, 0.75, seed);
    float hueShift = mix(0.0, hueBase, smoothstep(0.0, 1.0, prog * 0.8) + 0.2);
    float n = fract(sin(dot(worldPos, vec3(12.9898, 78.233, 37.719))) * 43758.5453);
    float d = length(worldPos - camPos) * 0.1;
    float dn = fract(sin(d + n * 37.719) * 15731.743);
    float coverage = mix(dn, 1.0, smoothstep(0.0, 1.0, prog));
    float coverageMask = smoothstep(0.0, 1.0, (coverage - n) * 4.0);

    vec3 hsv = rgb2hsv(base);
    hsv.x = fract(hsv.x + hueShift + n * 0.2);
    float fadeIn = smoothstep(0.0, 0.05, prog);
    float fadeOut = smoothstep(0.0, 0.025, berserkTimeRemaining);

    vec3 berserkColor = mix(base, hsv2rgb(hsv), coverageMask * fadeIn * fadeOut);

    // Inversion fade in final throes
    float invertFade = smoothstep(0.25, 0.22, berserkTimeRemaining) * fadeOut;
    vec3 inverted = vec3(1.0) - berserkColor;
    inverted *= inverted * 1.5;
    return mix(berserkColor, inverted * berserkColor * 1.5, invertFade);
}

const float reflectionWeights[25] = float[](
    0.00390625, 0.015625, 0.0234375, 0.015625, 0.00390625,
    0.015625,   0.0625,   0.09375,   0.0625,   0.015625,
    0.0234375,  0.09375,  0.140625,  0.09375,  0.0234375,
    0.015625,   0.0625,   0.09375,   0.0625,   0.015625,
    0.00390625, 0.015625, 0.0234375, 0.015625, 0.00390625
);

ivec2 reflectionOffsets[25] = ivec2[](
    ivec2(-2, -2), ivec2(-1, -2), ivec2(0, -2), ivec2(1, -2), ivec2(2,-2),
    ivec2(-2, -1), ivec2(-1, -1), ivec2(0, -1), ivec2(1, -1), ivec2(2,-1),
    ivec2(-2, 0), ivec2(-1, 0), ivec2(0, 0), ivec2(1, 0), ivec2(2,0),
    ivec2(-2, 1), ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1), ivec2(2, 1),
    ivec2(-2, 2), ivec2(-1, 2), ivec2(0, 2), ivec2(1, 2), ivec2(2, 2)
);

void main() {
    vec2 texCoordUsed = TexCoord;
    if (empEffectActive > 0u) texCoordUsed.y += timeVal * 15.0;
    vec4 color = texture(tex, texCoordUsed).rgba;
    bool isSky = false;
    if (skyVisible > 0) {
        isSky = (color.a > 0.19 && color.a < 0.21); // Sky hack alpha
        float mappedLat = 0.0;
        if (isSky) {
            vec2 ndc = texCoordUsed * 2.0 - 1.0;
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
            vec3 shieldColor = vec3(0.0, 0.0, 0.0);
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
                if (stationShieldVisible >= 2) { // Level is above shield
                    float shieldDot = dot(skyDir, vec3(0.0,-1.0,0.0));
                    if (shieldDot < 0.4) shieldColor = vec3(0.0,0.0,0.0);
                }
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
    }

    ivec2 uv = ivec2(gl_FragCoord.xy);                // pixel centre
    vec4 fog = vec4(0.0,0.0,0.0,0.0);
    vec4 wpPack = vec4(0.0,0.0,0.0,0.0);
    vec3 surfPos = vec3(0.0,0.0,0.0);
    vec4 specColor = vec4(0.0,0.0,0.0,0.0);
    if (volumetricFogEnabled > 0 || reflectionsEnabled > 0 || berserkTimeRemaining > 0.0) {
        wpPack = imageLoad(inputWorldPos, uv);
        vec2 worldXY = unpackHalf2x16(floatBitsToUint(wpPack.r));
        vec2 worldZInst = unpackHalf2x16(floatBitsToUint(wpPack.g));
        surfPos = vec3(worldXY.x, worldXY.y, worldZInst.x);
    }

    if (reflectionsEnabled > 0) specColor = unpackColor32(floatBitsToUint(wpPack.a));

    if (volumetricFogEnabled > 0) {
        float surfDepth = length(surfPos - camPos);            // distance to geometry
        float tanHalf = tan(radians(fov) * 0.5);
        vec2 ndc = ((vec2(uv) + 0.5) / vec2(screenWidth, screenHeight)) * 2.0 - 1.0;
        ndc.x *= aspect;
        vec3 viewDir = normalize(invViewRot * normalize(vec3(ndc * tanHalf, -1.0)));
        const float stepLen = fogStepSize;
        vec3  accumScat   = vec3(0.0);
        float accumTrans  = 1.0;
        float marchedDist = 0.0;
        vec3  pos         = camPos;
        for (uint i = 0u; i < fogMaxSteps; ++i) {
            marchedDist += stepLen;
            if (marchedDist >= surfDepth) break;

            pos = camPos + viewDir * marchedDist;
            float density = fogDensity;
            vec3 lightCol;
            uint voxelIdx = GetVoxelIndex(pos);
            uint count    = min(voxelLightListIndices[voxelIdx * 2 + 1], 16u);
            uint listoffset  = (count > 0) ? voxelLightListIndices[voxelIdx * 2] : 0u;
            lightCol = vec3(0.0);
            for (uint i = 0u; i < count; ++i) {
                uint lightIdxInPVS = uniqueLightLists[listoffset + i];
                uint lightIdx = lightIdxInPVS * uint(LIGHT_DATA_SIZE);
                vec3  Lpos   = vec3(lights[lightIdx + LIGHT_DATA_OFFSET_POSX],
                                    lights[lightIdx + LIGHT_DATA_OFFSET_POSY],
                                    lights[lightIdx + LIGHT_DATA_OFFSET_POSZ]);
                float intensity = lights[lightIdx + LIGHT_DATA_OFFSET_INTENSITY];
                if (intensity < 0.05) continue;

                float range = lights[lightIdx + LIGHT_DATA_OFFSET_RANGE];
                vec3  toLight = Lpos - pos;
                float dist = length(toLight);
                if (dist > range) continue;

                vec3 lightDir = normalize(toLight);
                float lambertian = 1.0; // No normal to compare against
                float distOverRange = dist / range;
                float rangeFacSqrd = 1.0 - (distOverRange * distOverRange);
                float attenuation = rangeFacSqrd * lambertian;
                attenuation = attenuation * attenuation;                     // quadratic fall-off
                float shadowFactor = 1.0;
                if (shadowsEnabled > 0) {
                    float smearness = attenuation * attenuation * 38.0;
                    float bias = clamp(((0.125 * (1.0 - attenuation) * (1.0 - attenuation))) - 0.02,0.01,1.0);
                    float normalBias = 0.04;
                    vec3 a = abs(-toLight);
                    float maxAxis = max(max(a.x, a.y), a.z);
                    float invMax = (maxAxis > 0.0) ? (1.0 / maxAxis) : 0.0;  // avoid division by zero
                    vec3 dir = -toLight * invMax;
                    uint face;
                    vec2 uv;
                    if (a.x >= a.y && a.x >= a.z) {
                        face = -toLight.x > 0.0 ? 0u : 1u; uv = (face == 0u) ? vec2(-dir.z, dir.y) : vec2(dir.z, dir.y);
                    } else if (a.y >= a.x && a.y >= a.z) {
                        face = -toLight.y > 0.0 ? 2u : 3u; uv = (face == 2u) ? vec2(dir.x, -dir.z) : vec2(dir.x, dir.z);
                    } else {
                        face = -toLight.z > 0.0 ? 4u : 5u; uv = (face == 4u) ? vec2(dir.x, dir.y) : vec2(-dir.x, dir.y);
                    }

                    uv = uv * 0.5 + 0.5;
                    uint base = lightIdxInPVS * 6u * uint(shadowmapSize) * uint(shadowmapSize);
                    uint faceOff = base + face * uint(shadowmapSize) * uint(shadowmapSize);
                    vec2 tc = uv * shadowmapSize;
                    float tx = clamp(tc.x, 0.0, shadowmapSize - 1.0);
                    float ty = clamp(tc.y, 0.0, shadowmapSize - 1.0);
                    uint utx = uint(tx);
                    uint uty = uint(ty);
                    uint ssbo_index = faceOff + uty * uint(shadowmapSize) + utx;
                    uint distInt = shadowMaps[ssbo_index];
                    float d = float(distInt) / 10000.0;
                    float depthDiff = dist - d - (bias + normalBias);
                    shadowFactor = depthDiff > 0.0 ? 0.0 : 1.0;
                    if (shadowFactor < 0.005) continue;
                }

                vec3 Lcol = vec3(lights[lightIdx + LIGHT_DATA_OFFSET_R],
                                lights[lightIdx + LIGHT_DATA_OFFSET_G],
                                lights[lightIdx + LIGHT_DATA_OFFSET_B]);

                lightCol += Lcol * intensity * attenuation * shadowFactor;
            }

            float phase = 1.0 / (4.0 * PI);
            vec3 scat = lightCol * phase * fogLightIntensity;

            // Beer-Lambert
            float extinction     = density * stepLen;
            float transmittance  = exp(-extinction);
            accumScat  += accumTrans * scat * density * stepLen;
            accumTrans *= transmittance;
            if (accumTrans < 0.01) break;            // early-out
        }

        vec3 fogColor = clamp(accumScat + fogBaseColor * accumTrans,0.0,1.0);
        fog = vec4(fogColor, 1.0 - accumTrans);       // rgb = fog, a = opacity
    }

    if (!isSky) color.rgb = mix(color.rgb, fog.rgb, fog.a);
    if (debugValue > 0) { FragColor = vec4(color.rgb, 1.0); return; }

    ivec2 pixel = ivec2(texCoordUsed * vec2(screenWidth/SSR_RES, screenHeight/SSR_RES));
    if (debugView != 4) {
        if (reflectionsEnabled > 0) {
            vec2 sampleUV = (vec2(pixel)) / vec2(screenWidth/SSR_RES, screenHeight/SSR_RES);
            vec4 reflectionColor = vec4(0.0);
//             vec3 reflectionColor = texture(outputImage, sampleUV).rgb * specColor.rgb * 1.4;
            for (int i = 0; i < 9; ++i) {
                ivec2 samplePixel = pixel + reflectionOffsets[i];
                samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
                vec3 sampleWeight = vec3(reflectionWeights[i],reflectionWeights[i],reflectionWeights[i]);
//                 reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 6.0;
                reflectionColor.rgb += texture(outputImage, sampleUV).rgb * specColor.rgb * sampleWeight * 6.0;
            }
            if (isSky) {
                FragColor.rgb += reflectionColor.rgb;
                return;
            }

            color.rgb += reflectionColor.rgb;
        }

        vec3 aaColor = color.rgb; // Default to chromatic aberration result
        if (aaEnabled > 0) {
            // SMAA-Inspired Edge-Directed Antialiasing
            // Compute luminance for edge detection
            vec2 pixelSize = vec2(1.0 / float(screenWidth), 1.0 / float(screenHeight));
            vec3 centerColor = texture(tex, texCoordUsed).rgb;
            float lumaCenter = dot(centerColor, vec3(0.299, 0.587, 0.114)); // Luminance (Rec. 601)
            vec3 dx = texture(tex, texCoordUsed + vec2(pixelSize.x, 0.0)).rgb - texture(tex, texCoordUsed - vec2(pixelSize.x, 0.0)).rgb;
            vec3 dy = texture(tex, texCoordUsed + vec2(0.0, pixelSize.y)).rgb - texture(tex, texCoordUsed - vec2(0.0, pixelSize.y)).rgb;
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
                    vec2 sampleUV = texCoordUsed + orthoDir * dist * pixelSize;
                    sampleColor += texture(tex, sampleUV).rgb * weight;
                    aaWeightSum += weight;
                }
                sampleColor /= aaWeightSum;

                // Dynamic blending based on edge contrast
                float blendFactor = clamp(gradientMag * 0.5, 2.0, 4.0); // Adjust blend based on edge strength
                aaColor = mix(color.rgb, sampleColor, blendFactor);
            }
        }

        // VHS Blur
        if (vhsBlurAmount > 0.0) {
            vec3 blurred = vhsBlur(texCoordUsed);
            aaColor = mix(aaColor, blurred, clamp(vhsBlurAmount, 0.0, 1.0));
        }

        // Banded Static
        if (staticIntensity > 0.0) aaColor += bandedStatic(texCoordUsed);

        // Gamma/Brightness Setting
        aaColor.rgb = pow(aaColor.rgb, vec3(1.0 / (float(brightnessSetting) / 100.0)));

        // Berserk last as it's a brain effect not an eye effect
        if (berserkTimeRemaining > 0.0) aaColor = applyBerserk(surfPos, aaColor);
        FragColor = vec4(aaColor, 1.0);
    } else {
        vec2 sampleUV = (vec2(pixel) + 0.5) / vec2(screenWidth/SSR_RES, screenHeight/SSR_RES);
        FragColor = texture(outputImage, sampleUV);
    }
}
