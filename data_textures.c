#include <malloc.h>
#include <stdint.h>
#include <stdbool.h>
#define STB_IMAGE_IMPLEMENTATION // Indicate to stb_image to compile it in.
#define STBI_ONLY_PNG
#define STBI_MAX_DIMENSIONS 4096
#include "External/stb_image.h"
#include <GL/glew.h>
#include <sys/stat.h> // For stat
#include <errno.h>
#include <uthash.h>
#include "data_parser.h"
#include "debug.h"
#include "render.h"
#include "event.h"

// #define DEBUG_TEXTURE_LOAD_DATA 1
#define MAX_PALETTE_SIZE 65535u

DataParser texture_parser;
GLuint colorBufferID = 0; // Single color buffer
GLuint textureSizesID = 0;
GLuint textureOffsetsID = 0;
GLuint texturePalettesID = 0; // New SSBO for palette colors
GLuint texturePaletteOffsetsID = 0; // New SSBO for palette offsets
uint32_t* textureOffsets = NULL; // Pixel offsets
uint32_t* texturePaletteOffsets = NULL; // Offsets into texturePalettes
uint32_t* texturePalettes = NULL; // Concatenated palette colors
uint32_t totalPixels = 0; // Total pixels across all textures
uint32_t totalPaletteColors = 0; // Total colors across all palettes
int* textureSizes = NULL; // Needs to be textureCount * 2
uint16_t textureCount;
bool* doubleSidedTexture = NULL;
bool* transparentTexture = NULL;

bool isDoubleSided(uint32_t texIndexToCheck) { return (doubleSidedTexture[texIndexToCheck]); }
bool isTransparent(uint32_t texIndexToCheck) { return (transparentTexture[texIndexToCheck]); }

typedef struct {
    uint32_t color;    // RGBA color (packed)
    uint16_t index;    // Palette index
    UT_hash_handle hh;
} ColorEntry;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
int LoadTextures(void) {
    double start_time = get_time();
    DebugRAM("start of LoadTextures");
    textureCount = 0u;
    
    // First parse ./Data/textures.txt to see what textures to load to what indices
    parser_init(&texture_parser);
    if (!parse_data_file(&texture_parser, "./Data/textures.txt",0)) { DualLogError("Could not parse ./Data/textures.txt!\n"); return 1; }
    
    int maxIndex = -1;
    for (int k=0;k<texture_parser.count;k++) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != UINT16_MAX) { maxIndex = texture_parser.entries[k].index; }
    }
    
    textureCount = (uint16_t)texture_parser.count;
    if (textureCount > 4096) { DualLogError("Too many textures in parser count %d, greater than 4096!\n", textureCount); return 1; } 
    if (textureCount == 0) { DualLogError("No textures found in textures.txt\n"); return 1; }
    
    DualLog("Loading %d textures with max index %d, using stb_image version: 2.28...                         ",textureCount,maxIndex);
    textureOffsets = malloc(textureCount * sizeof(uint32_t));
    textureSizes = malloc(textureCount * 2 * sizeof(int)); // Times 2 for x and y pairs flat packed (e.g. x,y,x,y,x,y for 3 textures)
    texturePaletteOffsets = malloc(textureCount * sizeof(uint32_t));
    doubleSidedTexture = malloc(textureCount * sizeof(bool));
    transparentTexture = malloc(textureCount * sizeof(bool));
    size_t maxFileSize = 3000000; // 3MB
    uint8_t * file_buffer = malloc(maxFileSize); // Reused buffer for loading .png files.  64MB for 4096 * 4096 image.    

    // First Pass: Calculate total pixels and offsets
    totalPixels = 0;
    totalPaletteColors = 0;
    uint32_t totalPaletteColorsExtraSized = 170172; // Actual for Citadel is 170172
    struct stat file_stat;
    uint32_t pixel_offset = 0;
    uint32_t palette_offset = 0;
    uint32_t maxPalletSize = 0;
    uint16_t *indices = malloc(4096 * 4096 * sizeof(uint16_t));
    GLuint stagingBuffer;
    glGenBuffers(1, &stagingBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (((4096 * 4096) + 1) / 2) * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY); // Max texture size
    texturePalettes = malloc(totalPaletteColorsExtraSized * sizeof(uint32_t));

    // Create SSBO for texture palettes
    glGenBuffers(1, &texturePalettesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePalettesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPaletteColorsExtraSized * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    
    // Create SSBO for color buffer
    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBufferID);
    int colorBufferSize = ((54845087 + 1) / 2) * sizeof(uint32_t);
    glBufferData(GL_SHADER_STORAGE_BUFFER, colorBufferSize, NULL, GL_STATIC_DRAW);
    
    ColorEntry *color_pool = malloc(textureCount * MAX_PALETTE_SIZE * sizeof(ColorEntry));
    uint32_t *pool_indices = malloc(textureCount * sizeof(uint32_t));
    memset(pool_indices, 0, textureCount * sizeof(uint32_t));
    
    int matched_indices[textureCount];
    for (int i = 0; i < textureCount; i++) matched_indices[i] = -1;
    for (int k = 0; k < texture_parser.count; k++) {
        if (texture_parser.entries[k].index < textureCount) {
            matched_indices[texture_parser.entries[k].index] = k;
        }
    }
    
    for (int i = 0; i < textureCount; i++) {
//         if (i % 50 == 0 || i == textureCount - 1) RenderLoadingProgress(105,"Loading textures [%d of %d]...",i,textureCount);
        textureOffsets[i] = totalPixels;
        texturePaletteOffsets[i] = totalPaletteColors;
        int matchedParserIdx = matched_indices[i];
        if (matchedParserIdx < 0) continue;
        if (stat(texture_parser.entries[matchedParserIdx].path, &file_stat) != 0) { DualLogError("Failed to stat %s: %s\n", texture_parser.entries[matchedParserIdx].path, strerror(errno)); return 1; }

        size_t file_size = file_stat.st_size;
        if (file_size > maxFileSize) { DualLogError("\nPNG file %s too large (%zu bytes)\n", texture_parser.entries[matchedParserIdx].path, file_size); return 1; }
        
        FILE* fp = fopen(texture_parser.entries[matchedParserIdx].path, "rb");
        fread(file_buffer, 1, file_size, fp);
        fclose(fp);
        int width, height, channels;
        unsigned char* image_data = stbi_load_from_memory(file_buffer, file_size, &width, &height, &channels, STBI_rgb_alpha);
        if (!image_data) { DualLogError("\nstbi_load failed for %s: %s\n", texture_parser.entries[matchedParserIdx].path, stbi_failure_reason()); return 1; }
        
        doubleSidedTexture[i] = texture_parser.entries[matchedParserIdx].doublesided;
        
        // Build palette for this texture using uthash
        ColorEntry *color_table = NULL, *entry;
        uint32_t palette_size = 0;
        uint32_t pool_start = i * MAX_PALETTE_SIZE;
        for (int j = 0; j < width * height * 4; j += 4) {
            uint32_t color = ((uint32_t)image_data[j] << 24) | ((uint32_t)image_data[j + 1] << 16) |
                             ((uint32_t)image_data[j + 2] << 8) | (uint32_t)image_data[j + 3];

            if (image_data[j + 3] < 255) transparentTexture[i] = true; // Don't remove if, need to preserve in case any pixels are not transparent
            HASH_FIND_INT(color_table, &color, entry);
            if (!entry) {
                if (palette_size >= MAX_PALETTE_SIZE) { DualLogError("Palette size exceeded for %s\n", texture_parser.entries[matchedParserIdx].path); palette_size = MAX_PALETTE_SIZE - 1; return 1; }

                entry = &color_pool[pool_start + pool_indices[i]++];
                entry->color = color;
                entry->index = (uint16_t)palette_size++;
                HASH_ADD_INT(color_table, color, entry);
                texturePalettes[palette_offset + entry->index] = color;
            }
            
            indices[j / 4] = entry->index;
        }
        
        // Upload indices to colorBuffer via temporary staging buffer.
        // The staging buffer (reused for all textures) is deleted afterwards
        // letting the OpenGL driver delete the copy in RAM for CPU side and
        // just let VRAM alone store the texture data.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
        uint32_t *mapped_buffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,((width * height + 1) / 2) * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        for (int j = 0; j < width * height; j += 2) {
            uint32_t packed = (uint32_t)indices[j]; // Lower 16 bits
            if (j + 1 < width * height) packed |= (uint32_t)indices[j + 1] << 16; // Upper 16 bits
            mapped_buffer[j / 2] = packed;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_COPY_READ_BUFFER, stagingBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, colorBufferID);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, ((pixel_offset + 1) / 2) * sizeof(uint32_t), ((width * height + 1) / 2) * sizeof(uint32_t));
        pixel_offset += width * height;
        palette_offset += palette_size;
#ifdef DEBUG_TEXTURE_LOAD_DATA
        if (palette_size > 1000U) DualLog("\nLoaded %s with large palette size of \033[33m%d!\033[0m\n", texture_parser.entries[matchedParserIdx].path, palette_size);
#endif
        if (palette_size > maxPalletSize) maxPalletSize = palette_size; // Keep track of which had the largest.

        totalPaletteColors += palette_size;        
        totalPixels += width * height;
        textureSizes[i * 2] = width;
        textureSizes[(i * 2) + 1] = height;
        stbi_image_free(image_data);
    }
    
    glDeleteBuffers(1, &stagingBuffer);
    free(indices);
    free(file_buffer);
    free(color_pool);
    free(pool_indices);

#ifdef DEBUG_TEXTURE_LOAD_DATA
    DualLog("\nLargest palette size of %d\n", maxPalletSize);
#endif
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, colorBufferID); // Set static buffer once for all shaders

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePalettesID);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, totalPaletteColors * sizeof(uint32_t), texturePalettes);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, texturePalettesID);
    free(texturePalettes);
    
#ifdef DEBUG_TEXTURE_LOAD_DATA
    DualLog("\nTotal pixels in buffer %d (", totalPixels);
    print_bytes_no_newline(colorBufferSize);
    DualLog("), total palette colors %d (", totalPaletteColors);
    print_bytes_no_newline(totalPaletteColors * 4);
    DualLog(")\n");
#endif
    
    // Send static uniforms to chunk shader
    glGenBuffers(1, &textureOffsetsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, textureCount * sizeof(uint32_t), textureOffsets, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, textureOffsetsID); // Set static buffer once for all shaders
    free(textureOffsets);
    
    glGenBuffers(1, &textureSizesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSizesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, textureCount * 2 * sizeof(int32_t), textureSizes, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, textureSizesID); // Set static buffer once for all shaders
    free(textureSizes);
    
    glGenBuffers(1, &texturePaletteOffsetsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePaletteOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, textureCount * sizeof(uint32_t), texturePaletteOffsets, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, texturePaletteOffsetsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    free(texturePaletteOffsets);

    CHECK_GL_ERROR();
    glFlush();
    glFinish();
    malloc_trim(0);
    double end_time = get_time();
    DualLog(" took %f seconds\n", end_time - start_time);
    DebugRAM("After LoadTextures");
    return 0;
}
#pragma GCC diagnostic pop
