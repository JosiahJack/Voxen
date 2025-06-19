#include <GL/glew.h>
#include <stdio.h>
#include "shaders.glsl.h"
#include "cull.compute"
// #include "transformation.compute TODO
#include "rasterize.compute"
#include "text.glsl"
#include "imageblit.glsl"

int CompileShaders(void) {
    // Cull Compute Shader
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &cull_computeShader, NULL);
    glCompileShader(computeShader);
    GLint success;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        fprintf(stderr, "Compute Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    cullShaderProgram = glCreateProgram();
    glAttachShader(cullShaderProgram, computeShader);
    glLinkProgram(cullShaderProgram);
    glGetProgramiv(cullShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(cullShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Compute Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }
    
    glDeleteShader(computeShader);
    
    // ------------------------------------------------------------------------
    
    // Rasterize Compute Shader
    GLuint rasterizeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(rasterizeShader, 1, &rasterize_computeShader, NULL);
    glCompileShader(rasterizeShader);
    glGetShaderiv(rasterizeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(rasterizeShader, 512, NULL, infoLog);
        fprintf(stderr, "Rasterize Compute Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }
    
    rasterizeShaderProgram = glCreateProgram();
    glAttachShader(rasterizeShaderProgram, rasterizeShader);
    glLinkProgram(rasterizeShaderProgram);
    glGetProgramiv(rasterizeShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(rasterizeShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Rasterize Compute Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }
    
    glDeleteShader(rasterizeShader);
    
    // ------------------------------------------------------------------------
    
    // Transform Compute Shader - Transforms from cartesian world coordinates into screen space (polar coordinates)
    GLuint transformShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(transformShader, 1, &rasterize_computeShader, NULL);
    glCompileShader(transformShader);
    glGetShaderiv(transformShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(transformShader, 512, NULL, infoLog);
        fprintf(stderr, "Transform Compute Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }
    
    transformShaderProgram = glCreateProgram();
    glAttachShader(transformShaderProgram, transformShader);
    glLinkProgram(transformShaderProgram);
    glGetProgramiv(transformShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(transformShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Transform Compute Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }
    
    glDeleteShader(transformShader);
    
    // ------------------------------------------------------------------------
   
   
    // Image Blit Shader (For full screen image effects, rendering compute results, etc.)
    GLuint quadVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(quadVertexShader, 1, &quadVertexShaderSource, NULL);
    glCompileShader(quadVertexShader);
    glGetShaderiv(quadVertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(quadVertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Image Blit Vertex Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }
    GLuint quadFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(quadFragmentShader, 1, &quadFragmentShaderSource, NULL);
    glCompileShader(quadFragmentShader);
    glGetShaderiv(quadFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(quadFragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "Image Blit Fragment Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }
    imageBlitShaderProgram = glCreateProgram();
    glAttachShader(imageBlitShaderProgram, quadVertexShader);
    glAttachShader(imageBlitShaderProgram, quadFragmentShader);
    glLinkProgram(imageBlitShaderProgram);
    glGetProgramiv(imageBlitShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(imageBlitShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Image Blit Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }
    glDeleteShader(quadVertexShader);
    glDeleteShader(quadFragmentShader);
    
    // ------------------------------------------------------------------------
    
    // Text Shader
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
