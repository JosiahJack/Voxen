#include <GL/glew.h>
#include <stdio.h>
#include "shaders.glsl.h"

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
    "in vec2 TexCoord;\n"
    "flat in int TexIndex;\n"
    "out vec4 FragColor;\n"
    "layout(std430, binding = 0) buffer ColorBuffer {\n"
    "    float colors[];\n" // 1D color array (RGBA)
    "};\n"
    "uniform uint textureOffsets[3];\n" // Offsets for each texture
    "uniform ivec2 textureSizes[3];\n" // Width, height for each texture
    "void main() {\n"
    "    ivec2 texSize = textureSizes[TexIndex];\n"
    "    vec2 uv = clamp(vec2(1.0 - TexCoord.x, 1.0 - TexCoord.y), 0.0, 1.0);\n" // Invert V
    "    int x = int(uv.x * float(texSize.x));\n"
    "    int y = int(uv.y * float(texSize.y));\n"
    "    int pixelIndex = int(textureOffsets[TexIndex] * 4) + (y * texSize.x + x) * 4;\n" // Calculate 1D index
    "    FragColor = vec4(colors[pixelIndex], colors[pixelIndex + 1], colors[pixelIndex + 2], colors[pixelIndex + 3]);\n"
    "}\n";
    
// ----------------------------------------------------------------------------

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
    
// ----------------------------------------------------------------------------

int CompileShaders(void) {
    // Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Vertex Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragSource = fragmentShaderTraditional;
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Fragment Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Shader Program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Text Vertex Shader
    GLuint textVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(textVertexShader, 1, &textVertexShaderSource, NULL);
    glCompileShader(textVertexShader);
    glGetShaderiv(textVertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(textVertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Text Vertex Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Text Fragment Shader
    GLuint textFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(textFragmentShader, 1, &textFragmentShaderSource, NULL);
    glCompileShader(textFragmentShader);
    glGetShaderiv(textFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(textFragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Text Fragment Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    // Text Shader Program
    textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, textVertexShader);
    glAttachShader(textShaderProgram, textFragmentShader);
    glLinkProgram(textShaderProgram);
    glGetProgramiv(textShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(textShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Text Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }

    glDeleteShader(textVertexShader);
    glDeleteShader(textFragmentShader);
    return 0;
}
