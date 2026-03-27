// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
#version 430 core
#extension GL_ARB_shading_language_packing : require
#extension GL_ARB_shader_image_load_store : enable

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

layout(location =  0) uniform uint instanceIndex; // start vert shader uniforms
layout(location =  1) uniform uint normInstanceIndex;
layout(location =  2) uniform mat4 viewProjection;
layout(location =  3) uniform uint isUI; // end vert shader uniforms
layout(location =  4) uniform vec3 fogColor;
layout(location =  6) uniform uint screenWidth;
layout(location =  7) uniform uint screenHeight;
layout(location =  8) uniform float worldMin_x;
layout(location =  9) uniform float worldMin_z;
layout(location = 10) uniform vec3 camPos;
layout(location = 14) uniform uint reflectionsEnabled;
layout(location = 15) uniform uint shadowsEnabled;
layout(location = 17) uniform uint unlit;
layout(location = 18) uniform uint texIndex;
layout(location = 19) uniform uint glowIndex;
layout(location = 20) uniform uint specIndex;
layout(location = 21) uniform uint shadowMapSize;
layout(location = 22) uniform float shadowMapSizeF;
layout(location = 23) uniform uint lightCount;
layout(location = 24) uniform uint maxLightsPerVoxel;
layout(location = 25) uniform uint constIndex;
layout(location = 26) uniform uint grayscaleEnabled;
layout(location = 27) uniform float volume;

struct Light {
    vec3  pos;
    float intensity;
    vec3  col;
    uint  lflags;
    float range;
    float spotAng;
    float maxIntensity;
    float minIntensity;
    vec4  spotDir; // Quaternions are vec4
};

layout(location = 0) out vec4 outAlbedo;   // GL_COLOR_ATTACHMENT0
layout(location = 1) out vec4 outWorldPos; // GL_COLOR_ATTACHMENT1
layout(location = 2) out vec4 outSpecular; // GL_COLOR_ATTACHMENT2
layout(location = 3) out vec2 outNormal;   // GL_COLOR_ATTACHMENT3
layout(std430, binding = 5) buffer ShadowMaps { uint shadowMaps[]; };
layout(std430, binding = 8) buffer ShadowMapsIndirection { uint shadowMapsIndirection[]; };
layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430, binding = 13) buffer BlueNoise { float blueNoiseColors[]; };
layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; }; // x,y pairs for width and height of textures
layout(std430, binding = 16) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors
layout(std430, binding = 17) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; }; // Palette starting indices for each texture
layout(std430, binding = 19) buffer LightIndices { Light lights[]; };
layout(std430, binding = 6) buffer VoxelLightListCounts { uint voxelLightListCounts[]; };
layout(std430, binding = 27) buffer UniqueLightLists { uint uniqueLightLists[]; };

const float WORLDCELL_WIDTH_F = 2.56;
const float VOXEL_SIZE = 0.32;
const vec3 baseDir = vec3(0.0, 0.0, 1.0);

uint GetVoxelIndex(vec3 worldPos) {
    float offsetX = worldPos.x - worldMin_x;
    float offsetZ = worldPos.z - worldMin_z;
    uint voxelX = uint(offsetX / VOXEL_SIZE);
    uint voxelZ = uint(offsetZ / VOXEL_SIZE);
    return (voxelZ * 512) + voxelX;
}

const int PCF_SAMPLES = 12;
const float invSamples = 1.0 / float(PCF_SAMPLES);
const vec2 poissonDisk[PCF_SAMPLES] = vec2[](
    vec2( 0.0000,  0.0000), vec2( 0.0812,  0.0941),
    vec2(-0.0451,  0.1192), vec2(-0.1221,  0.0321),
    vec2(-0.0914, -0.0882), vec2( 0.0021, -0.1275),
    vec2( 0.1023, -0.0621), vec2( 0.0452,  0.0325),
    vec2(-0.0512,  0.0421), vec2(-0.0215, -0.0612),
    vec2( 0.0581, -0.0214), vec2( 0.1215,  0.0112)
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

const vec4 BYTE_TO_FLOAT = vec4(1.0/255.0);

vec4 getTextureColor(uint texIndex, ivec2 texCoord) {
    uint pixelOffset = textureOffsets[texIndex] + uint(texCoord.y) * textureSizes[texIndex].x + uint(texCoord.x);
    uint slotIndex = pixelOffset >> 2u;// / 4u;
    uint packedIdx = colors[slotIndex];
    uint localOffset = pixelOffset & 3u;//% 4u;
    uint paletteIndex = (packedIdx >> (localOffset << 3u)) & 0xFFu; // << 3u is same as * 8
    uint paletteOffset = texturePaletteOffsets[texIndex];
    uint color = texturePalettes[paletteOffset + paletteIndex];
    return vec4(color & 0xFFu, (color>>8)&0xFFu, (color>>16)&0xFFu, color>>24) * BYTE_TO_FLOAT;
}

vec2 EncodeOctahedral(vec3 n) {
    n = normalize(n);
    vec2 p = n.xy / (abs(n.x) + abs(n.y) + abs(n.z));
    return n.z >= 0.0 ? p : (1.0 - abs(p.yx)) * vec2(n.x >= 0.0 ? 1.0 : -1.0, n.y >= 0.0 ? 1.0 : -1.0);
}

void main() {
    vec3 worldPos = FragPos.xyz;
    vec3 viewDir = (camPos - worldPos);
    float distToPixel = length(viewDir);
    viewDir = normalize(viewDir);
    ivec2 texSize = textureSizes[texIndex];
    vec2 uv = (vec2(TexCoord.x, 1.0 - TexCoord.y)); // Invert V (aka Y), OpenGL convention vs import
    ivec2 texUV = ivec2(int(floor(uv.x * float(texSize.x))), int(floor(uv.y * float(texSize.y))));
    texUV.x = texUV.x % texSize.x;
    texUV.y = texUV.y % texSize.y;
    vec4 albedoColor = getTextureColor(texIndex,texUV);
    if (albedoColor.a < 0.05 && volume < 0.05) discard; // Alpha cutout threshold

    float heat = 0.0;
    if (grayscaleEnabled > 0) {
        if (constIndex >= 419 && constIndex <= 448) { // NPC's are hot!
            heat = 4.0;
            if (constIndex == 419 || constIndex == 422 || constIndex == 424 || constIndex == 429 || constIndex == 430
            || constIndex == 431 || constIndex == 433 || constIndex == 437 || constIndex == 438 || constIndex == 441)
                heat = 1.5;
        }
    }
    vec3 adjustedNormal = Normal;
    if (normInstanceIndex != 0) { //  && distToPixel < 10.24 only has 0.073ms savings, leaving off for better quality of visuals
        vec3 dp1 = dFdx(FragPos);
        vec3 dp2 = dFdy(FragPos);
        vec2 duv1 = dFdx(TexCoord);
        vec2 duv2 = dFdy(TexCoord);
        float uvArea = abs(duv1.x * duv2.y - duv1.y * duv2.x);
        if (uvArea > 0.000000001) {
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

    vec4 glowColor = vec4(0.0);
    if (glowIndex != 0) {
        ivec2 texSizeGlow = textureSizes[glowIndex];
        ivec2 texUVGlow = ivec2(int(floor(uv.x * float(texSizeGlow.x))), int(floor(uv.y * float(texSizeGlow.y))));
        texUVGlow.x = texUVGlow.x % texSizeGlow.x;
        texUVGlow.y = texUVGlow.y % texSizeGlow.y;
        glowColor = getTextureColor(glowIndex,texUVGlow);
    }

    vec4 specColor = vec4(0.0);
    ivec2 texSizeSpec = textureSizes[specIndex];
    ivec2 texUVSpec = ivec2(int(floor(uv.x * float(texSizeSpec.x))),int(floor(uv.y * float(texSizeSpec.y))));
    texUVSpec.x = texUVSpec.x % texSizeSpec.x;
    texUVSpec.y = texUVSpec.y % texSizeSpec.y;
    specColor = getTextureColor(specIndex,texUVSpec);
    if (reflectionsEnabled > 0 && isUI == 0) {
        outSpecular = specColor;
        outWorldPos.xyz = FragPos.xyz;
        outNormal = EncodeOctahedral(adjustedNormal) * 0.5 + 0.5;  // Map to [0,1]
    }

    uint voxelIdx = GetVoxelIndex(worldPos);
    uint count = voxelLightListCounts[voxelIdx];
    if (unlit > 0) count = 0;
    vec3 lighting = vec3(0.0);
    uint listoffset = 0;
    float intensityTotal = 0.0;
    for (uint i = 0u; i < count; i++) {
        uint lightIdx = uniqueLightLists[(voxelIdx * maxLightsPerVoxel) + i];
        if (lightIdx >= lightCount) continue;

        vec3 lightPos = lights[lightIdx].pos;
        float intensity = lights[lightIdx].intensity;
        if (intensity < 0.1) continue;

        float range = lights[lightIdx].range;
        vec3 toLight = lightPos - worldPos;
        float dist = length(toLight);
        if (dist > range) continue;

        vec3 lightDir = normalize(toLight);
        float NdotL = dot(adjustedNormal, lightDir);
        float lambertian = clamp(max(NdotL, 0.0),0.0,1.0);
        float distOverRange = dist / range;
        float distOverRangeSqd = distOverRange * distOverRange;
        float attenuation = (1.0 - distOverRangeSqd) * lambertian;
        if (attenuation < 0.015) continue;

        float spotAng = lights[lightIdx].spotAng;
        float spotFalloff = 1.0;
        if (spotAng > 0.0) { // Extremely rare, only ~15 spot lights in entire game out of several thousand lights.
            vec4 quat = lights[lightIdx].spotDir;
            vec3 spotDir = normalize(quat_rotate(quat, baseDir));
            float spotdot = dot(spotDir, -lightDir);
            float cosAngle = cos(radians(spotAng / 2.0));
            if (spotdot < cosAngle) continue;
            
            float cosOuterAngle = cos(radians(spotAng / 2.0));
            float cosInnerAngle = cos(radians(spotAng * 0.8 / 2.0));
            spotFalloff = smoothstep(cosOuterAngle, cosInnerAngle, spotdot);
            if (spotFalloff <= 0.0) continue;
        }

        float shadowFactor = 1.0;
        uint shadowIndex = shadowMapsIndirection[lightIdx];
        if (shadowsEnabled > 0 && shadowIndex < lightCount) {
            float smearness = distOverRangeSqd * 24.0 + range + intensity + 4.51;
            vec3 a = abs(toLight);
            float mx = step(a.y, a.x) * step(a.z, a.x);
            float my = step(a.x, a.y) * step(a.z, a.y);
            vec3 mxyz = vec3(mx, my, 1.0 - mx - my);
            vec3 sxyz = vec3(step(0.0, -toLight.x), step(0.0, -toLight.y), step(0.0, -toLight.z));
            vec3 fxyz = vec3(mix(1.0, 0.0, sxyz.x), mix(3.0, 2.0, sxyz.y), mix(5.0, 4.0, sxyz.z));
            uint face = uint(mxyz.x * fxyz.x + mxyz.y * fxyz.y + mxyz.z * fxyz.z);
            float invMax = 1.0 / max(max(a.x, a.y), a.z);
            vec3 dir = -toLight * invMax;
            vec2 uvx = mix(vec2( dir.z, dir.y), vec2(-dir.z, dir.y), sxyz.x);
            vec2 uvy = mix(vec2( dir.x, dir.z), vec2( dir.x,-dir.z), sxyz.y);
            vec2 uvz = mix(vec2(-dir.x, dir.y), vec2( dir.x, dir.y), sxyz.z);
            vec2 uv = mxyz.x * uvx + mxyz.y * uvy + mxyz.z * uvz;
            uv = uv * 0.5 + 0.5;

            uint faceOff = (shadowIndex * shadowMapSize * shadowMapSize * 6) + (face * shadowMapSize * shadowMapSize);
            vec2 tc = uv * shadowMapSizeF;
            float slopeBias = 0.24 * (1.0 - NdotL);
            slopeBias = max(slopeBias,0.035);
            float bias = slopeBias * distOverRange;
            bias = max(bias,0.0);
            bias += 0.03; // Account for glancing angle acne

            // Pseudo-Stochastic PCF sampling
            float sum = 0.0;
            float shadSizeMaxUV = shadowMapSizeF - 1.0;
            for (int si = 0; si < PCF_SAMPLES; ++si) {
                vec2 off = poissonDisk[si] * smearness;
                vec2 t = tc + off;
                vec2 st = fract(t - 0.5);
                vec2 base = floor(t - 0.5);
                float samples[4];
                for(int i=0; i<2; ++i) {
                    for(int j=0; j<2; ++j) {
                        vec2 coord = clamp(base + vec2(i, j), 0.0, shadSizeMaxUV);
                        uint ssbo_idx = faceOff + uint(coord.y) * shadowMapSize + uint(coord.x);
                        float d = float(shadowMaps[ssbo_idx]) * 0.00001;
                        samples[i + j*2] = dist <= (d + bias) ? 1.0 : 0.0;
                    }
                }

                float res = mix(mix(samples[0], samples[1], st.x), mix(samples[2], samples[3], st.x), st.y); // Manual bilinear shadowmap filter
                sum += res;
            }

            shadowFactor = sum * invSamples;
        }

        vec3 lightColor = lights[lightIdx].col;
        vec3 baseLighting = albedoColor.rgb  * lightColor * intensity * pow(attenuation, 1.75);
        lighting = fma(baseLighting,vec3(spotFalloff * shadowFactor),lighting);
        intensityTotal = fma(intensity,attenuation * 1.5,intensityTotal);
        vec3 halfDir = normalize(lightDir + viewDir);
        float ndh = max(dot(adjustedNormal, halfDir), 0.0);
        float strength = texIndex == 36 || texIndex == 887 ? 1.0 : max(specColor.r, max(specColor.g, specColor.b)) * 4.51;
        float spec = clamp(pow(ndh, 100.0),0.0,1.0);
        lighting += specColor.rgb * intensity * attenuation * spotFalloff * spec * shadowFactor * strength;
    }

    float rim = 1.0 - max(dot(adjustedNormal, viewDir), 0.0);
    lighting += clamp(pow(rim, 4.0) * 0.25 * clamp(intensityTotal,0.0,1.0) * specColor.rgb,0.0,1.0); // Specular "rim" fresnel (tested and performance impact is essentially zero)
    lighting = (unlit > 0) ? albedoColor.rgb : lighting + glowColor.rgb;
    if (heat > 0.0) {
        lighting += albedoColor.rgb * heat;
        lighting = pow(lighting, vec3(1.2));
        lighting += clamp(pow(rim, 3.0) * 0.5 * specColor.rgb, 0.0, 1.0);
    }

    // Blue Noise Dither for banding (0.03ms performance cost, leaving in for quality)
    ivec2 sp = ivec2(gl_FragCoord.xy);
    int idx = (sp.x & 63) + (sp.y & 63) * 64;
    float blue = blueNoiseColors[idx*3 + 0];
    float dither = (blue - 0.5) * 0.003921569; // 1.0 / 255.0;
    lighting.rgb += vec3(dither);

    // Fog
    if (unlit == 0) {
        float fogFac = clamp(distToPixel * 0.013950893, 0.0, 1.0); // This is inverse of fog dist so * (1 / 71.68 far plane)
        float lum = dot(lighting, vec3(0.299, 0.587, 0.114));
        fogFac = clamp(fogFac * (1.0 - lum), 0.0, 1.0);
        lighting = mix(fogColor, lighting, 1.0 - fogFac);
    }

    outAlbedo = vec4(lighting.rgb, albedoColor.a);
}
