// render.c
// Handles setup of OpenGL rendering related objects, buffers.  Defines calls
// for all rendering tasks.
// Render order should be:
// 1. Clear
// 2. Render static geometry 
// 3. Render UI images (TODO)
// 4. Render UI text (debug only at the moment, e.g. frame stats)

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <stdio.h>
#include "constants.h"
#include "text.glsl"
#include "data_textures.h"
#include "data_models.h"
#include "event.h"
#include "quaternion.h"
#include "matrix.h"
#include "player.h"
#include "render.h"
#include "input.h"
#include "lights.h"

uint32_t drawCallCount = 0;
uint32_t vertexCount = 0;

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
    "\n"
    "in vec2 TexCoord;\n"
    "flat in int TexIndex;\n"
    "in vec3 Normal;\n"
    "in vec3 FragPos;\n"
    "\n"
    "layout(std430, binding = 0) buffer ColorBuffer {\n"
    "    float colors[];\n" // 1D color array (RGBA)
    "};\n"
    "uniform uint textureOffsets[3];\n" // Offsets for each texture
    "uniform ivec2 textureSizes[3];\n" // Width, height for each texture
    "\n"
    "layout(location = 0) out vec4 outAlbedo;\n"
    "layout(location = 1) out vec4 outNormal;\n"
    "layout(location = 2) out vec4 outWorldPos;\n"
    "\n"
    "void main() {\n"
    "    ivec2 texSize = textureSizes[TexIndex];\n"
    "    vec2 uv = clamp(vec2(1.0 - TexCoord.x, 1.0 - TexCoord.y), 0.0, 1.0);\n" // Invert V
    "    int x = int(uv.x * float(texSize.x));\n"
    "    int y = int(uv.y * float(texSize.y));\n"
    "    int pixelIndex = int(textureOffsets[TexIndex] * 4) + (y * texSize.x + x) * 4;\n" // Calculate 1D index
    "    outAlbedo = vec4(colors[pixelIndex], colors[pixelIndex + 1], colors[pixelIndex + 2], colors[pixelIndex + 3]);\n"
    "    outNormal = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);\n"
    "    outWorldPos = vec4(FragPos, 1.0);\n"
    "}\n";
    
// ----------------------------------------------------------------------------
    
// image blit
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
    "void main() {\n"
    "    FragColor = texture(tex, TexCoord);\n"
    "}\n";

// ----------------------------------------------------------------------------
    
const char *deferredLighting_computeShader =
    "#version 450 core\n"
    "\n"
    "layout(local_size_x = 8, local_size_y = 8) in;\n"
    "\n"
    "layout(rgba8, binding = 0) uniform image2D inputImage;\n"
    "layout(rgba16f, binding = 1) uniform image2D inputNormals;\n"
    "layout(r32f, binding = 2) uniform image2D inputDepth;\n"
    "layout(rgba32f, binding = 3) uniform image2D inputWorldPos;\n"
    "layout(rgba8, binding = 4) uniform image2D outputImage;\n"
    "\n"
    "uniform uint screenWidth;\n"
    "uniform uint screenHeight;\n"
    "uniform int lightDataSize;\n"
    "\n"
    "layout(std430, binding = 5) buffer LightBuffer {\n"
    "    float lights[];\n"
    "};\n"
    "\n"
    "void main() {\n"
    "    uvec2 pixel = gl_GlobalInvocationID.xy;\n"
    "    if (pixel.x >= screenWidth || pixel.y >= screenHeight) return;\n"
    "\n"
    "    // Read G-buffer data\n"
    "    vec4 color = imageLoad(inputImage, ivec2(pixel));\n"
    "    vec3 normal = normalize(imageLoad(inputNormals, ivec2(pixel)).xyz * 2.0 - 1.0);\n"
    "    float depth = imageLoad(inputDepth, ivec2(pixel)).r;\n"
    "    vec3 worldPos = imageLoad(inputWorldPos, ivec2(pixel)).xyz;\n"
    "\n"
    "    vec3 lighting = vec3(0.0);\n"
    "    int lightStride = lightDataSize > 0 && lightDataSize < 13 ? lightDataSize : 12;\n"
//     "    int lightStride = lightDataSize;\n" // This on its own locks up program
    "    for (int i = 0; i < lights.length(); i+=lightStride) {\n"
    "        float intensity =    lights[i + 3];\n"
    "        if (intensity < 0.015f) continue;\n" // LightAnimation has minIntensity of 0.01f
    "\n"
    "        float range =        lights[i + 4];\n"
    "        vec3 lightPos = vec3(lights[i + 0],\n"  // posx
    "                             lights[i + 1],\n"  // posy
    "                             lights[i + 2]);\n" // posz
    "        vec3 toLight = lightPos - worldPos;\n"
    "        float dist = length(toLight);\n"    
    "        if (dist > range) continue;\n"
    "\n"
    "        vec3 lightDir = normalize(toLight);\n"
    "\n"
    "        float spotAng =      lights[i + 5];\n"
    "        vec3 spotDir =  vec3(lights[i + 6],\n"  // spotDirx
    "                             lights[i + 7],\n"  // spotDiry
    "                             lights[i + 8]);\n" // spotDirz
    "        vec3 lightColor = vec3(lights[i + 9],\n"  // r
    "                               lights[i +10],\n"  // g
    "                               lights[i +11]);\n" // b
    "\n"
    "        float spotFalloff = 1.0;\n"
    "        if (spotAng > 0.0) {\n"
    "            float spotdot = dot(spotDir,-lightDir);\n"
    "            float cosAngle = cos(radians(spotAng / 2.0));  // Convert half-angle to radians and get cosine\n"
    "            if (spotdot < cosAngle) continue;\n"
    "\n"
    "            float cosOuterAngle = cos(radians(spotAng / 2.0));  // Outer angle in radians\n"
    "            float cosInnerAngle = cos(radians(spotAng * 0.8 / 2.0));  // Inner angle for full brightness (80% of outer angle)\n"
    "            spotFalloff = smoothstep(cosOuterAngle, cosInnerAngle, spotdot);\n"
    "            if (spotFalloff <= 0.0) continue;  // Outside the cone completely\n"
    "        }\n"
    "\n"
            // Shadows here (taken from RayTracer.shader from separate project
//             Ray ray;
//             ray.origin = pos;
//             ray.dir = normalize(lit.position - pos);
//             if (shadowsEnabled > 0 && lit.range > 1.5 && lit.intensity > 0.5) {
//                 ModelHitInfo hitInfo = CalculateRayCollision(ray); // Light to point check (aka SHADOWS!)
//                 float shaddist = length(hitInfo.hitPoint - ray.origin);
//                 if (hitInfo.didHit && shaddist < distance && shaddist > 0.001) continue;
//             }
    "\n"
    "        float attenuation = (1.0 - (dist / range)) * max(dot(normal,lightDir),0.0);\n"
    "\n"
    "        float redFinal = color.r * intensity * attenuation * lightColor.r * spotFalloff;\n"
    "        float greenFinal = color.g * intensity * attenuation * lightColor.g * spotFalloff;\n"
    "        float blueFinal = color.b * intensity * attenuation * lightColor.b * spotFalloff;\n"
    "        lighting.r += redFinal;\n"
    "        lighting.g += greenFinal;\n"
    "        lighting.b += blueFinal;\n"
    "    }\n"
    "\n"
    "    // Write to output image\n"
    "    imageStore(outputImage, ivec2(pixel), vec4(lighting, color.a));\n" // Output world pos directly to debug.
    "}\n";
    
// ----------------------------------------------------------------------------

int CompileShaders(void) {
    // ------------------------------
    // Chunk Shader
    
    // Vertex Subshader
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

    // Fragment Subshader
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
    chunkShaderProgram = glCreateProgram();
    glAttachShader(chunkShaderProgram, vertexShader);
    glAttachShader(chunkShaderProgram, fragmentShader);
    glLinkProgram(chunkShaderProgram);
    glGetProgramiv(chunkShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(chunkShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Chunk Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ------------------------------
    // Text Shader
    
    // Text Vertex Subshader
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

    // Text Fragment Subshader
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

    // ------------------------------  
    // Deferred Lighting Compute Shader Program
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &deferredLighting_computeShader, NULL);
    glCompileShader(computeShader);
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        fprintf(stderr, "Compute Shader Compilation Failed: %s\n", infoLog);
        return 1;
    }

    deferredLightingShaderProgram = glCreateProgram();
    glAttachShader(deferredLightingShaderProgram, computeShader);
    glLinkProgram(deferredLightingShaderProgram);
    glGetProgramiv(deferredLightingShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(deferredLightingShaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Compute Shader Program Linking Failed: %s\n", infoLog);
        return 1;
    }

    glDeleteShader(computeShader);
    
    // ------------------------------  
    // Image Blit Shader (For full screen image effects, rendering compute results, etc.)
    
    // Full Screen Quad Vertex Subshader
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

    // Full Screen Quad Fragment Subshader
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

    // ------------------------------  

    return 0;
}

GLuint chunkShaderProgram;
GLuint inputImageID, inputNormalsID, inputDepthID, inputWorldPosID, outputImageID, gBufferFBO;

void SetupGBuffer(void) {
    printf("SetupGBuffer \n");
    // Create G-buffer textures
    glGenTextures(1, &inputImageID);
    glBindTexture(GL_TEXTURE_2D, inputImageID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &inputNormalsID);
    glBindTexture(GL_TEXTURE_2D, inputNormalsID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &inputDepthID);
    glBindTexture(GL_TEXTURE_2D, inputDepthID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // GL_RGBA32F for higher precision worldPos for lighting.
    glGenTextures(1, &inputWorldPosID);
    glBindTexture(GL_TEXTURE_2D, inputWorldPosID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &outputImageID);
    glBindTexture(GL_TEXTURE_2D, outputImageID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create framebuffer
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputImageID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, inputNormalsID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, inputWorldPosID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, inputDepthID, 0);

    // Specify draw buffers
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0,
                             GL_COLOR_ATTACHMENT1,
                             GL_COLOR_ATTACHMENT2 };
                             
    glDrawBuffers(3, drawBuffers);

    // Check framebuffer status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                fprintf(stderr, "Framebuffer incomplete: Attachment issue\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                fprintf(stderr, "Framebuffer incomplete: Missing attachment\n");
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                fprintf(stderr, "Framebuffer incomplete: Unsupported configuration\n");
                break;
            default:
                fprintf(stderr, "Framebuffer incomplete: Error code %d\n", status);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Image Effect Blit quad
GLuint imageBlitShaderProgram;
GLuint quadVAO, quadVBO;
void SetupQuad(void) {
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
        -1.0f,  1.0f, 0.0f, 1.0f, // Top-left
         1.0f,  1.0f, 1.0f, 1.0f, // Top-right
         1.0f, -1.0f, 1.0f, 0.0f  // Bottom-right
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GLuint deferredLightingShaderProgram;
float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
GLuint lightBufferID;

// Initialize lights with random positions
void InitializeLights(void) {
    srand(time(NULL)); // Seed random number generator
    for (int i = 0; i < LIGHT_COUNT; i++) {
        int base = i * LIGHT_DATA_SIZE; // Step by 12
        lights[base + 0] = base * 0.08f; // posx
        lights[base + 1] = base * 0.08f; // posy
        lights[base + 2] = 0.0f; // posz
        lights[base + 3] = base > 12 * 256 ? 0.0f : 2.0f; // intensity
        lights[base + 4] = 5.24f; // radius
        lights[base + 5] = 0.0f; // spotAng
        lights[base + 6] = 0.0f; // spotDirx
        lights[base + 7] = 0.0f; // spotDiry
        lights[base + 8] = -1.0f; // spotDirz
        lights[base + 9] = 1.0f; // r
        lights[base + 10] = 1.0f; // g
        lights[base + 11] = 1.0f; // b
    }
    
    lights[0] = 10.24f;
    lights[1] = 0.0f;
    lights[2] = 0.0f; // Fixed Z height
    lights[3] = 2.0f; // Default intensity
    lights[4] = 10.0f; // Default radius
    lights[6] = 0.0f;
    lights[7] = 0.0f;
    lights[8] = -1.0f;
    lights[9] = 1.0f;
    lights[10] = 1.0f;
    lights[11] = 1.0f;
    
    lights[0 + 12] = 10.24f;
    lights[1 + 12] = 0.0f;
    lights[2 + 12] = 0.0f; // Fixed Z height
    lights[3 + 12] = 2.0f; // Default intensity
    lights[4 + 12] = 10.0f; // Default radius
    lights[6 + 12] = 0.0f;
    lights[7 + 12] = 0.0f;
    lights[8 + 12] = -1.0f;
    lights[9 + 12] = 1.0f;
    lights[10 + 12] = 0.0f;
    lights[11 + 12] = 0.0f;

    // Create and bind SSBO
    glGenBuffers(1, &lightBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

GLuint textShaderProgram;
TTF_Font* font = NULL;
GLuint textVAO, textVBO;

// Quad for text (2 triangles, positions and tex coords)
float textQuadVertices[] = {
    // Positions   // Tex Coords
    0.0f, 0.0f,    0.0f, 0.0f, // Bottom-left
    1.0f, 0.0f,    1.0f, 0.0f, // Bottom-right
    1.0f, 1.0f,    1.0f, 1.0f, // Top-right
    0.0f, 1.0f,    0.0f, 1.0f  // Top-left
};

void SetupTextQuad(void) {
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textQuadVertices), textQuadVertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Tex Coord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

int ClearFrameBuffers(void) {
    // Clear the G-buffer FBO
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clear color (black, fully opaque)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Clear the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Ensure consistent clear color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return 0;
}

// Global uniform locations (cached during init)
GLint viewLoc = -1, projectionLoc = -1, modelLoc = -1, texIndexLoc = -1;
GLint textureOffsetsLoc = -1, textureSizesLoc = -1;

void CacheUniformLocationsForChunkShader(void) {
    // Called after shader compilation in InitializeEnvironment
    viewLoc = glGetUniformLocation(chunkShaderProgram, "view");
    projectionLoc = glGetUniformLocation(chunkShaderProgram, "projection");
    modelLoc = glGetUniformLocation(chunkShaderProgram, "model");
    texIndexLoc = glGetUniformLocation(chunkShaderProgram, "texIndex");
    textureOffsetsLoc = glGetUniformLocation(chunkShaderProgram, "textureOffsets");
    textureSizesLoc = glGetUniformLocation(chunkShaderProgram, "textureSizes");
}

int RenderStaticMeshes(void) {
    // Attempt to update the test light to be "attached" to the player
    lights[0] = testLight_x;
    lights[1] = testLight_y;
    lights[2] = testLight_z;
    lights[3] = testLight_intensity;
    lights[4] = testLight_range;
    lights[5] = testLight_spotAng;
    
    drawCallCount = 0; // Reset per frame
    vertexCount = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glUseProgram(chunkShaderProgram);

    // Set up view and projection matrices
    float view[16], projection[16];
    float fov = 65.0f;
    mat4_perspective(projection, fov, (float)screen_width / screen_height, 0.02f, 1300.0f);
    mat4_lookat(view, cam_x, cam_y, cam_z, &cam_rotation);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, colorBufferID);
    glUniform1uiv(textureOffsetsLoc, TEXTURE_COUNT, textureOffsets);
    glUniform2iv(textureSizesLoc, TEXTURE_COUNT, textureSizes);
    glBindVertexArray(vao);

    // Render each model, simple draw calls, no instancing yet
    float model[16];
    int drawCallLimit = 50;
    int currentModelType = 1;
    int loopIter = 0;
    for (int yarray=0;yarray<drawCallLimit;yarray++) {
        for (int xarray=0;xarray<drawCallLimit;xarray++) {
            mat4_identity(model);
            mat4_translate(model,(float)xarray * 2.56f, (float)yarray * 2.56f, 0.0f);
            glUniform1i(texIndexLoc, currentModelType);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);
            glBindVertexBuffer(0, vbos[currentModelType], 0, VERTEX_ATTRIBUTES_COUNT * sizeof(float));
            glDrawArrays(GL_TRIANGLES, 0, modelVertexCounts[currentModelType]);
            drawCallCount++;
            vertexCount += modelVertexCounts[0];
            loopIter++;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Apply deferred lighting with compute shader
    glUseProgram(deferredLightingShaderProgram);
    GLint screenWidthLoc = glGetUniformLocation(deferredLightingShaderProgram, "screenWidth");
    GLint screenHeightLoc = glGetUniformLocation(deferredLightingShaderProgram, "screenHeight");
    GLint lightDataSizeLoc = glGetUniformLocation(deferredLightingShaderProgram, "lightDataSize");
    glUniform1ui(screenWidthLoc, screen_width);
    glUniform1ui(screenHeightLoc, screen_height);
    glUniform1ui(lightDataSizeLoc, LIGHT_DATA_SIZE);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, LIGHT_COUNT * LIGHT_DATA_SIZE * sizeof(float), lights, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindImageTexture(0, inputImageID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(1, inputNormalsID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(2, inputDepthID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_DEPTH_COMPONENT32F);
    glBindImageTexture(3, inputWorldPosID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(4, outputImageID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    // Dispatch compute shader
    GLuint groupX = (screen_width + 7) / 8;
    GLuint groupY = (screen_height + 7) / 8;
    glDispatchCompute(groupX, groupY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    // Render final gather lighting
    glUseProgram(imageBlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outputImageID);
    glUniform1i(glGetUniformLocation(imageBlitShaderProgram, "tex"), 0);
    glBindVertexArray(quadVAO);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    return 0;
}

// Renders text at x,y coordinates specified using pointer to the string array.
void render_debug_text(float x, float y, const char *text, SDL_Color color) {
    if (!font || !text) { fprintf(stderr, "Font or text is NULL\n"); return; }
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) { fprintf(stderr, "TTF_RenderText_Solid failed: %s\n", TTF_GetError()); return; }
    SDL_Surface *rgba_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba_surface) { fprintf(stderr, "SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError()); return; }

    // Create and bind texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_surface->w, rgba_surface->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Use text shader
    glUseProgram(textShaderProgram);

    // Set up orthographic projection
    float projection[16];
    mat4_ortho(projection, 0.0f, (float)screen_width, (float)screen_height, 0.0f, -1.0f, 1.0f);
    GLint projLoc = glGetUniformLocation(textShaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);

    // Set text color (convert SDL_Color to 0-1 range)
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    float a = color.a / 255.0f;
    GLint colorLoc = glGetUniformLocation(textShaderProgram, "textColor");
    glUniform4f(colorLoc, r, g, b, a);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLint texLoc = glGetUniformLocation(textShaderProgram, "textTexture");
    glUniform1i(texLoc, 0);

    // Enable blending for text transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // Disable depth test for 2D overlay

    // Bind VAO and adjust quad position/size
    glBindVertexArray(textVAO);
    float scaleX = (float)rgba_surface->w;
    float scaleY = (float)rgba_surface->h;
    float vertices[] = {
        x,          y,          0.0f, 0.0f, // Bottom-left
        x + scaleX, y,          1.0f, 0.0f, // Bottom-right
        x + scaleX, y + scaleY, 1.0f, 1.0f, // Top-right
        x,          y + scaleY, 0.0f, 1.0f  // Top-left
    };
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Render quad (two triangles)
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    drawCallCount++;
    vertexCount+=4*128;

    // Cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); // Re-enable depth test for 3D rendering
    glUseProgram(0);
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(rgba_surface);
}

double lastFrameSecCountTime = 0.00;
uint32_t lastFrameSecCount = 0;
uint32_t framesPerLastSecond = 0;

int RenderUI(double deltaTime) {
    glDisable(GL_LIGHTING); // Disable lighting for text
    SDL_Color textCol = {255, 255, 255, 255}; // White

    // Draw debug text
    char text1[128];
    snprintf(text1, sizeof(text1), "x: %.2f, y: %.2f, z: %.2f", cam_x, cam_y, cam_z);
    render_debug_text(10, 25, text1, textCol); // Top-left corner (10, 10)

    float cam_quat_yaw = 0.0f;
    float cam_quat_pitch = 0.0f;
    float cam_quat_roll = 0.0f;
    quat_to_euler(&cam_rotation,&cam_quat_yaw,&cam_quat_pitch,&cam_quat_roll);
    char text2[128];
    snprintf(text2, sizeof(text2), "cam yaw: %.2f, cam pitch: %.2f, cam roll: %.2f", cam_yaw, cam_pitch, cam_roll);
    render_debug_text(10, 40, text2, textCol);

    char text3[128];
    snprintf(text3, sizeof(text3), "cam quat yaw: %.2f, cam quat pitch: %.2f, cam quat roll: %.2f", cam_quat_yaw, cam_quat_pitch, cam_quat_roll);
    render_debug_text(10, 55, text3, textCol);

    char text4[128];
    snprintf(text4, sizeof(text4), "Peak frame queue count: %.2d", maxEventCount_debug);
    render_debug_text(10, 70, text4, textCol);
    
    char text5[128];
    snprintf(text5, sizeof(text5), "testLight_spotAng: %.4f", testLight_spotAng);
    render_debug_text(10, 95, text5, textCol);
    
    // Frame stats
    drawCallCount++; // Add one more for this text render ;)
    char text0[256];
    snprintf(text0, sizeof(text0), "Frame time: %.6f (FPS: %d), Draw calls: %d, Vertices: %d", deltaTime * 1000.0,framesPerLastSecond,drawCallCount,vertexCount);
    render_debug_text(10, 10, text0, textCol); // Top-left corner (10, 10)
    double time_now = get_time();
    if ((time_now - lastFrameSecCountTime) >= 1.00) {
        lastFrameSecCountTime = time_now;
        framesPerLastSecond = globalFrameNum - lastFrameSecCount;
        lastFrameSecCount = globalFrameNum;
    }
    return 0;
}

// Main render call for entire graphics pipeline.
int ClientRender() {
    int exitCode = 0;
    exitCode = ClearFrameBuffers();
    if (exitCode) return exitCode;
    
    exitCode = RenderStaticMeshes(); // FIRST FOR RESETTING DRAW CALL COUNTER!
    if (exitCode) return exitCode;

    exitCode = RenderUI(get_time() - last_time);
    return exitCode;
}
