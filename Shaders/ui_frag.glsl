// ui_frag.glsl: Generic shader for unlit textured UI images (mostly cutouts)
in vec2 TexCoord;
layout(location=0) uniform uint texIndex;
layout(location=0) out vec4 outUI; // GL_COLOR_ATTACHMENT0
layout(std430,binding=12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430,binding=14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430,binding=15) buffer TextureSizes { ivec2 textureSizes[]; }; // x,y pairs for width and height of textures
layout(std430,binding=16) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors
layout(std430,binding=17) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; }; // Palette starting indices for each texture
const vec4 BYTE_TO_FLOAT = vec4(1.0/255.0);
vec4 getTextureColor(uint texIndex, ivec2 texCoord, int texSizeX) {
    uint pixelOffset = textureOffsets[texIndex] + uint(texCoord.y) * texSizeX + uint(texCoord.x);
    uint slotIndex = pixelOffset >> 2u;
    uint packedIdx = colors[slotIndex];
    uint localOffset = pixelOffset & 3u;
    uint paletteIndex = (packedIdx >> (localOffset << 3u)) & 0xFFu; // << 3u is same as * 8
    uint paletteOffset = texturePaletteOffsets[texIndex];
    uint color = texturePalettes[paletteOffset + paletteIndex];
    return vec4(color & 0xFFu,(color>>8)&0xFFu,(color>>16)&0xFFu,color>>24) * BYTE_TO_FLOAT;
}

void main() {
    ivec2 texSize = textureSizes[texIndex];
    vec2 uv = (vec2(TexCoord.x, 1.0 - TexCoord.y)); // Invert V (aka Y), OpenGL convention vs import
    ivec2 texUV = ivec2(int(floor(uv.x * float(texSize.x))), int(floor(uv.y * float(texSize.y))));
    texUV.x = texUV.x % texSize.x; texUV.y = texUV.y % texSize.y;
    vec4 albedoColor = getTextureColor(texIndex,texUV,texSize.x);
    if (albedoColor.a < 0.05) discard; // Alpha cutout threshold
    outUI = albedoColor;
}
