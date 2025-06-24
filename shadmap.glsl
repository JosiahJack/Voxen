// Shadow Mapping shader
const char *shadMapVertSource =
    "#version 450 core\n"
    "\n"
    "layout(location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec3 FragPos;\n"
    "\n"
    "void main() {\n"
    "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
    "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\n";

const char * shadMapFragSource =
    "#version 450 core\n"
    "\n"
    "in vec3 FragPos;\n"
    "\n"
    "layout(location = 0) out vec4 outAlbedo;\n"
    "\n"
    "void main() {\n"
    "    float ndcDepth = (2.0 * gl_FragCoord.z - 1.0);\n"
    "    float clipDepth = ndcDepth / gl_FragCoord.w;\n"
//     "    float linearDepth = (clipDepth - 0.02) / (100.0 - 0.02);\n"
    "    outAlbedo = vec4(vec3(clipDepth), 1.0);\n"  // Depth debug output linearDepth
    "}\n";
