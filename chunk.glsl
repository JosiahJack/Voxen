// Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
const char *vertexShaderSource =
    "#version 450 core\n"
    "\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "layout(location = 2) in vec2 aTexCoord;\n"
    "uniform int texIndex;\n"
    "uniform int instanceIndex;\n"
    "uniform int instanceCount;\n"
    "uniform int modelIndex;\n"
    "uniform int modelCount;\n"
    "uniform int textureCount;\n"
    "uniform mat4 matrix;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec3 FragPos;\n"
    "out vec3 Normal;\n"
    "out vec2 TexCoord;\n"
    "flat out int TexIndex;\n"
    "flat out int TextureCount;\n"
    "flat out int InstanceIndex;\n"
    "flat out int InstanceCount;\n"
    "flat out int ModelIndex;\n"
    "flat out int ModelCount;\n"
    "\n"
    "void main() {\n"
    "    FragPos = vec3(matrix * vec4(aPos, 1.0));\n"
    "    Normal = mat3(transpose(inverse(matrix))) * aNormal;\n"
    "    TexCoord = aTexCoord;\n"
    "    TexIndex = texIndex;\n"
    "    TextureCount = textureCount;\n"
    "    ModelIndex = modelIndex;\n"
    "    ModelCount = modelCount;\n"
    "    InstanceIndex = instanceIndex;\n"
    "    InstanceCount = instanceCount;\n"
    "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\n";

const char *fragmentShaderTraditional =
    "#version 450 core\n"
    "\n"
    "in vec2 TexCoord;\n"
    "flat in int TexIndex;\n"
    "flat in int TextureCount;\n"
    "flat in int InstanceIndex;\n"
    "flat in int InstanceCount;\n"
    "flat in int ModelIndex;\n"
    "flat in int ModelCount;\n"
    "in vec3 Normal;\n"
    "in vec3 FragPos;\n"
    "\n"
    "layout(std430, binding = 12) buffer ColorBuffer {\n"
    "    uint colors[];\n" // 1D color array (RGBA)
    "};\n"

    "layout(std430, binding = 14) buffer TextureOffsets {\n"
    "    uint textureOffsets[];\n" // Starting index in colors for each texture
    "};\n"

    "layout(std430, binding = 15) buffer TextureSizes {\n"
    "    ivec2 textureSizes[];\n" // x,y pairs for width and height of textures
    "};\n"

    "uniform int debugView;\n"
    "\n"
    "layout(location = 0) out vec4 outAlbedo;\n"              // GL_COLOR_ATTACHMENT0
    "layout(location = 1) out vec4 outNormal;\n"              // GL_COLOR_ATTACHMENT1
    "layout(location = 2) out vec4 outWorldPos;\n"            // GL_COLOR_ATTACHMENT2
    "layout(location = 3) out ivec4 outModelInstanceIndex;\n" // GL_COLOR_ATTACHMENT3
    "\n"
    "void main() {\n"
    "    int texIndexChecked = 0;\n"
    "    if (TexIndex < TextureCount) texIndexChecked = TexIndex;\n"
    "    ivec2 texSize = textureSizes[texIndexChecked];\n"

    "    vec2 uv = clamp(vec2(TexCoord.x, 1.0 - TexCoord.y), 0.0, 1.0);\n" // Invert V, OpenGL convention vs import
    "    int x = int(uv.x * float(texSize.x));\n"
    "    int y = int(uv.y * float(texSize.y));\n"
    "    int pixelIndex = int(textureOffsets[texIndexChecked]) + (y * texSize.x + x);\n" // Calculate 1D index
    "    outModelInstanceIndex = ivec4(InstanceIndex,ModelIndex,texIndexChecked, 1.0);\n"
    "    if (debugView == 3) {\n"
    "        float ndcDepth = (2.0 * gl_FragCoord.z - 1.0);\n" // Depth debug
    "        float clipDepth = ndcDepth / gl_FragCoord.w;\n"
    "        float linearDepth = (clipDepth - 0.02) / (100.0 - 0.02);\n"
    "        outAlbedo = vec4(vec3(linearDepth), 1.0);\n"
    "    } else if (debugView == 4) {\n"
    "        outAlbedo.r = float(InstanceIndex) / float(InstanceCount);\n"
    "        outAlbedo.g = float(ModelIndex) / float(ModelCount);\n"
    "        outAlbedo.b = float(texIndexChecked) / float(TextureCount);\n"
    "        outAlbedo.a = 1.0;\n"
    "    } else {\n"
        // Unpack RGBA from a single uint
    "        uint color = colors[pixelIndex];\n"
    "        float r = float((color >> 24) & 0xFF) / 255.0;\n" // Extract red
    "        float g = float((color >> 16) & 0xFF) / 255.0;\n" // Extract green
    "        float b = float((color >> 8) & 0xFF) / 255.0;\n"  // Extract blue
    "        float a = float(color & 0xFF) / 255.0;\n"         // Extract alpha
    "        outAlbedo  = vec4(r, g, b, a);\n"
    "    }\n"
    "    outNormal = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);\n"
    "    outWorldPos = vec4(FragPos, 1.0);\n"
    "}\n";
