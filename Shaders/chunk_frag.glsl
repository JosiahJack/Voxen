// chunk_frag.glsl: Generic shader for all world objects
// #extension GL_ARB_shading_language_packing : require
// #extension GL_ARB_shader_image_load_store : enable
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
layout(location=0) uniform uint instanceIndex; // start vert shader uniforms
layout(location=1) uniform uint normInstanceIndex;
layout(location=2) uniform mat4 viewProjection; /* Used by vert shader
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
layout(location=13) uniform uint useStrengthMod; // For spec boost on some materials
layout(location=14) uniform uint reflectionsEnabled;
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
layout(location=1) out vec4 outSpecular; // GL_COLOR_ATTACHMENT1
layout(location=2) out vec2 outNormal;   // GL_COLOR_ATTACHMENT2
layout(std430,binding=2) buffer VoxelLightListCounts { uint voxelLightListCounts[]; };
layout(std430,binding=3) buffer UniqueLightLists { uint uniqueLightLists[]; };
layout(std430,binding=4) buffer LightIndices { Light lights[]; };
layout(std430,binding=5) buffer ShadowMaps { uint shadowMaps[]; };
layout(std430,binding=6) buffer ShadowMapsIndirection { uint shadowMapsIndirection[]; };
layout(std430,binding=12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430,binding=14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430,binding=15) buffer TextureSizes { ivec2 textureSizes[]; }; // x,y pairs for width and height of textures
layout(std430,binding=8) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors
layout(std430,binding=9) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; }; // Palette starting indices for each texture
float getBlueNoise(ivec2 p) {uint n = (uint(p.x) * 73856093u) ^ (uint(p.y) * 19349663u); n=(n^(n >> 13u))*1274126177u; return float(n & 0xFFu) * 0.00392156862; }// 0.00392156862 == 1.0 / 255.0
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
const vec2 poissonDisk[PCF_SAMPLES] = vec2[](vec2(0.0,0.0), vec2(0.0812,0.0941),vec2(-0.0451,0.1192),vec2(-0.1221,0.0321),vec2(-0.0914,-0.0882),vec2(0.0021,-0.1275),vec2(0.1023,-0.0621), vec2(0.0452,0.0325));
vec4 getTextureColor(uint texIdx, ivec2 texCoord, int texSizeX) {
    uint pixelOffset = textureOffsets[texIdx] + uint(texCoord.y) * texSizeX + uint(texCoord.x);
    uint slotIndex = pixelOffset >> 2u;
    uint packedIdx = colors[slotIndex];
    uint localOffset = pixelOffset & 3u;
    uint paletteIndex = (packedIdx >> (localOffset << 3u)) & 0xFFu;
    uint paletteOffset = texturePaletteOffsets[texIdx];
    uint color = texturePalettes[paletteOffset + paletteIndex];
    return unpackUnorm4x8(color);
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
    float oneInv_mxy = 1.0 - mx - my;
    vec3 mxyz = vec3(mx,my,oneInv_mxy);
    vec3 sxyz = vec3(step(0.0,-toLight.x),step(0.0,-toLight.y),step(0.0,-toLight.z));
    vec3 fxyz = vec3(mix(1.0,0.0,sxyz.x), mix(3.0,2.0,sxyz.y), mix(5.0,4.0,sxyz.z));
    uint face = uint(mxyz.x * fxyz.x + mxyz.y * fxyz.y + mxyz.z * fxyz.z);
    float invMax = 1.0 / max(max(a.x,a.y),a.z);
    vec3 dir = -toLight * invMax;
    vec2 uvx = mix(vec2( dir.z, dir.y), vec2(-dir.z, dir.y), sxyz.x);
    vec2 uvy = mix(vec2( dir.x, dir.z), vec2( dir.x,-dir.z), sxyz.y);
    vec2 uvz = mix(vec2(-dir.x, dir.y), vec2( dir.x, dir.y), sxyz.z);
    vec2 uv = mx * uvx + my * uvy + oneInv_mxy * uvz;
    uv = uv * 0.5 + 0.5;
    faceOff = (shadowIndex * shadSizeSqd * 6u) + (face * shadSizeSqd);
    tc = uv * shadowMapSizeF;
}

float quintic_polynomial_smoothstep( float x ) { return x*x*x*(x*(x*6.0-15.0)+10.0); } // From https://iquilezles.org/articles/smoothsteps/
void main() {
    vec3 worldPos = FragPos.xyz;
    vec3 viewDir = (camPos - worldPos);
    float distToPixelSq = dot(viewDir, viewDir);
    float distToPixel = sqrt(distToPixelSq);
    viewDir *= inversesqrt(distToPixelSq);
    ivec2 texSize = useCamView > 0 ? ivec2(camViewSize) : textureSizes[texIndex];
    vec2 uv = (vec2(TexCoord.x, 1.0 - TexCoord.y)); // Invert V (aka Y), OpenGL convention vs import 
    vec4 albedoColor;
    if (useCamView > 0) { albedoColor = texture(camViewTex, vec2(TexCoord.x, TexCoord.y)); } 
    else albedoColor = getTextureColor(texIndex, ivec2(fract(uv) * vec2(texSize)), texSize.x);
    if (albedoColor.a < 0.05 && volume < 0.05) discard; 
    vec3 adjustedNormal = Normal;
    bool hasNormalMap = normInstanceIndex != 0;
    float blend = 0.0;
    float facing = dot(Normal, viewDir);
    if (distToPixel < 5.12 && hasNormalMap) blend = 1.0;
    else if (distToPixel < 30.0 && hasNormalMap) { blend = smoothstep(30.0,5.12,distToPixel) * smoothstep(0.1,0.4,facing); }
    if (hasNormalMap && blend > 0.01) {
        vec3 dp1 = dFdx(FragPos); vec3 dp2 = dFdy(FragPos);
        vec2 duv1 = dFdx(TexCoord); vec2 duv2 = dFdy(TexCoord);
        float uvArea = abs(duv1.x * duv2.y - duv1.y * duv2.x);
        if (uvArea > 0.000000001) {
            vec3 t = dp1 * duv2.y - dp2 * duv1.y; t *= inversesqrt(dot(t, t));
            vec3 b = dp1 * duv2.x - dp2 * duv1.x; b *= inversesqrt(dot(b, b));
            mat3 TBN3x3 = mat3(t, b, adjustedNormal);
            ivec2 texSizeNorm = textureSizes[normInstanceIndex];
            vec3 normalColor = (getTextureColor(normInstanceIndex,ivec2(fract(uv) * vec2(texSizeNorm)),texSizeNorm.x).rgb * 2.0 - 1.0);
            normalColor.g = -normalColor.g;
            vec3 transformed = TBN3x3 * normalColor;
            transformed *= inversesqrt(dot(transformed, transformed));
            if (dot(transformed, Normal) < 0.0) transformed = Normal;
            adjustedNormal = mix(Normal, transformed, blend);
        }
    }
    vec4 glowColor = vec4(0.0);
    if (glowIndex != 0) { ivec2 texSizeGlow = textureSizes[glowIndex]; glowColor = getTextureColor(glowIndex,ivec2(fract(uv) * vec2(texSizeGlow)),texSizeGlow.x); }
    vec4 specColor = vec4(0.0);
    if (specIndex  != 0) { ivec2 texSizeSpec = textureSizes[specIndex]; specColor = getTextureColor(specIndex,ivec2(fract(uv) * vec2(texSizeSpec)),texSizeSpec.x); }
    if (reflectionsEnabled > 0) { outSpecular = specColor; outNormal = EncodeOctahedral(adjustedNormal) * 0.5 + 0.5; } // Map to [0,1]
    uint voxelIdx = GetVoxelIndex(worldPos);
    uint count = voxelLightListCounts[voxelIdx];
    if (unlit > 0 || useCamView > 0) count = 0;
    vec3 lighting = vec3(0.0);
    float intensityTotal = 0.0;
    float strength = (useStrengthMod > 0) ? 1.0 : max(specColor.r, max(specColor.g, specColor.b)) * 4.51;
    float shadSizeMaxUV = shadowMapSizeF - 1.0;
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
        float lambertian = clamp(NdotL,0.0,1.0);
        float distOverRange = dist * invRange;
        float attenuation = (1.0 - (distOverRange * distOverRange)) * lambertian;
        if (attenuation < 0.03) continue;
        float spotAng = lights[lightIdx].spotAng;
        float spotFalloff = 1.0;
        if (spotAng > 0.0) { // Extremely rare, only ~15 spot lights in entire game out of several thousand lights.
            vec4 quat = lights[lightIdx].spotDir;
            vec3 spotDir = vec3(2.0 * (quat.x * quat.z + quat.w * quat.y),2.0 * (quat.y * quat.z - quat.w * quat.x),1.0 - 2.0 * (quat.x * quat.x + quat.y * quat.y));
            float spotdot = dot(spotDir, -lightDir);
            float cosAngle = cos(radians(spotAng / 2.0)); if (spotdot < cosAngle) continue;
            spotFalloff = smoothstep(cosAngle,cos(radians(spotAng * 0.8 / 2.0))/*Inner angle*/,spotdot); if (spotFalloff <= 0.0) continue;
        }
        float shadowFactor = 1.0;
        uint shadowIndex = shadowMapsIndirection[lightIdx];
        bool lightHasShadows = (lights[lightIdx].lflags & SHADON) != 0u;
        if (shadowsEnabled > 0 && (shadowIndex < lightCount) && lightHasShadows) {
            uint faceOff; vec2 tc; GetCubemapSampleCoord(toLight,shadowIndex,faceOff,tc);
            vec2 basec = floor(tc - 0.5);
            vec2 coordc = clamp(basec,0.0,shadSizeMaxUV);
            uint ssbo_idx_center = faceOff + uint(coordc.y) * shadowMapSize + uint(coordc.x);
            float occDist = float(shadowMaps[ssbo_idx_center]) * 0.00001;
            float distToOccluderFac = clamp(occDist * 0.130208333, 0.0, 1.0);
            float smearness = distToOccluderFac * (range + intensity + 4.51) * 2.5;
            float bias = quintic_polynomial_smoothstep(distToOccluderFac) * 0.48 + quintic_polynomial_smoothstep(clamp(0.16 * attenuation, 0.0, 1.0));
            float thresh = dist - bias;
            uint threshInt = uint(max(0.0, thresh) * 100000.0 + 0.5);
            float sum = 0.0;
            for (int si=0;si<PCF_SAMPLES;++si) { // Pseudo-Stochastic PCF sampling
                vec2 t = tc + poissonDisk[si] * smearness;
                vec2 halfT = (t - 0.5);
                vec2 st = fract(halfT);
                vec2 base = floor(halfT);
                uint x0 = uint(clamp(base.x, 0.0, shadSizeMaxUV));
                uint y0 = uint(clamp(base.y, 0.0, shadSizeMaxUV));
                uint x1 = uint(clamp(base.x + 1.0, 0.0, shadSizeMaxUV));
                uint y1 = uint(clamp(base.y + 1.0, 0.0, shadSizeMaxUV));
                uint row0 = faceOff + y0 * shadowMapSize;
                uint row1 = faceOff + y1 * shadowMapSize;
                float s00 = (shadowMaps[row0 + x0] >= threshInt) ? 1.0 : 0.0;
                float s10 = (shadowMaps[row0 + x1] >= threshInt) ? 1.0 : 0.0;
                float s01 = (shadowMaps[row1 + x0] >= threshInt) ? 1.0 : 0.0;
                float s11 = (shadowMaps[row1 + x1] >= threshInt) ? 1.0 : 0.0;
                sum += mix(mix(s00,s10,st.x),mix(s01,s11,st.x),st.y); // Manual bilinear shadowmap filter
            }
            shadowFactor = (sum * invSamples);
        }
        vec3 lightColor = lights[lightIdx].col;
        vec3 baseLighting = albedoColor.rgb  * lightColor * intensity * (attenuation * attenuation / sqrt(sqrt(attenuation)));
        lighting = fma(baseLighting,vec3(spotFalloff * shadowFactor),lighting);
        intensityTotal = fma(intensity,attenuation * 1.5,intensityTotal);
        vec3 halfDir = normalize(lightDir + viewDir);
        float ndh = max(dot(adjustedNormal, halfDir), 0.0);
        float spec = clamp(pow(ndh,100.0),0.0,1.0);
        lighting += specColor.rgb * intensity * attenuation * spotFalloff * spec * shadowFactor * strength;
    }
    float rim = 1.0 - max(dot(adjustedNormal, viewDir), 0.0);
    lighting += clamp(rim * rim * rim * rim * 0.25 * clamp(intensityTotal,0.0,1.0) * specColor.rgb,0.0,1.0); // Specular "rim" fresnel (tested and performance impact is essentially zero)
    lighting = (unlit > 0 || useCamView > 0) ? albedoColor.rgb : lighting + glowColor.rgb;
    if (heat > 0.0) { lighting += albedoColor.rgb * heat; lighting = pow(lighting, vec3(1.2)); lighting += clamp(rim * rim * rim * rim * 0.5 * specColor.rgb, 0.0, 1.0); }
    float blue = getBlueNoise(ivec2(gl_FragCoord.xy)); // Blue Noise Dither for banding (0.03ms performance cost, leaving in for quality)
    lighting.rgb += vec3((blue - 0.5) * 0.003921569);
    if (unlit == 0) { // Fog
        float fogFac = clamp(distToPixel * 0.013950893, 0.0, 1.0); // This is inverse of fog dist so * (1 / 71.68 far plane)
        float lum = dot(lighting, vec3(0.299, 0.587, 0.114));
        fogFac = clamp(fogFac * (1.0 - lum), 0.0, 1.0);
        lighting = mix(fogColor, lighting, 1.0 - fogFac);
    }
    outAlbedo = vec4(lighting.rgb, albedoColor.a);
}
