const char *vertexShaderSource =
    "#version 450 core\n"
    "\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "layout(location = 2) in vec2 aTexCoord;\n"
    "layout(location = 3) in float aTexIndex;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec2 TexCoord;\n"
    "out float TexIndex;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = projection * view * vec4(aPos, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "    TexIndex = aTexIndex;\n"
    "}\n";

const char *fragmentShaderBindless =
    "#version 450 core\n"
    "#extension GL_ARB_bindless_texture : require\n"
    "\n"
    "in vec2 TexCoord;\n"
    "in float TexIndex;\n"
    "out vec4 FragColor;\n"
    "layout(bindless_sampler) uniform sampler2D uTextures[3];\n"
    "\n"
    "void main() {\n"
    "    int index = int(TexIndex);\n"
    "    FragColor = texture(uTextures[index], TexCoord);\n"
    "}\n";

const char *fragmentShaderTraditional =
    "#version 450 core\n"
    "\n"
    "in vec2 TexCoord;\n"
    "in float TexIndex;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D uTextures[3];\n"
    "\n"
    "void main() {\n"
    "    int index = int(TexIndex);\n"
    "    FragColor = texture(uTextures[index], TexCoord);\n"
    "}\n";


// Vertex Shader for Text
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

// Fragment Shader for Text
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
