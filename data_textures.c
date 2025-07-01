#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION // Indicate to stb_image to compile it in.
#define STBI_NO_PSD // Excluded image formats to shrink binary size.
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include "stb_image.h"
#include <GL/glew.h>
#include <uthash.h>
#include "data_definitions.h"
#include "data_textures.h"
#include "constants.h"
#include "debug.h"
#include "render.h"

GLuint colorBufferID = 0; // Single color buffer
GLuint textureSizesID = 0;
GLuint textureOffsetsID = 0;
uint32_t * textureOffsets = NULL; // Pixel offsets
uint32_t totalPixels = 0; // Total pixels across all textures
int * textureSizes = NULL; // Needs to be textureCount * 2
int textureCount;

// Num unique colors exceeds 250,000 and is very very slow to process.

// uint32_t uniqueCols[MAX_UNIQUE_VALUE] = { [0 ... MAX_UNIQUE_VALUE - 1] = 0 };
// uint8_t uniqueReds[MAX_UNIQUE_VALUE] = { [0 ... MAX_UNIQUE_VALUE - 1] = 0 };
// uint8_t uniqueGrns[MAX_UNIQUE_VALUE] = { [0 ... MAX_UNIQUE_VALUE - 1] = 0 };
// uint8_t uniqueBlus[MAX_UNIQUE_VALUE] = { [0 ... MAX_UNIQUE_VALUE - 1] = 0 };
// int numUniqueColorValues = 0;
// int numUniqueRValues = 0;
// int numUniqueGValues = 0;
// int numUniqueBValues = 0;

// 128 * 128 = 16384
// 32 unique colors per texture = 
// 224 total 128x128 textures in project.

// int CheckIfUniqueColor(uint8_t valueR, uint8_t valueG, uint8_t valueB) { 
//     if (numUniqueColorValues >= MAX_UNIQUE_VALUE) { fprintf(stderr, "ERROR: Exceeded MAX_UNIQUE_VALUE (%d) for unique colors\n", MAX_UNIQUE_VALUE); return -1; }
//     
//     for (int i=0;i<=numUniqueColorValues;i++) {
//         if (   ((uint32_t)valueR << 16U) == (uniqueCols[i] & 0xFF0000U)
//             && ((uint32_t)valueG << 8U) == (uniqueCols[i] & 0x00FF00U)
//             && (uint32_t)valueB == (uniqueCols[i] & 0x0000FFU)) return i; // Found a match, return it.
//     }
//     
//     uniqueCols[numUniqueColorValues] = ((uint32_t)valueR << 16U) | ((uint32_t)valueG << 8U) | (uint32_t)valueB;
//     numUniqueColorValues++;
//     return numUniqueColorValues - 1; // Return index of the value.
// }
// 
// int CheckIfUniqueR(uint8_t value) {    
//     if (numUniqueRValues >= MAX_UNIQUE_VALUE) { fprintf(stderr, "ERROR: Exceeded MAX_UNIQUE_VALUE (%d) for unique reds\n", MAX_UNIQUE_VALUE); return -1; }
// 
//     for (int i=0;i<=numUniqueRValues;i++) {
//         if (value == uniqueReds[i]) return i; // Found a match, return it.
//     }
//     
//     uniqueReds[numUniqueRValues] = value;
//     numUniqueRValues++;
//     return numUniqueRValues - 1; // Return index of the value.
// }
// 
// int CheckIfUniqueG(uint8_t value) {    
//     if (numUniqueGValues >= MAX_UNIQUE_VALUE) { fprintf(stderr, "ERROR: Exceeded MAX_UNIQUE_VALUE (%d) for unique greens\n", MAX_UNIQUE_VALUE); return -1; }
// 
//     for (int i=0;i<=numUniqueGValues;i++) {
//         if (value == uniqueGrns[i]) return i; // Found a match, return it.
//     }
//     
//     uniqueGrns[numUniqueGValues] = value;
//     numUniqueGValues++;
//     return numUniqueGValues - 1; // Return index of the value.
// }
// 
// int CheckIfUniqueB(uint8_t value) {    
//     if (numUniqueBValues >= MAX_UNIQUE_VALUE) { fprintf(stderr, "ERROR: Exceeded MAX_UNIQUE_VALUE (%d) for unique blues\n", MAX_UNIQUE_VALUE); return -1; }
// 
//     for (int i=0;i<=numUniqueBValues;i++) {
//         if (value == uniqueBlus[i]) return i; // Found a match, return it.
//     }
//     
//     uniqueBlus[numUniqueBValues] = value;
//     numUniqueBValues++;
//     return numUniqueBValues - 1; // Return index of the value.
// }

const char *valid_texdata_keys[] = {"index"};

int LoadTextures(void) {
    textureCount = 0;
    
    // First parse ./Data/textures.txt to see what textures to load to what indices
    DataParser texture_parser;
    parser_init(&texture_parser, valid_texdata_keys, 1);
    if (!parse_data_file(&texture_parser, "./Data/textures.txt")) {
        printf("ERROR: Could not parse ./Data/textures.txt!\n");
        parser_free(&texture_parser);
        return 1;
    }
    
    textureCount = texture_parser.count;
    if (textureCount > 2048) { printf("ERROR: Too many textures in parser count %d, greater than 2048!\n", textureCount); parser_free(&texture_parser); return 1;
    } else printf("Parsing %d textures...\n",textureCount);
    if (textureCount == 0) {
        fprintf(stderr, "ERROR: No textures found in textures.txt\n");
        parser_free(&texture_parser);
        return 1;
    } else if (textureCount == INT32_MAX) {
        printf("WARNING: textureCount hit INT32_MAX!\n");
    }

    textureOffsets = malloc(textureCount * sizeof(uint32_t));
    textureSizes = malloc(textureCount * 2 * sizeof(int)); // Times 2 for x and y pairs flat packed (e.g. x,y,x,y,x,y for 3 textures)
    
    // Now load each one
    // Calculate total pixels and offsets
    totalPixels = 0;
    for (int i = 0; i < textureCount; i++) {
        textureOffsets[i] = totalPixels;
        int matchedParserIdx = -1;
        for (int k=0;k<texture_parser.count;k++) {
            if (texture_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }
        
        if (matchedParserIdx < 0) continue;
        
        int width, height, channels;
        unsigned char* image_data = stbi_load(texture_parser.entries[matchedParserIdx].path,&width,&height,&channels,STBI_rgb_alpha);
        if (!image_data) { fprintf(stderr, "stbi_load failed for %s: %s\n", texture_parser.entries[matchedParserIdx].path, stbi_failure_reason()); free(textureOffsets); free(textureSizes); return 1; }
        totalPixels += width * height;
        textureSizes[i * 2] = width;
        textureSizes[(i * 2) + 1] = height;
        stbi_image_free(image_data);
    }
    // Create SSBO for color buffer
    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPixels * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    
    // Load textures into color buffer
    uint32_t offset = 0;
    for (int i = 0; i < textureCount; i++) {
        int matchedParserIdx = -1;
        for (int k=0;k<texture_parser.count;k++) { // Find matching index to i that was parsed from file
            if (texture_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }
        
        if (matchedParserIdx < 0) continue;

        int width, height, channels;
        uint32_t * image_data = (uint32_t *)stbi_load(texture_parser.entries[matchedParserIdx].path,&width,&height,&channels,STBI_rgb_alpha);
        if (!image_data) { fprintf(stderr, "stbi_load failed for %s: %s\n", texture_parser.entries[matchedParserIdx].path, stbi_failure_reason()); free(textureOffsets); free(textureSizes);  return 1; }
        
        int pixelCount = width * height;
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, pixelCount * sizeof(uint32_t), image_data);
        offset += pixelCount * sizeof(uint32_t);
//             CheckIfUniqueR(image_data[j * 4 + 0]);
//             CheckIfUniqueG(image_data[j * 4 + 1]);
//             CheckIfUniqueB(image_data[j * 4 + 2]);
//             int palleteIndex = CheckIfUniqueColor(image_data[j * 4 + 0],image_data[j * 4 + 1],image_data[j * 4 + 2]);
//             if (palleteIndex < 0) { printf("ERROR, EXCEEDED PALLETE SIZE uint32 MAX!!\n"); free(textureOffsets); free(textureSizes);  return 1; }
        
        stbi_image_free(image_data);
    }
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, colorBufferID); // Set static buffer once for all shaders
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    printf("Total pixels in buffer %d (",totalPixels);
    print_bytes_no_newline(totalPixels * 4); // rgba = 4, 1 byte per channel
    printf(")\n");
//     printf("numUniqueColorValues: %d, numUniqueRValues: %d, numUniqueGValues: %d, numUniqueBValues: %d\n",numUniqueColorValues,numUniqueRValues,numUniqueGValues,numUniqueBValues);

    // 13 is BlueNoise for deferred shader
    
    // Send static uniforms to chunk shader
    glGenBuffers(1, &textureOffsetsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, textureCount * sizeof(uint32_t), textureOffsets, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, textureOffsetsID); // Set static buffer once for all shaders
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    glGenBuffers(1, &textureSizesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSizesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, textureCount * 2 * sizeof(int32_t), textureSizes, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, textureSizesID); // Set static buffer once for all shaders
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    glUniform1ui(textureCountLoc_chunk, textureCount);

    free(textureOffsets);
    free(textureSizes);
    return 0;
}
