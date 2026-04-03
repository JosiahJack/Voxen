// Shadowmap Fragment Shader
#version 430 core
in vec3 FragPos;
in vec2 TexCoord;
layout(std430,binding=5) buffer ShadowMaps { uint depthData[]; };
layout(location=2) uniform uint face;
layout(location=3) uniform vec3 lightPos;
layout(location=6) uniform uint texIndex;
layout(location=7) uniform uint offsetIntoSSBO;
layout(location=8) uniform uint isTransparent;
layout(location=9) uniform uint shadowMapSize;
layout(location=10) uniform ivec2 texSize;
layout(location=11) uniform uint texPaletteOffset;
layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430, binding = 16) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors

void main() {
    if (isTransparent > 0u) {
        vec2 uv = vec2(TexCoord.x,1.0 - TexCoord.y);
        ivec2 texUV = ivec2(uv * vec2(texSize)) & (texSize - ivec2(1));
        uint pixelOffset = textureOffsets[texIndex] + uint(texUV.y) * texSize.x + uint(texUV.x);
        uint slotIndex = pixelOffset >> 2u;
        uint packedIdx = colors[slotIndex];
        uint localOffset = pixelOffset & 3u;
        uint paletteIndex = (packedIdx >> (localOffset * 8u)) & 0xFFu;
        uint color = texturePalettes[texPaletteOffset + paletteIndex];
        if ((color >> 24) < 252u) discard;
    }

    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    uint ssbo_index = offsetIntoSSBO + texelCoord.y * shadowMapSize + texelCoord.x;
    vec3 toLight = lightPos - FragPos;
    float dist = length(toLight) + 0.02; // Slight pre-bias
    uint distInt = uint(dist * 100000.0 + 0.5);
    atomicMin(depthData[ssbo_index],distInt);
}
