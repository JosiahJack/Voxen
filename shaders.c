#include <GL/glew.h>
#include "shaders.h"
#include "render.h"
#include "text.h"
#include "text.glsl"
#include "chunk.glsl"
#include "imageblit.glsl"
#include "deferred_lighting.compute"
#include "lightvolume.compute"
#include "bluenoise64.cginc"
#include "debug.h"

GLuint blueNoiseBuffer;

GLuint CompileShader(GLenum type, const char *source, const char *shaderName) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    CHECK_GL_ERROR();
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        DualLogError("%s Compilation Failed: %s\n", shaderName, infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint LinkProgram(GLuint *shaders, int count, const char *programName) {
    GLuint program = glCreateProgram();
    for (int i = 0; i < count; i++) glAttachShader(program, shaders[i]);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        DualLogError("%s Linking Failed: %s\n", programName, infoLog);
        glDeleteProgram(program);
        return 0;
    }

    for (int i = 0; i < count; i++) glDeleteShader(shaders[i]);
    return program;
}

int CompileShaders(void) {
    GLuint vertShader, fragShader, computeShader;
    
    // Chunk Shader
    vertShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource, "Chunk Vertex Shader");            if (!vertShader) { return 1; }
    fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderTraditional, "Chunk Fragment Shader"); if (!fragShader) { glDeleteShader(vertShader); return 1; }
    chunkShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Chunk Shader Program");    if (!chunkShaderProgram) { return 1; }

    // Light Volume Shader
    vertShader = CompileShader(GL_VERTEX_SHADER, lightVolumeVertexShaderSource, "Light Volume Vertex Shader"); if (!vertShader) { return 1; }
    fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderTraditional, "Chunk Fragment Shader"); if (!fragShader) { glDeleteShader(vertShader); return 1; }
    lightVolumeShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Light Volume Shader Program");    if (!lightVolumeShaderProgram) { return 1; }
    
    // Light Volume Procedural Mesh Generation Compute Shader Program
    computeShader = CompileShader(GL_COMPUTE_SHADER, createLightVolume_computeShader, "Light Volume Mesh Compute Shader"); if (!computeShader) { return 1; }
    lightVolumeMeshShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Light Volume Mesh Shader Program");        if (!lightVolumeMeshShaderProgram) { return 1; }
    
    // Text Shader
    vertShader = CompileShader(GL_VERTEX_SHADER, textVertexShaderSource, "Text Vertex Shader");       if (!vertShader) { return 1; }
    fragShader = CompileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource, "Text Fragment Shader"); if (!fragShader) { glDeleteShader(vertShader); return 1; }
    textShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Text Shader Program");    if (!textShaderProgram) { return 1; }

    // Deferred Lighting Compute Shader Program
    computeShader = CompileShader(GL_COMPUTE_SHADER, deferredLighting_computeShader, "Deferred Lighting Compute Shader"); if (!computeShader) { return 1; }
    deferredLightingShaderProgram = LinkProgram((GLuint[]){computeShader}, 1, "Deferred Lighting Shader Program");        if (!deferredLightingShaderProgram) { return 1; }
    
    // Image Blit Shader (For full screen image effects, rendering compute results, etc.)
    vertShader = CompileShader(GL_VERTEX_SHADER,   quadVertexShaderSource,   "Image Blit Vertex Shader");     if (!vertShader) { return 1; }
    fragShader = CompileShader(GL_FRAGMENT_SHADER, quadFragmentShaderSource, "Image Blit Fragment Shader");   if (!fragShader) { glDeleteShader(vertShader); return 1; }
    imageBlitShaderProgram = LinkProgram((GLuint[]){vertShader, fragShader}, 2, "Image Blit Shader Program"); if (!imageBlitShaderProgram) { return 1; }

    glGenBuffers(1, &blueNoiseBuffer);
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blueNoiseBuffer);
    CHECK_GL_ERROR();
    glBufferData(GL_SHADER_STORAGE_BUFFER, 12288 * sizeof(float), blueNoise, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, blueNoiseBuffer); // Use binding point 13
    CHECK_GL_ERROR();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    CHECK_GL_ERROR();
    
    CacheUniformLocationsForShaders(); // After shader compile!
    return 0;
}
