// chunk_frag.glsl: Generic shader for all world objects
// #extension GL_ARB_shading_language_packing : require
// #extension GL_ARB_shader_image_load_store : enable
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
layout(location=0) uniform uint instanceIndex; // start vert shader uniforms
layout(location=1) uniform uint normInstanceIndex;
layout(location=2) uniform mat4 viewProjection; /*
layout(location=3) some weird
layout(location=4) padding apparently, so say some docs anyhow
layout(location=5) */
layout(location=6) uniform uint screenWidth;
layout(location=7) uniform uint screenHeight;
layout(location=8) uniform vec2 worldMin;
layout(location=9) uniform float heat;
layout(location=10) uniform vec3 camPos;
layout(location=11) uniform uint shadSizeSqd;
layout(location=12) uniform vec3 fogColor;
layout(location=14) uniform uint reflectionsEnabled;
layout(location=13) uniform uint useStrengthMod; // For spec boost on some materials
layout(location=15) uniform uint shadowsEnabled;
layout(location=17) uniform uint unlit;
layout(location=18) uniform uint texIndex;
layout(location=19) uniform uint glowIndex;
layout(location=20) uniform uint specIndex;
layout(location=21) uniform uint shadowMapSize;
layout(location=22) uniform float shadowMapSizeF;
layout(location=23) uniform uint lightCount;
layout(location=24) uniform uint maxLightsPerVoxel;
layout(location=25) uniform uint constIndex;
layout(location=26) uniform uint grayscaleEnabled;
layout(location=27) uniform float volume;
layout(location=28) uniform uvec2 camViewSize;
layout(location=29) uniform sampler2D camViewTex;
layout(location=30) uniform uint useCamView;
struct Light { vec3 pos; float intensity; vec3 col; uint lflags; float range; float spotAng; float maxIntensity; float minIntensity; vec4 spotDir; };
layout(location=0) out vec4 outAlbedo;   // GL_COLOR_ATTACHMENT0
layout(location=1) out vec4 outWorldPos; // GL_COLOR_ATTACHMENT1
layout(location=2) out vec4 outSpecular; // GL_COLOR_ATTACHMENT2
layout(location=3) out vec2 outNormal;   // GL_COLOR_ATTACHMENT3
layout(std430,binding=2) buffer VoxelLightListCounts { uint voxelLightListCounts[]; };
layout(std430,binding=3) buffer UniqueLightLists { uint uniqueLightLists[]; };
layout(std430,binding=4) buffer LightIndices { Light lights[]; };
layout(std430,binding=5) buffer ShadowMaps { uint shadowMaps[]; };
layout(std430,binding=6) buffer ShadowMapsIndirection { uint shadowMapsIndirection[]; };
layout(std430,binding=12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430,binding=14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430,binding=15) buffer TextureSizes { ivec2 textureSizes[]; }; // x,y pairs for width and height of textures
layout(std430,binding=16) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors
layout(std430,binding=17) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; }; // Palette starting indices for each texture

const uint blueNoise[64] = uint[](
    0x23F1A408u, 0x8C4BDE72u, 0x159D3F66u, 0xB1549A2Du, 0x47C20E89u, 0xD67B1F58u, 0x32A96C17u, 0xE4832B94u,
    0x915F36C8u, 0x1A7D42B9u, 0xE1842C9Au, 0x3F6E15B2u, 0x8D4C0F73u, 0x54A61EBDu, 0x27D39F0Cu, 0xC17A58E6u,
    0x4E921B6Fu, 0xB843D157u, 0x2A8C09F4u, 0x7E3596C2u, 0x1F5D8B41u, 0xAC6E24D9u, 0x38B712A3u, 0xF682490Du,
    0x147C5A3Eu, 0xD92E81F3u, 0x4B62079Cu, 0xA25F31D7u, 0x8B4E1676u, 0x2D935C1Bu, 0xF17A4802u, 0x39C46D15u,
    0x6D1A4F82u, 0xB42C9E35u, 0x1F8D5473u, 0xD60B7A42u, 0x39E1842Cu, 0x9A58C71Fu, 0x4B7D12B1u, 0xE62409F3u,
    0x2D8C4F1Au, 0x73B9541Eu, 0xC26E0B8Du, 0x159A3F47u, 0x842C6D1Bu, 0x3F7A4E92u, 0xB158D60Cu, 0x1A842D93u,
    0x547B1E8Cu, 0x9D3F6215u, 0x42B91A7Du, 0x0E842C9Au, 0xF31D7B4Eu, 0x24D60B8Cu, 0x81F34B62u, 0x39C17A48u,
    0xA25F147Cu, 0x8B4E2D93u, 0x1F7A4802u, 0xD62E81F3u, 0x4B62147Cu, 0xA25F31D7u, 0x8B4E1676u, 0x2D935C1Bu
);

float getBlueNoise(ivec2 p) {
    int idx = ((p.y & 15) << 4) | (p.x & 15);
    uint val = blueNoise[idx >> 2];
    uint byte = (val >> ((idx & 3) << 3)) & 0xFFu;
    return float(byte) * 0.00392156862; // Pre-multiplied 1.0/255.0
}

const float VOXEL_SIZE = 0.32;
const uint SHADON = 2u;
uint GetVoxelIndex(vec3 worldPos) {
    float offsetX = worldPos.x - worldMin.x;
    float offsetZ = worldPos.z - worldMin.y;
    uint voxelX = uint(offsetX / VOXEL_SIZE);
    uint voxelZ = uint(offsetZ / VOXEL_SIZE);
    return (voxelZ * 512) + voxelX;
}

const int PCF_SAMPLES = 8;
const float invSamples = 1.0 / float(PCF_SAMPLES);
const vec2 poissonDisk[PCF_SAMPLES] = vec2[](
    vec2( 0.0000, 0.0000), vec2( 0.0812, 0.0941),
    vec2(-0.0451, 0.1192), vec2(-0.1221, 0.0321),
    vec2(-0.0914,-0.0882), vec2( 0.0021,-0.1275),
    vec2( 0.1023,-0.0621), vec2( 0.0452, 0.0325));

vec3 quat_rotate(vec4 q, vec3 v) {
    float x2 = q.x + q.x; float y2 = q.y + q.y; float z2 = q.z + q.z;
    float xx2 = q.x * x2; float yy2 = q.y * y2; float zz2 = q.z * z2;
    float xy2 = q.x * y2; float xz2 = q.x * z2; float yz2 = q.y * z2;
    float wx2 = q.w * x2; float wy2 = q.w * y2; float wz2 = q.w * z2;
    return vec3(
        v.x * (1.0 - yy2 - zz2) + v.y * (xy2 - wz2) + v.z * (xz2 + wy2),
        v.x * (xy2 + wz2) + v.y * (1.0 - xx2 - zz2) + v.z * (yz2 - wx2),
        v.x * (xz2 - wy2) + v.y * (yz2 + wx2) + v.z * (1.0 - xx2 - yy2)
    );
}

const vec4 BYTE_TO_FLOAT = vec4(1.0/255.0);
vec4 getTextureColor(uint texIndex, ivec2 texCoord, int texSizeX) {
    uint pixelOffset = textureOffsets[texIndex] + uint(texCoord.y) * texSizeX + uint(texCoord.x);
    uint slotIndex = pixelOffset >> 2u;// / 4u;
    uint packedIdx = colors[slotIndex];
    uint localOffset = pixelOffset & 3u;//% 4u;
    uint paletteIndex = (packedIdx >> (localOffset << 3u)) & 0xFFu; // << 3u is same as * 8
    uint paletteOffset = texturePaletteOffsets[texIndex];
    uint color = texturePalettes[paletteOffset + paletteIndex];
    return vec4(color & 0xFFu,(color>>8)&0xFFu,(color>>16)&0xFFu,color>>24) * BYTE_TO_FLOAT;
}

vec2 EncodeOctahedral(vec3 n) {
    n = normalize(n);
    vec2 p = n.xy / (abs(n.x) + abs(n.y) + abs(n.z));
    return n.z >= 0.0 ? p : (1.0 - abs(p.yx)) * vec2(n.x >= 0.0 ? 1.0 : -1.0, n.y >= 0.0 ? 1.0 : -1.0);
}

void GetCubemapSampleCoord(vec3 toLight, uint shadowIndex, out uint faceOff, out vec2 tc) {
    vec3 a = abs(toLight);
    float mx = step(a.y,a.x) * step(a.z,a.x);
    float my = step(a.x,a.y) * step(a.z,a.y);
    vec3 mxyz = vec3(mx,my,1.0 - mx - my);
    vec3 sxyz = vec3(step(0.0,-toLight.x),step(0.0,-toLight.y),step(0.0,-toLight.z));
    vec3 fxyz = vec3(mix(1.0,0.0,sxyz.x), mix(3.0,2.0,sxyz.y), mix(5.0,4.0,sxyz.z));
    uint face = uint(mxyz.x * fxyz.x + mxyz.y * fxyz.y + mxyz.z * fxyz.z);
    float invMax = 1.0 / max(max(a.x,a.y),a.z);
    vec3 dir = -toLight * invMax;
    vec2 uvx = mix(vec2( dir.z, dir.y), vec2(-dir.z, dir.y), sxyz.x);
    vec2 uvy = mix(vec2( dir.x, dir.z), vec2( dir.x,-dir.z), sxyz.y);
    vec2 uvz = mix(vec2(-dir.x, dir.y), vec2( dir.x, dir.y), sxyz.z);
    float fmx = step(a.y,a.x)*step(a.z,a.x);
    float fmy = step(a.x,a.y)*step(a.z,a.y);
    vec2 uv = fmx * uvx + fmy * uvy + (1.0 - fmx - fmy) * uvz;
    uv = uv * 0.5 + 0.5;
    faceOff = (shadowIndex * shadSizeSqd * 6u) + (face * shadSizeSqd);
    tc = uv * shadowMapSizeF;
}

float quintic_polynomial_smoothstep( float x ) { return x*x*x*(x*(x*6.0-15.0)+10.0); } // From https://iquilezles.org/articles/smoothsteps/
const vec3 baseDir = vec3(0.0,0.0,1.0);
void main() {
    vec3 worldPos = FragPos.xyz;
    vec3 viewDir = (camPos - worldPos);
    float distToPixel = length(viewDir);
    viewDir = normalize(viewDir);
    ivec2 texSize = useCamView > 0 ? ivec2(camViewSize) : textureSizes[texIndex];
    vec2 uv = (vec2(TexCoord.x, 1.0 - TexCoord.y)); // Invert V (aka Y), OpenGL convention vs import
    ivec2 texUV = ivec2(int(floor(uv.x * float(texSize.x))), int(floor(uv.y * float(texSize.y))));
    texUV.x = texUV.x % texSize.x;
    texUV.y = texUV.y % texSize.y;
    vec4 albedoColor;
    if (useCamView > 0) {
        vec2 uv2 = (vec2(TexCoord.x,TexCoord.y));
        albedoColor = texture(camViewTex,uv2);
    } else albedoColor = getTextureColor(texIndex,texUV,texSize.x);
    if (albedoColor.a < 0.05 && volume < 0.05) discard; // Alpha cutout threshold

    vec3 adjustedNormal = Normal;
    bool hasNormalMap = normInstanceIndex != 0;
    float blend = 0.0;
    float facing = dot(Normal,viewDir);
    if (distToPixel < 5.12 && hasNormalMap) blend = 1.0;
    else if (distToPixel < 30.0 && hasNormalMap) {
        blend = smoothstep(30.0,5.12,distToPixel);
        blend *= smoothstep(0.1,0.4,facing);
    }

    if (hasNormalMap && blend > 0.01) {
        vec3 dp1 = dFdx(FragPos); vec3 dp2 = dFdy(FragPos);
        vec2 duv1 = dFdx(TexCoord); vec2 duv2 = dFdy(TexCoord);
        float uvArea = abs(duv1.x * duv2.y - duv1.y * duv2.x);
        if (uvArea > 0.000000001) {
            vec3 t = normalize(dp1 * duv2.y - dp2 * duv1.y);
            vec3 b = normalize(dp1 * duv2.x - dp2 * duv1.x);
            mat3 TBN3x3 = mat3(t, b, adjustedNormal);
            ivec2 texSizeNorm = textureSizes[normInstanceIndex];
            ivec2 texUVNorm = ivec2(int(floor(uv.x * float(texSizeNorm.x))), int(floor(uv.y * float(texSizeNorm.y))));
            texUVNorm.x = texUVNorm.x % texSizeNorm.x; texUVNorm.y = texUVNorm.y % texSizeNorm.y;
            vec3 normalColor = (getTextureColor(normInstanceIndex,texUVNorm,texSizeNorm.x).rgb * 2.0 - 1.0);
            normalColor.g = -normalColor.g;
            adjustedNormal = normalize(TBN3x3 * normalColor);
            if (dot(adjustedNormal,Normal) < 0.0) adjustedNormal = Normal;
            adjustedNormal = mix(Normal,adjustedNormal,blend);
        }
    }

    vec4 glowColor = vec4(0.0);
    if (glowIndex != 0) {
        ivec2 texSizeGlow = textureSizes[glowIndex];
        ivec2 texUVGlow = ivec2(int(floor(uv.x * float(texSizeGlow.x))), int(floor(uv.y * float(texSizeGlow.y))));
        texUVGlow.x = texUVGlow.x % texSizeGlow.x;
        texUVGlow.y = texUVGlow.y % texSizeGlow.y;
        glowColor = getTextureColor(glowIndex,texUVGlow,texSizeGlow.x);
    }

    vec4 specColor = vec4(0.0);
    if (specIndex  != 0) {
        ivec2 texSizeSpec = textureSizes[specIndex];
        ivec2 texUVSpec = ivec2(int(floor(uv.x * float(texSizeSpec.x))),int(floor(uv.y * float(texSizeSpec.y))));
        texUVSpec.x = texUVSpec.x % texSizeSpec.x;
        texUVSpec.y = texUVSpec.y % texSizeSpec.y;
        specColor = getTextureColor(specIndex,texUVSpec,texSizeSpec.x);
    }

    if (reflectionsEnabled > 0) {
        outSpecular = specColor;
        outWorldPos.xyz = FragPos.xyz;
        outNormal = EncodeOctahedral(adjustedNormal) * 0.5 + 0.5;  // Map to [0,1]
    }

    uint voxelIdx = GetVoxelIndex(worldPos);
    uint count = voxelLightListCounts[voxelIdx];
    if (unlit > 0 || useCamView > 0) count = 0;
    vec3 lighting = vec3(0.0);
    uint listoffset = 0;
    float intensityTotal = 0.0;
    for (uint i = 0u; i < count; i++) {
        uint lightIdx = uniqueLightLists[(voxelIdx * maxLightsPerVoxel) + i];
        vec3 lightPos = lights[lightIdx].pos;
        float intensity = lights[lightIdx].intensity;
        float range = lights[lightIdx].range;
        float invRange = 1.0 / range;
        vec3 toLight = lightPos - worldPos;
        float dist = length(toLight);
        vec3 lightDir = toLight * (1.0 / dist);
        float NdotL = dot(adjustedNormal, lightDir);
        float lambertian = clamp(max(NdotL, 0.0),0.0,1.0);
        float distOverRange = dist * invRange;
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
        bool lightHasShadows = (lights[lightIdx].lflags & SHADON) != 0u;
        if (shadowsEnabled > 0 && (shadowIndex < lightCount) && lightHasShadows) {
            uint faceOff; vec2 tc; GetCubemapSampleCoord(toLight,shadowIndex,faceOff,tc);
            float shadSizeMaxUV = shadowMapSizeF - 1.0;
            vec2 halfTc = (tc - 0.5);
            vec2 stc = fract(halfTc);
            vec2 basec = floor(halfTc);
            vec2 coordc = clamp(basec,0.0,shadSizeMaxUV);
            uint ssbo_idx_center = faceOff + uint(coordc.y) * shadowMapSize + uint(coordc.x);
            float dc = float(shadowMaps[ssbo_idx_center]) * 0.00001;
            float distToOccluderFac = dc * 0.130208333;//(1.0 / 7.68);
            float smearness = clamp(distToOccluderFac,0.0,1.0) * (range + intensity + 4.51) * 2.5;
            float bias = quintic_polynomial_smoothstep(distToOccluderFac) * 0.48 + quintic_polynomial_smoothstep(0.16 * attenuation);
            float sum = 0.0;
            for (int si=0;si<PCF_SAMPLES;++si) { // Pseudo-Stochastic PCF sampling
                vec2 off = poissonDisk[si] * smearness;
                vec2 t = tc + off;
                vec2 halfT = (t - 0.5);
                vec2 st = fract(halfT);
                vec2 base = floor(halfT);
                float samples[4];
                for(int i=0; i<2; ++i) {
                    for(int j=0; j<2; ++j) {
                        vec2 coord = clamp(base + vec2(i, j),0.0,shadSizeMaxUV);
                        uint ssbo_idx = faceOff + uint(coord.y) * shadowMapSize + uint(coord.x);
                        float d = float(shadowMaps[ssbo_idx]) * 0.00001;
                        samples[i + j*2] = dist <= (d + bias) ? 1.0 : 0.0;
                    }
                }

                sum += mix(mix(samples[0],samples[1],st.x),mix(samples[2],samples[3],st.x),st.y); // Manual bilinear shadowmap filter
            }

            shadowFactor = (sum * invSamples);
        }

        vec3 lightColor = lights[lightIdx].col;
        vec3 baseLighting = albedoColor.rgb  * lightColor * intensity * pow(attenuation, 1.75);
        lighting = fma(baseLighting,vec3(spotFalloff * shadowFactor),lighting);
        intensityTotal = fma(intensity,attenuation * 1.5,intensityTotal);
        vec3 halfDir = normalize(lightDir + viewDir);
        float ndh = max(dot(adjustedNormal, halfDir), 0.0);
        float strength = (useStrengthMod > 0) ? 1.0 : max(specColor.r, max(specColor.g, specColor.b)) * 4.51;
        float spec = clamp(pow(ndh,100.0),0.0,1.0);
        lighting += specColor.rgb * intensity * attenuation * spotFalloff * spec * shadowFactor * strength;
    }

    float rim = 1.0 - max(dot(adjustedNormal, viewDir), 0.0);
    lighting += clamp(pow(rim, 4.0) * 0.25 * clamp(intensityTotal,0.0,1.0) * specColor.rgb,0.0,1.0); // Specular "rim" fresnel (tested and performance impact is essentially zero)
    lighting = (unlit > 0 || useCamView > 0) ? albedoColor.rgb : lighting + glowColor.rgb;
    if (heat > 0.0) {
        lighting += albedoColor.rgb * heat;
        lighting = pow(lighting, vec3(1.2));
        lighting += clamp(pow(rim, 3.0) * 0.5 * specColor.rgb, 0.0, 1.0);
    }

    // Blue Noise Dither for banding (0.03ms performance cost, leaving in for quality)
    float blue = getBlueNoise(ivec2(gl_FragCoord.xy));
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
