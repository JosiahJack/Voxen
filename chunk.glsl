// ----------------------------------------------------------------------------
// Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
const char *vertexShaderSource =
    "#version 450 core\n"
    "\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "layout(location = 2) in vec2 aTexCoord;\n"
    "uniform int texIndex;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec3 FragPos;\n"
    "out vec3 Normal;\n"
    "out vec2 TexCoord;\n"
    "flat out int TexIndex;\n"
    "\n"
    "void main() {\n"
    "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
    "    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
    "    TexCoord = aTexCoord;\n"
    "    TexIndex = texIndex;\n"
    "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\n";

const char *fragmentShaderTraditional =
    "#version 450 core\n"
    "\n"
    "in vec2 TexCoord;\n"
    "flat in int TexIndex;\n"
    "in vec3 Normal;\n"
    "\n"
    "layout(std430, binding = 0) buffer ColorBuffer {\n"
    "    float colors[];\n" // 1D color array (RGBA)
    "};\n"
    "uniform uint textureOffsets[3];\n" // Offsets for each texture
    "uniform ivec2 textureSizes[3];\n" // Width, height for each texture
    "\n"
    "layout(location = 0) out vec4 outAlbedo;\n"
    "layout(location = 1) out vec4 outNormal;\n"
    "\n"
    "void main() {\n"
    "    ivec2 texSize = textureSizes[TexIndex];\n"
    "    vec2 uv = clamp(vec2(1.0 - TexCoord.x, 1.0 - TexCoord.y), 0.0, 1.0);\n" // Invert V
    "    int x = int(uv.x * float(texSize.x));\n"
    "    int y = int(uv.y * float(texSize.y));\n"
    "    int pixelIndex = int(textureOffsets[TexIndex] * 4) + (y * texSize.x + x) * 4;\n" // Calculate 1D index
    "    outAlbedo = vec4(colors[pixelIndex], colors[pixelIndex + 1], colors[pixelIndex + 2], colors[pixelIndex + 3]);\n"
    "    outNormal = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);\n"
    "}\n";
