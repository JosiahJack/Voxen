// Shadowmap Fragment Shader
#version 430 core
in vec3 FragPos;
in vec2 TexCoord;

layout(std430,  binding = 5) buffer ShadowMaps { uint depthData[]; };
layout(std430, binding = 19) buffer LightIndices { float lights[]; };

// layout(location = 0) uniform uint instanceIndex; // start vert shader uniforms
// layout(location = 1) uniform mat4 viewProjMatrix; // end vert shader uniforms
layout(location = 2) uniform uint face;
layout(location = 3) uniform uint lightIndex;
layout(location = 4) uniform uint shadowmapSize;
layout(location = 5) uniform uint shadowmapIndirection;
layout(location = 6) uniform uint texIndex;
layout(location = 7) uniform uint offsetIntoSSBO;
layout(location = 8) uniform uint isTransparent;

layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; }; // 1D color array (RGBA)
layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; }; // Starting index in colors for each texture
layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; }; // x,y pairs for width and height of textures
layout(std430, binding = 16) buffer TexturePalettes { uint texturePalettes[]; }; // Palette colors
layout(std430, binding = 17) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; }; // Palette starting indices for each texture

const int LIGHT_DATA_SIZE = 13;
const int LIGHT_DATA_OFFSET_POSX = 0;
const int LIGHT_DATA_OFFSET_POSY = 1;
const int LIGHT_DATA_OFFSET_POSZ = 2;
const vec4 BYTE_TO_FLOAT = vec4(1.0/255.0);

uint getTextureAlpha(uint texIndex, ivec2 texCoord) {
    uint pixelOffset = textureOffsets[texIndex] + uint(texCoord.y) * textureSizes[texIndex].x + uint(texCoord.x);
    uint slotIndex = pixelOffset / 4u;
    uint packedIdx = colors[slotIndex];
    uint localOffset = pixelOffset % 4u;
    uint paletteIndex = (packedIdx >> (8u * localOffset)) & 0xFFu;
    uint paletteOffset = texturePaletteOffsets[texIndex];
    uint color = texturePalettes[paletteOffset + paletteIndex];
    return color>>24;
}

void main() {
    if (isTransparent > 0) {
        int texIndexChecked = 0;
        if (texIndex >= 0) texIndexChecked = int(texIndex);
        ivec2 texSize = textureSizes[texIndexChecked];
        vec2 uv = (vec2(TexCoord.x, 1.0 - TexCoord.y)); // Invert V (aka Y), OpenGL convention vs import
        ivec2 pixel = ivec2(uv);
        ivec2 texUV = ivec2(int(floor(uv.x * float(texSize.x))), int(floor(uv.y * float(texSize.y))));
        texUV.x = texUV.x % texSize.x;
        texUV.y = texUV.y % texSize.y;
        if (getTextureAlpha(texIndexChecked,texUV) < 252u) return; // Alpha cutout threshold for {fence style textures
    }

    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    uint ssbo_indexBase = offsetIntoSSBO + (face * shadowmapSize * shadowmapSize);
    uint ssbo_index = ssbo_indexBase + texelCoord.y * shadowmapSize + texelCoord.x;
    vec3 lightPos = vec3(lights[lightIndex + LIGHT_DATA_OFFSET_POSX], lights[lightIndex + LIGHT_DATA_OFFSET_POSY], lights[lightIndex + LIGHT_DATA_OFFSET_POSZ]);
    vec3 toLight = lightPos - FragPos;
    float dist = length(toLight);
    uint distInt = uint(dist * 100000.0);
    atomicMin(depthData[ssbo_index], distInt);
}
