// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
#version 450 core
#extension GL_ARB_shading_language_packing : require

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 Barycentric;

uniform int debugView;
uniform int debugValue;
uniform uint screenWidth;
uniform uint screenHeight;
uniform float worldMin_x;
uniform float worldMin_z;
uniform vec3 camPos;
uniform float fogColorR;
uniform float fogColorG;
uniform float fogColorB;
uniform uint reflectionsEnabled;
uniform uint shadowsEnabled;
uniform uint ditherEnabled;

flat in uint TexIndex;
flat in uint GlowIndex;
flat in uint SpecIndex;
flat in uint NormalIndex;

layout(location = 0) out vec4 outAlbedo;   // GL_COLOR_ATTACHMENT0
layout(location = 1) out vec4 outWorldPos; // GL_COLOR_ATTACHMENT1
layout(std430, binding = 5) buffer ShadowMaps { float shadowMaps[]; };
layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
// layout(std430, binding = 13) buffer BlueNoise { float blueNoiseColors[]; };
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

const float SHADOW_MAP_SIZE = 256.0;
const float INV_FOG_DIST = 1.0 / 71.68;

// Small Poisson disk for stochastic PCF.
// 12 samples gives good quality when temporally accumulated.
const int PCF_SAMPLES = 6;
const vec2 poissonDisk[6] = vec2[](
    vec2(-0.04, -0.04), vec2(0.07, -0.07), vec2(-0.10, -0.10),
    vec2(0.04, 0.04), vec2(0.07, 0.07), vec2(0.10, 0.10)
);

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
    uint pixelOffset = textureOffsets[texIndex] + texCoord.y * textureSizes[texIndex].x + texCoord.x;
    uint slotIndex = pixelOffset / 2;
    uint packedIdx = colors[slotIndex];
    uint paletteIndex = (pixelOffset % 2 == 0) ? (packedIdx & 0xFFFFu) : (packedIdx >> 16);
    uint paletteOffset = texturePaletteOffsets[texIndex];
    uint color = texturePalettes[paletteOffset + paletteIndex];
    return vec4(
        float((color >> 24) & 0xFFu) / 255.0,
        float((color >> 16) & 0xFFu) / 255.0,
        float((color >> 8) & 0xFFu) / 255.0,
        float(color & 0xFFu) / 255.0
    );
}

uint packColor(vec4 color) {
    uvec4 c = uvec4(clamp(color * 255.0, 0.0, 255.0));
    return (c.r << 24) | (c.g << 16) | (c.b << 8) | c.a;
}

void main() {
    vec3 worldPos = FragPos.xyz;
    vec3 viewDir = (camPos - worldPos);
    float distToPixel = length(viewDir);
    if (distToPixel > 71.66) return;

    viewDir = normalize(viewDir);
    int texIndexChecked = 0;
    if (TexIndex >= 0) texIndexChecked = int(TexIndex); 
    ivec2 texSize = textureSizes[texIndexChecked];
    vec2 uv = clamp(vec2(TexCoord.x, 1.0 - TexCoord.y), 0.0, 1.0); // Invert V (aka Y), OpenGL convention vs import
    ivec2 pixel = ivec2(uv);
    int x = int(floor(uv.x * float(texSize.x)));
    int y = int(floor(uv.y * float(texSize.y)));
    ivec2 texUV = ivec2(x,y);
    vec4 albedoColor = getTextureColor(texIndexChecked,texUV);
    if (albedoColor.a < 0.05) discard; // Alpha cutout threshold

    vec3 adjustedNormal = Normal;

    // Bevel Shader: Compute distance to nearest edge using barycentric coordinates
    float bevelWidth = 0.02; // Adjust for bevel size
    float edgeDist = min(min(Barycentric.x, Barycentric.y), Barycentric.z); // Distance to nearest edge
    float bevelFactor = smoothstep(0.0, bevelWidth, edgeDist); // 0 near edge, 1 in center (fixed negative sign)

    // Compute geometric normal for flat surfaces
    vec3 dp1 = dFdx(FragPos);
    vec3 dp2 = dFdy(FragPos);
    vec3 geometricNormal = normalize(cross(dp1, dp2));

    // Estimate edge hardness by checking normal variance
    vec3 normalDx = dFdx(Normal);
    vec3 normalDy = dFdy(Normal);
    float normalVariance = length(normalDx) + length(normalDy); // Magnitude of normal gradient
    float hardnessThreshold = 0.9; // Adjust to control sensitivity (0.05 to 0.5)
    float edgeHardness = smoothstep(0.0, hardnessThreshold, normalVariance); // 1 for hard edges, 0 for smooth

    // Alternative: Compare vertex normal to geometric normal
    float normalDiff = 1.0 - dot(Normal, geometricNormal); // 0 if normals are similar, >0 if different
    float normalHardness = smoothstep(0.0, 0.2, normalDiff); // Adjust threshold (0.1 to 0.3)
    edgeHardness = max(edgeHardness, normalHardness); // Combine both metrics for robustness

    // Apply bevel only on hard edges
    vec3 edgeDir = normalize(dFdx(Barycentric) + dFdy(Barycentric)); // Approximate edge direction
    vec3 bevelDir = normalize(Normal + edgeDir * 0.5 + viewDir * 0.2); // Perturb normal
    adjustedNormal = normalize(mix(bevelDir, Normal, bevelFactor * edgeHardness)); // Modulate by edge hardness

    if (NormalIndex != 41) {
        vec3 dp1 = dFdx(FragPos);
        vec3 dp2 = dFdy(FragPos);
        vec2 duv1 = dFdx(TexCoord);
        vec2 duv2 = dFdy(TexCoord);
        float uvArea = abs(duv1.x * duv2.y - duv1.y * duv2.x);
        if (uvArea > 0.0000001) {
            vec3 t = normalize(dp1 * duv2.y - dp2 * duv1.y);
            vec3 b = normalize(-dp1 * duv2.x + dp2 * duv1.x);
            mat3 TBN3x3 = mat3(t, b, adjustedNormal);
            vec3 normalColor = (getTextureColor(NormalIndex,texUV).rgb * 2.0 - 1.0);
            normalColor.g = -normalColor.g;
            adjustedNormal = normalize(TBN3x3 * normalColor);
        }
    }

    vec4 glowColor = getTextureColor(GlowIndex,texUV);
    if (reflectionsEnabled > 0) {
        vec4 normalPack = vec4((adjustedNormal.x + 1.0) * 0.5,(adjustedNormal.y + 1.0) * 0.5,(adjustedNormal.z + 1.0) * 0.5,0.0);
        vec4 specColor = getTextureColor(SpecIndex,texUV);
        vec4 worldPosPack = vec4(uintBitsToFloat(packHalf2x16(FragPos.xy)),
                                uintBitsToFloat(packHalf2x16(vec2(FragPos.z,0.0))),
                                uintBitsToFloat(packColor(normalPack)),
                                uintBitsToFloat(packColor(specColor)) );
        outWorldPos = worldPosPack;
    }

    uint voxelIdx = GetVoxelIndex(worldPos);
    uint count  = min(voxelLightListIndices[voxelIdx * 2 + 1],16u);
    vec3 lighting = vec3(0.0, 0.0, 0.0);
    vec3 normal = adjustedNormal;
    uint listoffset = 0;
    if (count > 0) listoffset = voxelLightListIndices[voxelIdx * 2];
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
        float lambertian = max(dot(normal, lightDir), 0.0);
//         if (lambertian < 0.25) continue;

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
        float rangeFacSqrd = 1.0 - (distOverRange * distOverRange);
        float attenuation = rangeFacSqrd * lambertian;
        float shadowFactor = 1.0;
//         if (debugValue != 2 && shadowsEnabled > 0) {
        if (shadowsEnabled > 0) {
            float smearness = attenuation * attenuation * 38.0;
            float bias = clamp(((0.24 * (1.0 - attenuation) * (1.0 - attenuation))) - 0.02,0.025,1.0);
            float normalBias = 0.0;//clamp(0.01 * (1.0 - dot(normal, lightDir)), 0.0, 0.05);
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
            uint base = lightIdxInPVS * 6u * uint(SHADOW_MAP_SIZE) * uint(SHADOW_MAP_SIZE);
            uint faceOff = base + face * uint(SHADOW_MAP_SIZE) * uint(SHADOW_MAP_SIZE);
            vec2 tc = uv * SHADOW_MAP_SIZE;

            if (shadowsEnabled > 1 && distToPixel < 10.0) {
                // Pseudo-Stochastic PCF sampling
                float sum = 0.0;
                float invSamples = 1.0 / float(PCF_SAMPLES);
                for (int si = 0; si < PCF_SAMPLES; ++si) {
                    vec2 off = poissonDisk[si] * smearness;
                    vec2 t = tc + off;
                    float tx = clamp(t.x, 0.0, SHADOW_MAP_SIZE - 1.0);
                    float ty = clamp(t.y, 0.0, SHADOW_MAP_SIZE - 1.0);
                    uint utx = uint(tx);
                    uint uty = uint(ty);
                    uint ssbo_index = faceOff + uty * uint(SHADOW_MAP_SIZE) + utx;
                    float d = shadowMaps[ssbo_index];
                    float depthDiff = dist - d - (bias + normalBias);
                    float shadowContrib = depthDiff > 0.0 ? 0.0 : 1.0;
                    sum += shadowContrib * invSamples;
                }

                shadowFactor = sum;
            } else {
                float tx = clamp(tc.x, 0.0, SHADOW_MAP_SIZE - 1.0);
                float ty = clamp(tc.y, 0.0, SHADOW_MAP_SIZE - 1.0);
                uint utx = uint(tx);
                uint uty = uint(ty);
                uint ssbo_index = faceOff + uty * uint(SHADOW_MAP_SIZE) + utx;
                float d = shadowMaps[ssbo_index];
                float depthDiff = dist - d - (bias + normalBias);
                shadowFactor = depthDiff > 0.0 ? 0.0 : 1.0;
            }

            if (shadowFactor < 0.005) continue;
        }

        vec3 lightColor = vec3(lights[lightIdx + LIGHT_DATA_OFFSET_R], lights[lightIdx + LIGHT_DATA_OFFSET_G], lights[lightIdx + LIGHT_DATA_OFFSET_B]);
        lighting += (albedoColor.rgb * intensity * pow(attenuation, 1.6) * lightColor * spotFalloff * shadowFactor);
    }

    lighting += glowColor.rgb;

    // Fog
    float fogFac = clamp(distToPixel * INV_FOG_DIST, 0.0, 1.0);
    float lum = dot(lighting, vec3(0.299, 0.587, 0.114));
    vec3 fogColor = vec3(fogColorR, fogColorG, fogColorB);
    fogFac = clamp(fogFac * (1.0 - lum), 0.0, 1.0);
    lighting = mix(fogColor, lighting, 1.0 - fogFac);

//     if (debugView == 1) {
//         outAlbedo = albedoColor;
//         outAlbedo.a = 1.0;
//     } else if (debugView == 2) {
//         outAlbedo.r = (adjustedNormal.x + 1.0) * 0.5f;
//         outAlbedo.g = (adjustedNormal.y + 1.0) * 0.5f;
//         outAlbedo.b = (adjustedNormal.z + 1.0) * 0.5f;
//         outAlbedo.a = 1.0;
//     } else if (debugView == 3) {
//         float ndcDepth = (2.0 * gl_FragCoord.z - 1.0); // Depth debug
//         float clipDepth = ndcDepth / gl_FragCoord.w;
//         float linearDepth = (clipDepth - 0.02) / (71.68 - 0.02);
//         outAlbedo = vec4(vec3(linearDepth), 1.0);
//     } else if (debugView == 4) {
//         outAlbedo.r = float(InstanceIndex) / 5500.0;
//         outAlbedo.g = 0.0;
//         outAlbedo.b = float(texIndexChecked) / 1231.0;
//         outAlbedo.a = 1.0;
//     } else if (debugView == 5) { // Worldpos debug
//         outAlbedo.rgb = vec3(1.0);//worldPosPack.xyz;
//         outAlbedo.a = 1.0;
//     } else {
        outAlbedo = vec4(lighting.rgb, albedoColor.a);
//     }
}
