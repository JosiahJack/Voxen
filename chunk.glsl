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

    "uniform int debugView;\n"
    "uniform int shadowsEnabled;\n"
    "uniform int instancesInPVSCount;\n"
    "uniform float overrideGlowR = 0.0;\n"
    "uniform float overrideGlowG = 0.0;\n"
    "uniform float overrideGlowB = 0.0;\n"

    "flat in int TexIndex;\n"
    "flat in int GlowIndex;\n"
    "flat in int SpecIndex;\n"
    "flat in int NormalIndex;\n"
    "flat in int InstanceIndex;\n"
    "flat in int ModelIndex;\n"

    "struct Instance {\n"
    "    int modelIndex;\n"
    "    int texIndex;\n"
    "    int glowIndex;\n"
    "    int specIndex;\n"
    "    int normIndex;\n"
    "    float posx;\n"
    "    float posy;\n"
    "    float posz;\n"
    "    float sclx;\n"
    "    float scly;\n"
    "    float sclz;\n"
    "    float rotx;\n"
    "    float roty;\n"
    "    float rotz;\n"
    "    float rotw;\n"
    "};\n"

    "layout(location = 0) out vec4 outAlbedo;\n"   // GL_COLOR_ATTACHMENT0
    "layout(location = 1) out vec4 outWorldPos;\n" // GL_COLOR_ATTACHMENT1
    "layout(r8, binding = 2) uniform image2D inputShadowStencil;\n"

//     "layout(std430, binding = 6) readonly buffer ModelVertexOffsets { uint vertexOffsets[]; };\n"
    "layout(std430, binding = 7) buffer BoundsBuffer { float bounds[]; };\n"
//     "layout(std430, binding = 8) readonly buffer ModelVertexCounts { uint modelVertexCounts[]; };\n"
    "layout(std430, binding = 9) readonly buffer InstancesInPVS { uint instancesIndices[]; };\n"
    "layout(std430, binding = 10) readonly buffer InstancesBuffer { Instance instances[]; };\n"
    "layout(std430, binding = 11) readonly buffer InstancesMatricesBuffer { mat4 instanceMatrices[]; };\n"
    "layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; };\n" // 1D color array (RGBA)
    "layout(std430, binding = 13) buffer BlueNoise { float blueNoiseColors[]; };\n"
    "layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; };\n" // Starting index in colors for each texture
    "layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; };\n" // x,y pairs for width and height of textures
    "layout(std430, binding = 16) buffer TexturePalettes { uint texturePalettes[]; };\n" // Palette colors
    "layout(std430, binding = 17) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; };\n" // Palette starting indices for each texture

    "layout(std430, binding = 19) buffer LightIndices { float lightInPVS[]; };\n"
//     "layout(std430, binding = 20) readonly buffer MasterVertexBuffer { float vertexData[]; };\n"

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

    // --- Ray-Triangle Intersection (Möller-Trumbore) ---
    "bool RayTriangle(vec3 origin, vec3 dir, vec3 v0, vec3 v1, vec3 v2, out float t) {\n"
    "    vec3 edge1 = v1 - v0;\n"
    "    vec3 edge2 = v2 - v0;\n"
    "    vec3 h = cross(dir, edge2);\n"
    "    float a = dot(edge1, h);\n"
    "    if (abs(a) < 1e-6) return false;\n"

    "    float f = 1.0 / a;\n"
    "    vec3 s = origin - v0;\n"
    "    float u = f * dot(s, h);\n"
    "    if (u < 0.0 || u > 1.0) return false;\n"

    "    vec3 q = cross(s, edge1);\n"
    "    float v = f * dot(dir, q);\n"
    "    if (v < 0.0 || u + v > 1.0) return false;\n"

    "    t = f * dot(edge2, q);\n"
    "    return t > 0.001;\n"
    "}\n"

    // --- Trace Ray for Shadow ---
    "const uint VERTEX_ATTRIBUTES_COUNT = 14;\n"
    "const uint BOUNDS_ATTRIBUTES_COUNT = 7;\n"
//     "float TraceRay(vec3 origin, vec3 dir, float maxDist) {\n"
//     "    for (int i = 0; i < instancesInPVSCount; i++) {\n"
//     "        uint instanceIdx = instancesIndices[i];\n"
//     "        Instance inst = instances[instanceIdx];\n"
//     "        if (inst.texIndex == 881) continue;\n" // Fullbright light
// 
//     "        mat4 invModel = inverse(instanceMatrices[instanceIdx]);\n"
//     "        vec3 localOrigin = (invModel * vec4(origin, 1.0)).xyz;\n"
//     "        float instanceRadius = bounds[instanceIdx * BOUNDS_ATTRIBUTES_COUNT + 6];\n" // first 6 are the mins,maxs xyz
//     "        if (length(localOrigin - origin) > (maxDist + instanceRadius)) continue;\n"
// 
//     "        vec3 localDir = ((invModel * vec4(dir, 0.0)).xyz);\n"
//     "        uint modelIndex = inst.modelIndex;\n"
//     "        uint vertCount = modelVertexCounts[modelIndex];\n"
//     "        if (vertCount > 1000) continue;\n"
// 
//     "        uint triCount = vertCount / 3;\n"
//     "        mat4 matrix = instanceMatrices[instanceIdx];\n"
//     "        uint j = 0;\n"
//     "        uint vertexIdx;\n"
//     "        vec3 v0, v1, v2;\n"
//     "        for (uint tri = 0; tri < triCount; tri++) {\n"
//     "            vertexIdx = (vertexOffsets[modelIndex] * VERTEX_ATTRIBUTES_COUNT) + (tri * VERTEX_ATTRIBUTES_COUNT);\n"
//     "            j = 0;\n"
//     "            v0 = vec3(vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 0], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 1], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 2]);\n"
//     "            j++;\n"
//     "            v1 = vec3(vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 0], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 1], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 2]);\n"
//     "            j++;\n"
//     "            v2 = vec3(vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 0], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 1], vertexData[vertexIdx + j * VERTEX_ATTRIBUTES_COUNT + 2]);\n"
//     "            float t;\n" // Output result
//     "            if (RayTriangle(localOrigin, localDir, v0, v1, v2, t) && (t < maxDist)) return 0.0;\n"
//     "        }\n"
//     "    }\n"
//     "    return 1.0;\n"
//     "}\n"

    "void main() {\n"
    "    int texIndexChecked = 0;\n"
    "    if (TexIndex >= 0) texIndexChecked = TexIndex;\n"
    "    ivec2 texSize = textureSizes[texIndexChecked];\n"

    "    vec2 uv = clamp(vec2(TexCoord.x, 1.0 - TexCoord.y), 0.0, 1.0);\n" // Invert V, OpenGL convention vs import
    "    int x = int(uv.x * float(texSize.x));\n"
    "    int y = int(uv.y * float(texSize.y));\n"
    "    vec4 albedoColor = getTextureColor(texIndexChecked,ivec2(x,y));\n"
//     "    if (albedoColor.a < 0.05) {\n" // Alpha cutout threshold
//     "        discard;\n" // And we're outta here!
//     "    }\n"

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
    "    } else if (debugView == 6) {\n" // Lightview Mode
    "        outAlbedo = vec4(overrideGlowR,overrideGlowG,overrideGlowB,1.0);\n"
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
//         "        if (i == 0 && shadowsEnabled > 0 && intensity > 0.5 && range > 1.5) {\n"
//         "            shadow = TraceRay(worldPos + adjustedNormal * 0.01, lightDir, range);\n"
//         "        }\n"

        "        float attenuation = (1.0 - (dist / range)) * max(dot(adjustedNormal, lightDir), 0.0);\n"
        "        attenuation *= shadow;\n"

        "        lighting += albedoColor.rgb * intensity * attenuation * lightColor * spotFalloff;\n"
        "    }\n"

        "    lighting += glowColor.rgb;\n"

        // Dither
        "    int pixelIndex = int(((int(gl_FragCoord.y) % 64) * 64 + (int(gl_FragCoord.x) % 64))) * 3;\n" // Calculate 1D index.  * 4 for four rgba values.
        "    vec4 bluenoise = vec4(blueNoiseColors[pixelIndex], blueNoiseColors[pixelIndex + 1], blueNoiseColors[pixelIndex + 2], 1.0);\n"
        "    lighting += ((bluenoise.rgb * 1.0/255.0) - (0.5/255.0));\n"

    "        outAlbedo = vec4(lighting,albedoColor.a);\n"
    "    }\n"
    "    outWorldPos = worldPosPack;\n"
    "}\n";
