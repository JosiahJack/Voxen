// Shadowmap Fragment Shader
#version 430 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
flat in uint TexIndex;

layout(std430,  binding = 5) buffer ShadowMaps { uint depthData[]; };
layout(std430,  binding = 6) buffer ReflectionMaps { uint reflectionColors[]; };
layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; }; // x,y pairs for width and height of textures
layout(std430, binding = 16) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors
layout(std430, binding = 17) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; }; // Palette starting indices for each texture
layout(std430, binding = 19) buffer LightIndices { float lights[]; };

uniform vec3 lightPos;
uniform int lightIdx;
uniform int face;

const int SHADOW_MAP_SIZE = 128;
const uint MATERIAL_IDX_MAX = 2048;
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
const vec3 baseDir = vec3(0.0, 0.0, 1.0);

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
    if (texIndex >= MATERIAL_IDX_MAX) return vec4(0.0,0.0,0.0,1.0);

    uint pixelOffset = textureOffsets[texIndex] + texCoord.y * textureSizes[texIndex].x + texCoord.x;
    uint slotIndex = pixelOffset / 2;
    uint packedIdx = colors[slotIndex];
    uint paletteIndex = (pixelOffset % 2 == 0) ? (packedIdx & 0xFFFF) : (packedIdx >> 16);
    uint paletteOffset = texturePaletteOffsets[texIndex];
    uint color = texturePalettes[paletteOffset + paletteIndex];
    return vec4(
        float((color >> 24) & 0xFF) / 255.0,
        float((color >> 16) & 0xFF) / 255.0,
        float((color >> 8) & 0xFF) / 255.0,
        float(color & 0xFF) / 255.0
    );
}

uint packColor(vec4 color) {
    uvec4 c = uvec4(clamp(color * 255.0, 0.0, 255.0));
    return (c.r << 24) | (c.g << 16) | (c.b << 8) | c.a;
}

void main() {
    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    int texIndexChecked = 0;
    if (TexIndex >= 0) texIndexChecked = int(TexIndex); 
    ivec2 texSize = textureSizes[texIndexChecked];
    vec2 uv = clamp(vec2(TexCoord.x, 1.0 - TexCoord.y), 0.0, 1.0); // Invert V (aka Y), OpenGL convention vs import
    ivec2 pixel = ivec2(uv);
    int x = int(floor(uv.x * float(texSize.x)));
    int y = int(floor(uv.y * float(texSize.y)));
    ivec2 texUV = ivec2(x,y);
    int ssbo_index = (lightIdx * 6 * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) + // Note: Slight performance boost hardcoding it
                 face * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE +
                 texelCoord.y * SHADOW_MAP_SIZE + texelCoord.x;

    reflectionColors[ssbo_index] = packColor(vec4(0.0,0.0,0.0,1.0));
    vec4 albedoColor = getTextureColor(texIndexChecked,texUV);
    if (albedoColor.a < 0.05) discard; // Alpha cutout threshold

    int packedIndex = ssbo_index >> 1;           // two values per uint
    bool upper      = false;//(ssbo_index & 1) == 1;
    uint oldVal     = depthData[packedIndex];
    uint stored     = upper ? (oldVal >> 16) & 0xFFFFu : oldVal & 0xFFFFu;
    if (stored == 0) stored = 0xFFFFu; // Keep GL driver from increasing RAM by 1gb by clearing it here instead of on CPU call.

    vec3 toLight = lightPos - FragPos;
    float dist = length(toLight);
    dist = clamp(dist, 0.0, 15.36);              // clip to max distance
    uint q = uint((dist / 15.36) * 65535.0 + 0.5);
    if (q < stored || stored == 0xFFFFu) {            // closer or uninitialized
        if (upper) oldVal = (oldVal & 0x0000FFFFu) | (q << 16);
        else       oldVal = (oldVal & 0xFFFF0000u) | q;

        depthData[packedIndex] = oldVal;
    }

    float intensity = lights[lightIdx + LIGHT_DATA_OFFSET_INTENSITY];
    float range = lights[lightIdx + LIGHT_DATA_OFFSET_RANGE];
    vec3 lightDir = normalize(toLight);
    float lambertian = max(dot(Normal, lightDir), 0.0);
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
        if (spotdot < cosAngle) {
            spotFalloff = 0.0;
        } else {
            float cosOuterAngle = cos(radians(spotAng / 2.0));
            float cosInnerAngle = cos(radians(spotAng * 0.8 / 2.0));
            spotFalloff = smoothstep(cosOuterAngle, cosInnerAngle, spotdot);
        }
    }

    float distOverRange = dist / range;
    float attenuation = (1.0 - (distOverRange * distOverRange)) * lambertian;
    vec3 lightColor = vec3(lights[lightIdx + LIGHT_DATA_OFFSET_R], lights[lightIdx + LIGHT_DATA_OFFSET_G], lights[lightIdx + LIGHT_DATA_OFFSET_B]);
    vec3 lighting = albedoColor.rgb * (intensity * 0.4) * pow(attenuation, 1.6) * lightColor * spotFalloff;
    reflectionColors[ssbo_index] = packColor(vec4(lighting.rgb,1.0));
}
