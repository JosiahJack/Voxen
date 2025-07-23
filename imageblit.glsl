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
    "uniform int debugView;\n"
    "uniform int debugValue;\n"

    "void main() {\n"
    "    FragColor = texture(tex, TexCoord);\n"
    "}\n";
