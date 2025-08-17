// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
const char* vertexShaderSource =
    "#version 450 core\n"

    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "layout(location = 2) in vec2 aTexCoord;\n"

    "uniform int texIndex;\n"
    "uniform int glowIndex;\n"
    "uniform int specIndex;\n"
    "uniform int normalIndex;\n"
    "uniform int instanceIndex;\n"
    "uniform int modelIndex;\n"
    "uniform mat4 matrix;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"

    "out vec3 FragPos;\n"
    "out vec3 Normal;\n"
    "out vec2 TexCoord;\n"

    "flat out int TexIndex;\n"
    "flat out int GlowIndex;\n"
    "flat out int SpecIndex;\n"
    "flat out int NormalIndex;\n"
    "flat out int InstanceIndex;\n"
    "flat out int ModelIndex;\n"

    "void main() {\n"
    "    FragPos = vec3(matrix * vec4(aPos, 1.0));\n" // Convert vertex from the model's local space into world space
    "    Normal = mat3(transpose(inverse(matrix))) * aNormal;\n"
    "    TexCoord = aTexCoord;\n" // Pass along data to each vertex, shared for whole tri's pixels.
    "    TexIndex = texIndex;\n"
    "    GlowIndex = glowIndex;\n"
    "    SpecIndex = specIndex;\n"
    "    NormalIndex = normalIndex;\n"
    "    ModelIndex = modelIndex;\n"
    "    InstanceIndex = instanceIndex;\n"
    "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\n";

const char* fragmentShaderTraditional =
    "#version 450 core\n"
    "#extension GL_ARB_shading_language_packing : require\n"

    "in vec2 TexCoord;\n"
    "in vec3 Normal;\n"
    "in vec3 FragPos;\n"

    "uniform int debugView;\n"
    "uniform float overrideGlowR = 0.0;\n"
    "uniform float overrideGlowG = 0.0;\n"
    "uniform float overrideGlowB = 0.0;\n"

    "flat in int TexIndex;\n"
    "flat in int GlowIndex;\n"
    "flat in int SpecIndex;\n"
    "flat in int NormalIndex;\n"
    "flat in int InstanceIndex;\n"
    "flat in int ModelIndex;\n"

    "layout(location = 0) out vec4 outAlbedo;\n"   // GL_COLOR_ATTACHMENT0
    "layout(location = 1) out vec4 outWorldPos;\n" // GL_COLOR_ATTACHMENT1
    "layout(location = 2) out vec4 outNormal;\n"   // GL_COLOR_ATTACHMENT2

    "layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; };\n" // 1D color array (RGBA)
    "layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; };\n" // Starting index in colors for each texture
    "layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; };\n" // x,y pairs for width and height of textures
    "layout(std430, binding = 16) buffer TexturePalettes { uint texturePalettes[]; };\n" // Palette colors
    "layout(std430, binding = 17) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; };\n" // Palette starting indices for each texture

    "vec4 getTextureColor(uint texIndex, ivec2 texCoord) {\n"
    "    if (texIndex >= 65535) return vec4(0.0);\n"

    "    uint pixelOffset = textureOffsets[texIndex] + texCoord.y * textureSizes[texIndex].x + texCoord.x;\n"
    "    uint slotIndex = pixelOffset / 2;\n"
    "    uint packedIdx = colors[slotIndex];\n"
    "    uint paletteIndex = (pixelOffset % 2 == 0) ? (packedIdx & 0xFFFF) : (packedIdx >> 16);\n"
    "    uint paletteOffset = texturePaletteOffsets[texIndex];\n"
    "    uint color = texturePalettes[paletteOffset + paletteIndex];\n"
    "    return vec4(\n"
    "        float((color >> 24) & 0xFF) / 255.0,\n"
    "        float((color >> 16) & 0xFF) / 255.0,\n"
    "        float((color >> 8) & 0xFF) / 255.0,\n"
    "        float(color & 0xFF) / 255.0\n"
    "    );\n"
    "}\n"

    "uint packColor(vec4 color) {\n"
    "    uvec4 c = uvec4(clamp(color * 255.0, 0.0, 255.0));\n"
    "    return (c.r << 24) | (c.g << 16) | (c.b << 8) | c.a;\n"
    "}\n"

    "void main() {\n"
    "    int texIndexChecked = 0;\n"
    "    if (TexIndex >= 0) texIndexChecked = TexIndex;\n"
    "    ivec2 texSize = textureSizes[texIndexChecked];\n"
    "    vec2 uv = clamp(vec2(TexCoord.x, 1.0 - TexCoord.y), 0.0, 1.0);\n" // Invert V, OpenGL convention vs import
    "    int x = int(uv.x * float(texSize.x));\n"
    "    int y = int(uv.y * float(texSize.y));\n"
    "    vec4 albedoColor = getTextureColor(texIndexChecked,ivec2(x,y));\n"
    "    if (albedoColor.a < 0.05) discard;\n" // Alpha cutout threshold

    "    vec3 adjustedNormal = Normal;\n"
    "    if (!gl_FrontFacing) adjustedNormal = -Normal;\n"
    "    vec4 glowColor = getTextureColor(GlowIndex,ivec2(x,y));\n"
    "    vec4 specColor = getTextureColor(SpecIndex,ivec2(x,y));\n"
    "    vec4 worldPosPack = vec4(FragPos,intBitsToFloat(InstanceIndex));\n"
    "    vec3 worldPos = worldPosPack.xyz;\n"
    "    if (debugView == 1) {\n"
    "        outAlbedo = albedoColor;\n"
    "    } else if (debugView == 2) {\n"
    "        outAlbedo.r = (adjustedNormal.x + 1.0) * 0.5f;\n"
    "        outAlbedo.g = (adjustedNormal.y + 1.0) * 0.5f;\n"
    "        outAlbedo.b = (adjustedNormal.z + 1.0) * 0.5f;\n"
    "        outAlbedo.a = 1.0;\n"
    "    } else if (debugView == 3) {\n"
    "        float ndcDepth = (2.0 * gl_FragCoord.z - 1.0);\n" // Depth debug
    "        float clipDepth = ndcDepth / gl_FragCoord.w;\n"
    "        float linearDepth = (clipDepth - 0.02) / (100.0 - 0.02);\n"
    "        outAlbedo = vec4(vec3(linearDepth), 1.0);\n"
    "    } else if (debugView == 4) {\n"
    "        outAlbedo.r = float(InstanceIndex) / 5500.0;\n"
    "        outAlbedo.g = float(ModelIndex) / 668.0;\n"
    "        outAlbedo.b = float(texIndexChecked) / 1231.0;\n"
    "        outAlbedo.a = 1.0;\n"
    "    } else if (debugView == 5) {\n" // Worldpos debug
    "        outAlbedo.rgb = worldPos;\n"
    "        outAlbedo.a = 1.0;\n"
    "    } else if (debugView == 6) {\n" // Lightview Mode
    "        outAlbedo = vec4(overrideGlowR,overrideGlowG,overrideGlowB,1.0);\n"
    "    } else {\n"
    "        outAlbedo = albedoColor;\n"
    "    }\n"
    "    outNormal.r = uintBitsToFloat(packHalf2x16(adjustedNormal.xy));\n"
    "    outNormal.g = uintBitsToFloat(packHalf2x16(vec2(adjustedNormal.z,0.0)));\n"
    "    outNormal.b = uintBitsToFloat(packColor(glowColor));\n"
    "    outNormal.a = uintBitsToFloat(packColor(specColor));\n"
    "    outWorldPos = worldPosPack;\n"
    "}\n";
