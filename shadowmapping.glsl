// shadowmapping.glsl Renders depth for shadow mapping from point of view of a point light's position.
const char* depthVertexShader =
    "#version 450 core\n"

    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "layout(location = 2) in vec2 aTexCoord;\n"

    "uniform mat4 matrix;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"

    "out vec3 FragPos;\n"

    "void main() {\n"
    "    FragPos = vec3(matrix * vec4(aPos, 1.0));\n" // Convert vertex from the model's local space into world space
    "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\n";

const char* depthFragmentShader =
    "#version 450 core\n"

    "uniform int debugView;\n"
    "uniform int shadowFace;\n"

    "layout(location = 0) out vec4 outXPos;\n" // GL_COLOR_ATTACHMENT0
    "layout(location = 1) out vec4 outXNeg;\n" // GL_COLOR_ATTACHMENT1
    "layout(location = 2) out vec4 outYPos;\n" // GL_COLOR_ATTACHMENT2
    "layout(location = 3) out ivec4 outYNeg;\n" // GL_COLOR_ATTACHMENT3
    "layout(location = 4) out ivec4 outZPos;\n" // GL_COLOR_ATTACHMENT4
    "layout(location = 5) out ivec4 outZNeg;\n" // GL_COLOR_ATTACHMENT5

    "vec4 pack_depth(float depth) {\n"
    "    vec4 bit_shift = vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);\n"
    "    vec4 bit_mask = vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);\n"
    "    vec4 res = fract(depth * bit_shift);\n"
    "    res -= res.xxyz * bit_mask;\n"
    "    return res;\n"
    "}\n"

    "void main() {\n"
    "    float ndcDepth = (2.0 * gl_FragCoord.z - 1.0);\n" // Depth debug
    "    if (shadowFace == 0) {\n"
    "        outXPos = pack_depth(ndcDepth);\n"
    "    } else if (shadowFace == 1) {\n"
    "        outXNeg = pack_depth(ndcDepth);\n"
    "    } else if (shadowFace == 2) {\n"
    "        outYPos = pack_depth(ndcDepth);\n"
    "    } else if (shadowFace == 3) {\n"
    "        outYNeg = pack_depth(ndcDepth);\n"
    "    } else if (shadowFace == 4) {\n"
    "        outZPos = pack_depth(ndcDepth);\n"
    "    } else if (shadowFace == 5) {\n"
    "        outZNeg = pack_depth(ndcDepth);\n"
    "    }\n"
    "}\n";
