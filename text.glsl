// Text shader
const char *textVertexShaderSource =
    "#version 450 core\n"
    "\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "uniform mat4 projection;\n"
    "out vec2 TexCoord;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = projection * vec4(aPos, 0.0, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";

const char *textFragmentShaderSource =
    "#version 450 core\n"
    "\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D textTexture;\n"
    "uniform vec4 textColor;\n"
    "\n"
    "void main() {\n"
    "    vec4 sampled = texture(textTexture, TexCoord);\n"
    "    FragColor = vec4(textColor.rgb, sampled.a * textColor.a);\n"
    "}\n";
