// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
#version 430 core
#extension GL_ARB_shading_language_packing : require
#extension GL_ARB_shader_image_load_store : enable
layout(early_fragment_tests) in;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

layout(location =  0) uniform uint instanceIndex; // start vert shader uniforms
layout(location =  1) uniform uint normInstanceIndex;
layout(location =  2) uniform mat4 viewProjection;
layout(location =  3) uniform uint isUI; // end vert shader uniforms
layout(location =  4) uniform int debugView;
layout(location =  5) uniform int debugValue;
layout(location =  6) uniform uint screenWidth;
layout(location =  7) uniform uint screenHeight;
layout(location =  8) uniform float worldMin_x;
layout(location =  9) uniform float worldMin_z;
layout(location = 10) uniform vec3 camPos;
layout(location = 11) uniform float fogColorR;
layout(location = 12) uniform float fogColorG;
layout(location = 13) uniform float fogColorB;
layout(location = 14) uniform uint reflectionsEnabled;
layout(location = 15) uniform uint shadowsEnabled;
layout(location = 16) uniform float shadowmapSize;
layout(location = 17) uniform uint unlit;
layout(location = 18) uniform uint texIndex;
layout(location = 19) uniform uint glowIndex;
layout(location = 20) uniform uint specIndex;

layout(location = 0) out vec4 outAlbedo;   // GL_COLOR_ATTACHMENT0
layout(location = 1) out vec4 outWorldPos; // GL_COLOR_ATTACHMENT1
layout(location = 2) out vec4 outSpecular; // GL_COLOR_ATTACHMENT2
layout(std430, binding = 5) buffer ShadowMaps { uint shadowMaps[]; };
layout(std430, binding = 8) buffer ShadowMapsIndirection { uint shadowMapsIndirection[]; };
layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; }; // x,y pairs for width and height of textures
layout(std430, binding = 16) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors
layout(std430, binding = 17) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; }; // Palette starting indices for each texture
layout(std430, binding = 19) buffer LightIndices { float lights[]; };
layout(std430, binding = 26) buffer VoxelLightListIndices { uint voxelLightListIndices[]; };
layout(std430, binding = 27) buffer UniqueLightLists { uint uniqueLightLists[]; };

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
const vec3 baseDir = vec3(0.0, 0.0, 1.0);

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

const float INV_FOG_DIST = 1.0 / 71.68;
const int PCF_SAMPLES = 6;
const vec2 poissonDisk[PCF_SAMPLES] = vec2[](
    vec2(-0.0326212f, -0.0405810f),
    vec2(-0.0840144f, -0.0073580f),
    vec2(-0.0695914f,  0.0457137f),
    vec2(-0.0203345f,  0.0620716f),
    vec2( 0.0962340f, -0.0194983f),
    vec2( 0.0473434f, -0.0480026f));

vec3 quat_rotate(vec4 q, vec3 v) {
    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;
    float xx2 = q.x * x2;
    float yy2 = q.y * y2;
    float zz2 = q.z * z2;
    float xy2 = q.x * y2;
    float xz2 = q.x * z2;
    float yz2 = q.y * z2;
    float wx2 = q.w * x2;
    float wy2 = q.w * y2;
    float wz2 = q.w * z2;
    return vec3(
        v.x * (1.0 - yy2 - zz2) + v.y * (xy2 - wz2) + v.z * (xz2 + wy2),
        v.x * (xy2 + wz2) + v.y * (1.0 - xx2 - zz2) + v.z * (yz2 - wx2),
        v.x * (xz2 - wy2) + v.y * (yz2 + wx2) + v.z * (1.0 - xx2 - yy2)
    );
}

vec4 getTextureColor(uint texIndex, ivec2 texCoord) {
    uint pixelOffset = textureOffsets[texIndex] + uint(texCoord.y) * textureSizes[texIndex].x + uint(texCoord.x);
    uint slotIndex = pixelOffset / 4u;
    uint packedIdx = colors[slotIndex];
    uint localOffset = pixelOffset % 4u;
    uint paletteIndex = (packedIdx >> (8u * localOffset)) & 0xFFu;
    uint paletteOffset = texturePaletteOffsets[texIndex];
    uint color = texturePalettes[paletteOffset + paletteIndex];
    return vec4(
        float(color & 0xFFu) / 255.0,
        float((color >> 8u) & 0xFFu) / 255.0,
        float((color >> 16u) & 0xFFu) / 255.0,
        float((color >> 24u) & 0xFFu) / 255.0
    );
}

uint packColor(vec4 color) {
    uvec4 c = uvec4(clamp(color * 255.0, 0.0, 255.0));
    return (c.r << 24) | (c.g << 16) | (c.b << 8) | c.a;
}

// 3D Simplex noise
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 permute(vec4 x) { return mod(((x*34.0)+1.0)*x, 289.0); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float snoise3D(vec3 v) {
    const vec2 C = vec2(1.0/6.0, 1.0/3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy;      // -1.0 + 3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod(i, 289.0);
    vec4 p = permute(permute(permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0));

    // Gradients: 7x7x6 points over a cube
    vec4 j = p - 49.0 * floor(p / 49.0);  // mod(p, 7*7)

    vec4 x_ = floor(j / 7.0);
    vec4 y_ = floor(j - 7.0 * x_);
    vec4 x = x_ * (1.0/7.0) + 0.5;  // normalize to [0.5, 1.5)
    vec4 y = y_ * (1.0/7.0) + 0.5;

    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(b0.zw, h.z);
    vec3 p3 = vec3(b1.xy, h.w);

    // Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2,p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix contributions
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m*m*m*m;

    float n = 42.0 * dot(m, vec4(dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3)));

    return n; // ≈ [-1, 1]
}

void main() {
    vec3 worldPos = FragPos.xyz;
    vec3 viewDir = (camPos - worldPos);
    float distToPixel = length(viewDir);
    viewDir = normalize(viewDir);
    int texIndexChecked = 0;
    if (texIndex >= 0) texIndexChecked = int(texIndex);
    ivec2 texSize = textureSizes[texIndexChecked];
    vec2 uv = (vec2(TexCoord.x, 1.0 - TexCoord.y)); // Invert V (aka Y), OpenGL convention vs import
    ivec2 texUV = ivec2(int(floor(uv.x * float(texSize.x))), int(floor(uv.y * float(texSize.y))));
    texUV.x = texUV.x % texSize.x;
    texUV.y = texUV.y % texSize.y;
    vec4 albedoColor = getTextureColor(texIndexChecked,texUV);
    if (albedoColor.a < 0.05) discard; // Alpha cutout threshold

    if (texIndexChecked == 1230) albedoColor.a = 0.20;
    vec3 adjustedNormal = Normal;
    if (normInstanceIndex != 0 && debugValue < 1 && distToPixel < 10.24) {
        vec3 dp1 = dFdx(FragPos);
        vec3 dp2 = dFdy(FragPos);
        vec2 duv1 = dFdx(TexCoord);
        vec2 duv2 = dFdy(TexCoord);
        float uvArea = abs(duv1.x * duv2.y - duv1.y * duv2.x);
        if (uvArea > 0.0000001) {
            vec3 t = normalize(dp1 * duv2.y - dp2 * duv1.y);
            vec3 b = normalize(dp1 * duv2.x - dp2 * duv1.x);
            mat3 TBN3x3 = mat3(t, b, adjustedNormal);
            ivec2 texSizeNorm = textureSizes[normInstanceIndex];
            ivec2 texUVNorm = ivec2(int(floor(uv.x * float(texSizeNorm.x))), int(floor(uv.y * float(texSizeNorm.y))));
            texUVNorm.x = texUVNorm.x % texSizeNorm.x;
            texUVNorm.y = texUVNorm.y % texSizeNorm.y;
            vec3 normalColor = (getTextureColor(normInstanceIndex,texUVNorm).rgb * 2.0 - 1.0);
            normalColor.g = -normalColor.g;
            adjustedNormal = normalize(TBN3x3 * normalColor);
            if (dot(adjustedNormal,Normal) < 0.0) adjustedNormal = Normal;
        }
    }

    vec4 glowColor = vec4(0.0,0.0,0.0,0.0);
    if (glowIndex != 0) {
        ivec2 texSizeGlow = textureSizes[glowIndex];
        ivec2 texUVGlow = ivec2(int(floor(uv.x * float(texSizeGlow.x))), int(floor(uv.y * float(texSizeGlow.y))));
        texUVGlow.x = texUVGlow.x % texSizeGlow.x;
        texUVGlow.y = texUVGlow.y % texSizeGlow.y;
        glowColor = getTextureColor(glowIndex,texUVGlow);
    }

    vec4 specColor = vec4(0.0,0.0,0.0,0.0);
    if (reflectionsEnabled > 0) {
        vec4 normalPack = vec4((adjustedNormal.x + 1.0) * 0.5,(adjustedNormal.y + 1.0) * 0.5,(adjustedNormal.z + 1.0) * 0.5,0.0);
        ivec2 texSizeSpec = textureSizes[specIndex];
        ivec2 texUVSpec = ivec2(int(floor(uv.x * float(texSizeSpec.x))),int(floor(uv.y * float(texSizeSpec.y))));
        texUVSpec.x = texUVSpec.x % texSizeSpec.x;
        texUVSpec.y = texUVSpec.y % texSizeSpec.y;
        specColor = getTextureColor(specIndex,texUVSpec);
        vec4 worldPosPack = vec4(FragPos.xyz, uintBitsToFloat(packColor(normalPack)));
        outWorldPos = worldPosPack;
        outSpecular = specColor; 
    }

    uint voxelIdx = GetVoxelIndex(worldPos);
    uint count = voxelLightListIndices[voxelIdx * 2 + 1];
    if (unlit > 0) count = 0;
    vec3 lighting = vec3(0.0, 0.0, 0.0);
    vec3 normal = adjustedNormal;
    uint listoffset = 0;
    if (count > 0) listoffset = voxelLightListIndices[voxelIdx * 2];
    vec3 noiseCoord = worldPos;
    float n1 = snoise3D(noiseCoord);
    float n2 = snoise3D(noiseCoord * 2.03) * 0.5;   // half strength, double freq
    for (uint i = 0u; i < count; i++) {
        uint lightIdxInPVS = uniqueLightLists[listoffset + i];
        uint lightIdx = lightIdxInPVS * uint(LIGHT_DATA_SIZE);
        vec3 lightPos = vec3(lights[lightIdx + LIGHT_DATA_OFFSET_POSX], lights[lightIdx + LIGHT_DATA_OFFSET_POSY], lights[lightIdx + LIGHT_DATA_OFFSET_POSZ]);
        float intensity = lights[lightIdx + LIGHT_DATA_OFFSET_INTENSITY];
        if (intensity < 0.05) continue;

        float range = lights[lightIdx + LIGHT_DATA_OFFSET_RANGE];
        vec3 toLight = lightPos - worldPos;
        float dist = length(toLight);
        if (dist > range) continue;

        vec3 lightDir = normalize(toLight);
        float lambertian = clamp(max(dot(normal, lightDir), 0.0),0.0,1.0);
        float spotAng = lights[lightIdx + LIGHT_DATA_OFFSET_SPOTANG];
        float spotFalloff = 1.0;
        if (spotAng > 0.0) { // Extremely rare, only ~15 spot lights in entire game out of several thousand lights.
            float quat_x = lights[lightIdx + LIGHT_DATA_OFFSET_SPOTDIRX];
            float quat_y = lights[lightIdx + LIGHT_DATA_OFFSET_SPOTDIRY];
            float quat_z = lights[lightIdx + LIGHT_DATA_OFFSET_SPOTDIRZ];
            float quat_w = lights[lightIdx + LIGHT_DATA_OFFSET_SPOTDIRW];
            vec4 quat = vec4(quat_x, quat_y, quat_z, quat_w);
            vec3 spotDir = normalize(quat_rotate(quat, baseDir));
            float spotdot = dot(spotDir, -lightDir);
            float cosAngle = cos(radians(spotAng / 2.0));
            if (spotdot < cosAngle) continue;
            
            float cosOuterAngle = cos(radians(spotAng / 2.0));
            float cosInnerAngle = cos(radians(spotAng * 0.8 / 2.0));
            spotFalloff = smoothstep(cosOuterAngle, cosInnerAngle, spotdot);
            if (spotFalloff <= 0.0) continue;
        }

        float distOverRange = dist / range;
        float distOverRangeSqd = distOverRange * distOverRange;
        float attenuation = (1.0 - distOverRangeSqd) * lambertian;
        float shadowFactor = 1.0;
        uint shadowIndex = shadowMapsIndirection[lightIdxInPVS];
        if (debugValue != 2 && shadowsEnabled > 0 && shadowIndex < 1600) {
            float smearness = distOverRange * distOverRange * 24.0 + 12.0;
            vec3 a = abs(toLight);
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
            uint shadSizeSquared = uint(shadowmapSize) * uint(shadowmapSize);
            uint faceOff = shadowIndex * 6u * shadSizeSquared + face * shadSizeSquared;
            vec2 tc = uv * shadowmapSize;
            float bias = 0.02 + 0.17 * distOverRangeSqd;
            if (shadowsEnabled > 1 && distToPixel < 24.0) {
                // Pseudo-Stochastic PCF sampling
                float sum = 0.0;
                float invSamples = 1.0 / float(PCF_SAMPLES);
                for (int si = 0; si < PCF_SAMPLES; ++si) {
                    vec2 off = poissonDisk[si] * smearness;
                    vec2 t = tc + off;
                    float tx = clamp(t.x, 0.0, shadowmapSize - 1.0); // Minus 1 prevents tiny gaps
                    float ty = clamp(t.y, 0.0, shadowmapSize - 1.0);
                    uint utx = uint(tx);
                    uint uty = uint(ty);
                    uint ssbo_index = faceOff + uty * uint(shadowmapSize) + utx;
                    uint distInt = shadowMaps[ssbo_index];
                    float d = (float(distInt) / 100000.0);
                    float depthDiff = (dist) - d - bias;
                    float shadowContrib = clamp(1.0 - depthDiff / 0.005, 0.0, 1.0);
                    sum += shadowContrib * invSamples;
                }

                shadowFactor = sum;
            } else {
                float tx = clamp(tc.x, 0.0, shadowmapSize - 1.0); // Minus 1 prevents tiny gaps
                float ty = clamp(tc.y, 0.0, shadowmapSize - 1.0);
                uint utx = uint(tx);
                uint uty = uint(ty);
                uint ssbo_index = faceOff + uty * uint(shadowmapSize) + utx;
                uint distInt = shadowMaps[ssbo_index];
                float d = (float(distInt) / 100000.0);
                float depthDiff = (dist) - d - bias;
                shadowFactor = clamp(1.0 - depthDiff / 0.005, 0.0, 1.0);
            }
        }

        vec3 lightColor = vec3(lights[lightIdx + LIGHT_DATA_OFFSET_R], lights[lightIdx + LIGHT_DATA_OFFSET_G], lights[lightIdx + LIGHT_DATA_OFFSET_B]);
        lighting += (albedoColor.rgb * intensity * pow(attenuation, 2.2) * lightColor * spotFalloff * shadowFactor);
        if (specColor.r > 0.0 || specColor.g > 0.0 || specColor.b > 0.0) {
            vec3 halfDir = normalize(lightDir + viewDir);
            float ndh = max(dot(normal, halfDir), 0.0);
            float strength = texIndexChecked == 36 || texIndexChecked == 887 ? 1.0 : max(specColor.r, max(specColor.g, specColor.b)) * 0.451;
            float shininess = 130.0;
            float spec = clamp(pow(ndh, shininess),0.0,1.0);
            lighting += specColor.rgb * intensity * attenuation * spotFalloff * spec * shadowFactor * strength * 10.0;
        }
    }

    if (unlit > 0) lighting = albedoColor.rgb;
    else lighting += glowColor.rgb; // Glow (texture emission)

    float fogFac = clamp(distToPixel * INV_FOG_DIST, 0.0, 1.0);
    float lum = dot(lighting, vec3(0.299, 0.587, 0.114));
    vec3 fogColor = vec3(fogColorR, fogColorG, fogColorB);
    fogFac = clamp(fogFac * (1.0 - lum), 0.0, 1.0);
    if (unlit == 0) lighting = mix(fogColor, lighting, 1.0 - fogFac); // Fog
    if (debugView == 1) {
        outAlbedo = albedoColor;
        outAlbedo.a = 1.0;
    } else if (debugView == 2) {
        outAlbedo.r = (adjustedNormal.x + 1.0) * 0.5f;
        outAlbedo.g = (adjustedNormal.y + 1.0) * 0.5f;
        outAlbedo.b = (adjustedNormal.z + 1.0) * 0.5f;
        outAlbedo.a = 1.0;
    } else if (debugView == 3) {
        float ndcDepth = (2.0 * gl_FragCoord.z - 1.0); // Depth debug
        float clipDepth = ndcDepth / gl_FragCoord.w;
        float linearDepth = (clipDepth - 0.02) / (71.68 - 0.02);
        outAlbedo = vec4(vec3(linearDepth), 1.0);
    } else {
        outAlbedo = vec4(lighting.rgb, albedoColor.a);
    }
}
