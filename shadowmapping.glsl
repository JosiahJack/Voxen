// shadowmapping.glsl Renders depth for shadow mapping from point of view of a point light's position.
const char* depthVertexShader = "#version 450 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform int modelIndex;\n"
    "void main() {\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\n";

    const char* depthFragmentShader = "#version 450 core\n"
    "void main() {\n"
//     "    gl_FragDepth = 0.5;\n"// Depth is automatically written to depth buffer
    "}\n";
