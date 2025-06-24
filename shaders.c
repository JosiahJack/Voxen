#include <GL/glew.h>
#include "shaders.h"
#include "render.h"
#include "text.h"
#include "shadmap.glsl"
#include "text.glsl"
#include "chunk.glsl"
#include "imageblit.glsl"
#include "lights.h"
#include "deferred_lighting.compute"
#include "image_effects.h"

GLuint CompileShader(GLenum type, const char *source, const char *shaderName) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "%s Compilation Failed: %s\n", shaderName, infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint LinkProgram(GLuint *shaders, int count, const char *programName) {
    GLuint program = glCreateProgram();
    for (int i = 0; i < count; i++) {
        glAttachShader(program, shaders[i]);
    }
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "%s Linking Failed: %s\n", programName, infoLog);
        glDeleteProgram(program);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        glDeleteShader(shaders[i]);
    }
    return program;
}

int CompileShaders(void) {
    GLuint vertShader, fragShader, computeShader;
    
    // Chunk Shader
    vertShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource, "Chunk Vertex Shader");            if (!vertShader) { return 1; }
    fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderTraditional, "Chunk Fragment Shader"); if (!fragShader) { glDeleteShader(vertShader); return 1; }
    chunkShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Chunk Shader Program");    if (!chunkShaderProgram) { return 1; }

    // Text Shader
    vertShader = CompileShader(GL_VERTEX_SHADER, textVertexShaderSource, "Text Vertex Shader");       if (!vertShader) { return 1; }
    fragShader = CompileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource, "Text Fragment Shader"); if (!fragShader) { glDeleteShader(vertShader); return 1; }
    textShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Text Shader Program");    if (!textShaderProgram) { return 1; }
    
    // Deferred Lighting Compute Shader Program
    computeShader = CompileShader(GL_COMPUTE_SHADER, deferredLighting_computeShader, "Compute Shader");            if (!computeShader) { return 1; }
    deferredLightingShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Deferred Lighting Shader Program"); if (!deferredLightingShaderProgram) { return 1; }
    
    // Image Blit Shader (For full screen image effects, rendering compute results, etc.)
    vertShader = CompileShader(GL_VERTEX_SHADER,   quadVertexShaderSource,   "Image Blit Vertex Shader");     if (!vertShader) { return 1; }
    fragShader = CompileShader(GL_FRAGMENT_SHADER, quadFragmentShaderSource, "Image Blit Fragment Shader");   if (!fragShader) { glDeleteShader(vertShader); return 1; }
    imageBlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Image Blit Shader Program"); if (!imageBlitShaderProgram) { return 1; }

//     // Shadowmap Shader
//     vertShader = CompileShader(GL_VERTEX_SHADER,   shadMapVertSource, "Shadowmap Vertex Shader");            if (!vertShader) { return 1; }
//     fragShader = CompileShader(GL_FRAGMENT_SHADER, shadMapFragSource, "Shadowmap Fragment Shader");          if (!fragShader) { glDeleteShader(vertShader); return 1; }
//     shadowMapShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Shadowmap Shader Program"); if (!shadowMapShaderProgram) { return 1; }

    return 0;
}
