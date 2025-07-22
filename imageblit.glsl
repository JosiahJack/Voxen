// imageblit.glsl
// Full screen quad unlit textured for presenting image buffers such as results
// from compute shaders, image effects, post-processing, etc..
const char *quadVertexShaderSource =
    "#version 450 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";

const char *quadFragmentShaderSource =
    "#version 450 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D tex;\n"
    "uniform samplerCube shadowMap;\n" // Add this
    "uniform int debugView;\n"
    "uniform int debugValue;\n"

    "void main() {\n"
    "    if (debugView == 5) {\n" // Shadowmap debug
    "        vec3 directions[6] = vec3[6](\n"
    "            vec3(1.0, TexCoord.y * 2.0 - 1.0, -(TexCoord.x * 2.0 - 1.0)),\n"  // +X
    "            vec3(-1.0, TexCoord.y * 2.0 - 1.0, TexCoord.x * 2.0 - 1.0),\n"  // -X
    "            vec3(TexCoord.x * 2.0 - 1.0, 1.0, -(TexCoord.y * 2.0 - 1.0)),\n" // +Y
    "            vec3(TexCoord.x * 2.0 - 1.0, -1.0, TexCoord.y * 2.0 - 1.0),\n"   // -Y
    "            vec3(TexCoord.x * 2.0 - 1.0, TexCoord.y * 2.0 - 1.0, 1.0),\n"    // +Z
    "            vec3(-(TexCoord.x * 2.0 - 1.0), TexCoord.y * 2.0 - 1.0, -1.0)\n" // -Z
    "        );\n"
    "        int face = clamp(debugValue, 0, 5);\n"
    "        float depth = texture(shadowMap, directions[face]).r;\n"
    "        depth = (depth - 0.02) / (15.36 - 0.02);\n"
    "        FragColor = vec4(vec3(depth), 1.0);\n"
    "    } else {\n"
    "        FragColor = texture(tex, TexCoord);\n"
    "    }\n"
    "}\n";
