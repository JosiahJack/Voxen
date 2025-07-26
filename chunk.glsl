// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
const char *vertexShaderSource =
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

const char *fragmentShaderTraditional =
    "#version 450 core\n"
    "#extension GL_ARB_shading_language_packing : require\n"
    "in vec2 TexCoord;\n"
    "in vec3 Normal;\n"
    "in vec3 FragPos;\n"

    "flat in int TexIndex;\n"
    "flat in int GlowIndex;\n"
    "flat in int SpecIndex;\n"
    "flat in int NormalIndex;\n"
    "flat in int InstanceIndex;\n"
    "flat in int ModelIndex;\n"

    "layout(std430, binding = 12) buffer ColorBuffer {\n"
    "    uint colors[];\n" // 1D color array (RGBA)
    "};\n"

    "layout(std430, binding = 13) buffer BlueNoise { float blueNoiseColors[]; };\n"

    "layout(std430, binding = 14) buffer TextureOffsets {\n"
    "    uint textureOffsets[];\n" // Starting index in colors for each texture
    "};\n"

    "layout(std430, binding = 15) buffer TextureSizes {\n"
    "    ivec2 textureSizes[];\n" // x,y pairs for width and height of textures
    "};\n"

    "layout(std430, binding = 16) buffer TexturePalettes {\n"
    "    uint texturePalettes[];\n"  // Palette colors
    "};\n"

    "layout(std430, binding = 17) buffer TexturePaletteOffsets {\n"
    "    uint texturePaletteOffsets[];\n"  // Palette starting indices for each texture
    "};\n"

    "layout(std430, binding = 19) buffer LightIndices { float lightInPVS[]; };\n"

    "vec4 getTextureColor(uint texIndex, ivec2 texCoord) {\n"
    "    if (texIndex >= 65535) return vec4(0.0);\n"

    "    uint pixelOffset = textureOffsets[texIndex] + texCoord.y * textureSizes[texIndex].x + texCoord.x;\n"
    "    uint paletteIndex = colors[pixelOffset];\n"
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

    "uniform int debugView;\n"
    "\n"
    "layout(location = 0) out vec4 outAlbedo;\n"   // GL_COLOR_ATTACHMENT0
    "layout(location = 1) out vec4 outWorldPos;\n" // GL_COLOR_ATTACHMENT1
    "layout(r8, binding = 2) uniform image2D inputShadowStencil;\n"
    "\n"
    "void main() {\n"
    "    int texIndexChecked = 0;\n"
    "    if (TexIndex >= 0) texIndexChecked = TexIndex;\n"
    "    ivec2 texSize = textureSizes[texIndexChecked];\n"

    "    vec2 uv = clamp(vec2(TexCoord.x, 1.0 - TexCoord.y), 0.0, 1.0);\n" // Invert V, OpenGL convention vs import
    "    int x = int(uv.x * float(texSize.x));\n"
    "    int y = int(uv.y * float(texSize.y));\n"
    "    vec4 albedoColor = getTextureColor(texIndexChecked,ivec2(x,y));\n"
    "    if (albedoColor.a < 0.05) {\n" // Alpha cutout threshold
    "        discard;\n" // And we're outta here!
    "    }\n"

    "    vec3 adjustedNormal = Normal;\n"
    "    if (!gl_FrontFacing) {\n"
    "        adjustedNormal = -Normal;\n"
    "    }\n"

    "    vec4 glowColor = getTextureColor(GlowIndex,ivec2(x,y));\n"
    "    vec4 specColor = getTextureColor(SpecIndex,ivec2(x,y));\n"
    "    vec4 worldPosPack = vec4(FragPos,intBitsToFloat(InstanceIndex));\n"
    "    vec3 worldPos = worldPosPack.xyz;\n"
    "    uint shadowStencil = uint(imageLoad(inputShadowStencil, ivec2(gl_FragCoord.xy)).r * 255.0);\n"
    "    if (debugView == 3) {\n"
    "        float ndcDepth = (2.0 * gl_FragCoord.z - 1.0);\n" // Depth debug
    "        float clipDepth = ndcDepth / gl_FragCoord.w;\n"
    "        float linearDepth = (clipDepth - 0.02) / (100.0 - 0.02);\n"
    "        outAlbedo = vec4(vec3(linearDepth), 1.0);\n"
    "    } else if (debugView == 1) {\n"
    "        outAlbedo = albedoColor;\n"
    "    } else if (debugView == 2) {\n"
    "        outAlbedo.r = adjustedNormal.x;\n"
    "        outAlbedo.g = adjustedNormal.y;\n"
    "        outAlbedo.b = adjustedNormal.z;\n"
    "        outAlbedo.a = 1.0;\n"
    "    } else if (debugView == 4) {\n"
    "        outAlbedo.r = float(InstanceIndex) / 5500.0;\n"
    "        outAlbedo.g = float(ModelIndex) / 668.0;\n"
    "        outAlbedo.b = float(texIndexChecked) / 1231.0;\n"
    "        outAlbedo.a = 1.0;\n"
    "    } else if (debugView == 5) {\n" // Shadows debug
    "        float shadowStencil = imageLoad(inputShadowStencil, ivec2(gl_FragCoord.xy)).r;\n"
    "        outAlbedo = vec4(shadowStencil,shadowStencil,shadowStencil,1.0);\n"
    "    } else {\n"
        "    vec3 lighting = vec3(0.0,0.0,0.0);\n"
        "    uint lightIdx = 0;\n"
        "    for (int i = 0; i < 32; i++) {\n"
        "        uint lightIdx = i * 12;\n" // LIGHT_DATA_SIZE
        "        float intensity = lightInPVS[lightIdx + 3];\n"

        "        float range = lightInPVS[lightIdx + 4];\n"
        "        vec3 lightPos = vec3(lightInPVS[lightIdx + 0], lightInPVS[lightIdx + 1], lightInPVS[lightIdx + 2]);\n"
        "        vec3 toLight = lightPos - worldPos;\n"
        "        float dist = length(toLight);\n"
        "        if (dist > range) continue;\n"

        "        vec3 lightDir = normalize(toLight);\n"
        "        float spotAng = lightInPVS[lightIdx + 5];\n"
        "        vec3 lightColor = vec3(lightInPVS[lightIdx + 9], lightInPVS[lightIdx + 10], lightInPVS[lightIdx + 11]);\n"
        "        float spotFalloff = 1.0;\n"
        "        if (spotAng > 0.0) {\n"
        "            vec3 spotDir = vec3(lightInPVS[lightIdx + 6], lightInPVS[lightIdx + 7], lightInPVS[lightIdx + 8]);\n"
        "            float spotdot = dot(spotDir, -lightDir);\n"
        "            float cosAngle = cos(radians(spotAng / 2.0));\n"
        "            if (spotdot < cosAngle) continue;\n"
        "            float cosOuterAngle = cos(radians(spotAng / 2.0));\n"
        "            float cosInnerAngle = cos(radians(spotAng * 0.8 / 2.0));\n"
        "            spotFalloff = smoothstep(cosOuterAngle, cosInnerAngle, spotdot);\n"
        "            if (spotFalloff <= 0.0) continue;\n"
        "        }\n"

        "        float shadow = 1.0;\n"
//         "        if ((shadowStencil & (1u << i)) < 1) shadow = 0.0;\n"

        "        float attenuation = (1.0 - (dist / range)) * max(dot(adjustedNormal, lightDir), 0.0);\n"
        "        attenuation *= shadow;\n"

        "        lighting += albedoColor.rgb * intensity * attenuation * lightColor * spotFalloff;\n"
        "    }\n"

        "    lighting += glowColor.rgb;\n"

        // Dither
        "    int pixelIndex = int(((int(gl_FragCoord.y) % 64) * 64 + (int(gl_FragCoord.x) % 64))) * 3;\n" // Calculate 1D index.  * 4 for four rgba values.
        "    vec4 bluenoise = vec4(blueNoiseColors[pixelIndex], blueNoiseColors[pixelIndex + 1], blueNoiseColors[pixelIndex + 2], 1.0);\n"
        "    lighting += ((bluenoise.rgb * 1.0/255.0) - (0.5/255.0));\n"

    "        outAlbedo = vec4(lighting,1.0);\n"
    "    }\n"
    "    outWorldPos = worldPosPack;\n"
    "}\n";
