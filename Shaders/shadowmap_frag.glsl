// Shadowmap Fragment Shader
#version 430 core
in vec3 FragPos;
in vec2 TexCoord;

// layout(location = 0) out uint outShadowmap;
layout(location = 0) out float outShadowmap;

layout(std430, binding = 19) buffer LightIndices { float lights[]; };

layout(location = 3) uniform vec3 lightPos;
layout(location = 6) uniform uint texIndex;
layout(location = 8) uniform uint isTransparent;

layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; }; // x,y pairs for width and height of textures
layout(std430, binding = 16) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors
layout(std430, binding = 17) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; }; // Palette starting indices for each texture

uint getTextureAlpha(uint texIndex, ivec2 texCoord) {
    uint pixelOffset = textureOffsets[texIndex] + uint(texCoord.y) * textureSizes[texIndex].x + uint(texCoord.x);
    uint slotIndex = pixelOffset >> 2u;// / 4u;
    uint packedIdx = colors[slotIndex];
    uint localOffset = pixelOffset & 3u;//% 4u;
    uint paletteIndex = (packedIdx >> (localOffset << 3u)) & 0xFFu; // << 3u is same as * 8
    uint paletteOffset = texturePaletteOffsets[texIndex];
    uint color = texturePalettes[paletteOffset + paletteIndex];
    return color>>24;
}

void main() {
    if (isTransparent > 0) {
        ivec2 texSize = textureSizes[texIndex];
        vec2 uv = vec2(TexCoord.x, 1.0 - TexCoord.y); // Invert V (aka Y), OpenGL convention vs import
        ivec2 texUV = ivec2(uv * vec2(texSize));
        texUV &= texSize - ivec2(1);
        if (getTextureAlpha(texIndex,texUV) < 252u) discard; // Alpha cutout threshold for {fence style textures
    }

    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    vec3 toLight = lightPos - FragPos;
    float dist = length(toLight);
    outShadowmap = dist;
}
