// depth_prepass.glsl: Renders all opaque objects prior to main forward+ pass
in vec2 TexCoord;
in vec3 FragPos;
layout(location=0) uniform uint instanceIndex;
layout(location=2) uniform mat4 viewProjection;
layout(location=3) uniform uint texIndex;
layout(std430,binding=12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430,binding=14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430,binding=15) buffer TextureSizes { ivec2 textureSizes[]; }; // x,y pairs for width and height of textures
layout(std430,binding=8) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors
layout(std430,binding=9) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; }; // Palette starting indices for each texture
vec4 getTextureColor(uint texIdx, ivec2 texCoord) {
    uint pixelOffset = textureOffsets[texIdx] + uint(texCoord.y) * textureSizes[texIndex].x + uint(texCoord.x);
    uint slotIndex = pixelOffset >> 2u;
    uint packedIdx = colors[slotIndex];
    uint localOffset = pixelOffset & 3u;
    uint paletteIndex = (packedIdx >> (localOffset << 3u)) & 0xFFu;
    uint paletteOffset = texturePaletteOffsets[texIdx];
    uint color = texturePalettes[paletteOffset + paletteIndex];
    return unpackUnorm4x8(color);
}

void main() {
    vec3 worldPos = FragPos.xyz;
    ivec2 texSize = textureSizes[texIndex];
    vec2 uv = (vec2(TexCoord.x, 1.0 - TexCoord.y)); // Invert V (aka Y), OpenGL convention vs import
    ivec2 texUV = ivec2(int(floor(uv.x * float(texSize.x))), int(floor(uv.y * float(texSize.y))));
    texUV.x = texUV.x % texSize.x;
    texUV.y = texUV.y % texSize.y;
    vec4 albedoColor = getTextureColor(texIndex,texUV);
    if (albedoColor.a < 1.0) discard; // Alpha cutout threshold
}
