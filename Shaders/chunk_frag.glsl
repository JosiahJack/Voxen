// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
#version 450 core
#extension GL_ARB_shading_language_packing : require

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform int debugView;

flat in uint TexIndex;
flat in uint GlowIndex;
flat in uint SpecIndex;
flat in uint NormalIndex;
flat in uint InstanceIndex;
const uint MATERIAL_IDX_MAX = 2048;

layout(location = 0) out vec4 outAlbedo;   // GL_COLOR_ATTACHMENT0
layout(location = 1) out vec4 outWorldPos; // GL_COLOR_ATTACHMENT1

layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; }; // x,y pairs for width and height of textures
layout(std430, binding = 16) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors
layout(std430, binding = 17) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; }; // Palette starting indices for each texture

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
    int texIndexChecked = 0;
    if (TexIndex >= 0) texIndexChecked = int(TexIndex);
    ivec2 texSize = textureSizes[texIndexChecked];
    vec2 uv = clamp(vec2(TexCoord.x, 1.0 - TexCoord.y), 0.0, 1.0); // Invert V, OpenGL convention vs import
    int x = int(floor(uv.x * float(texSize.x)));
    int y = int(floor(uv.y * float(texSize.y)));
    vec4 albedoColor = getTextureColor(texIndexChecked,ivec2(x,y));
    if (albedoColor.a < 0.05) discard; // Alpha cutout threshold

    vec3 adjustedNormal = Normal;
    if (NormalIndex < MATERIAL_IDX_MAX && NormalIndex > 0 && NormalIndex != 41) {
        vec3 dp1 = dFdx(FragPos);
        vec3 dp2 = dFdy(FragPos);
        vec2 duv1 = dFdx(TexCoord);
        vec2 duv2 = dFdy(TexCoord);
        float uvArea = abs(duv1.x * duv2.y - duv1.y * duv2.x);
        if (uvArea > 1e-4) {
            vec3 t = normalize(dp1 * duv2.y - dp2 * duv1.y);
            vec3 b = normalize(-dp1 * duv2.x + dp2 * duv1.x);
            mat3 TBN3x3 = mat3(t, b, Normal);
            vec3 normalColor = normalize(getTextureColor(NormalIndex,ivec2(x,y)).rgb * 2.0 - 1.0);
            normalColor.g = -normalColor.g;
            adjustedNormal = normalize(TBN3x3 * normalColor);
        }
    }

    if (!gl_FrontFacing) adjustedNormal = -adjustedNormal;
    vec4 glowColor = getTextureColor(GlowIndex,ivec2(x,y));
    outAlbedo.a = glowColor.r;
    glowColor.r = (adjustedNormal.z + 1.0) * 0.5;
    glowColor.a = (adjustedNormal.x + 1.0) * 0.5;
    vec4 specColor = getTextureColor(SpecIndex,ivec2(x,y));
    specColor.a = (adjustedNormal.y + 1.0) * 0.5;
    vec4 worldPosPack = vec4(uintBitsToFloat(packHalf2x16(FragPos.xy)),
                             uintBitsToFloat(packHalf2x16(vec2(FragPos.z,uintBitsToFloat(InstanceIndex)))),
                             uintBitsToFloat(packColor(glowColor)),
                             uintBitsToFloat(packColor(specColor)) );
    outWorldPos = worldPosPack;
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
        float linearDepth = (clipDepth - 0.02) / (100.0 - 0.02);
        outAlbedo = vec4(vec3(linearDepth), 1.0);
    } else if (debugView == 4) {
        outAlbedo.r = float(InstanceIndex) / 5500.0;
        outAlbedo.g = 0.0;
        outAlbedo.b = float(texIndexChecked) / 1231.0;
        outAlbedo.a = 1.0;
    } else if (debugView == 5) { // Worldpos debug
        outAlbedo.rgb = worldPosPack.xyz;
        outAlbedo.a = 1.0;
    } else {
        outAlbedo.rgb = albedoColor.rgb * albedoColor.a;
    }
}
