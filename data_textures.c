#include <stdint.h>
#include <stdbool.h>
#define STB_IMAGE_IMPLEMENTATION // Indicate to stb_image to compile it in.
#define STBI_NO_PSD // Excluded image formats to shrink binary size.
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include "stb_image.h"
#include <GL/glew.h>
#include <uthash.h>
#include "data_parser.h"
#include "data_textures.h"
#include "constants.h"
#include "debug.h"
#include "render.h"

#define MAX_PALETTE_SIZE 65535u

DataParser texture_parser;
GLuint colorBufferID = 0; // Single color buffer
GLuint textureSizesID = 0;
GLuint textureOffsetsID = 0;
GLuint texturePalettesID = 0; // New SSBO for palette colors
GLuint texturePaletteOffsetsID = 0; // New SSBO for palette offsets
uint32_t * textureOffsets = NULL; // Pixel offsets
uint32_t *texturePaletteOffsets = NULL; // Offsets into texturePalettes
uint32_t *texturePalettes = NULL; // Concatenated palette colors
uint32_t totalPixels = 0; // Total pixels across all textures
uint32_t totalPaletteColors = 0; // Total colors across all palettes
int * textureSizes = NULL; // Needs to be textureCount * 2
int textureCount;
const char *valid_texdata_keys[] = {"index"};

typedef struct {
    uint32_t color; // RGBA color (packed)
    int index;      // Palette index
    UT_hash_handle hh;
} ColorEntry;

typedef enum {
    TEX_PARSER = 0,
    TEX_OFFSETS,
    TEX_SIZES,
    TEX_PALOFFSETS,
    TEX_PALETS,
    TEX_SSBOS,
    TEX_COUNT // Number of subsystems
} TextureLoadDataType;

bool loadTextureItemInitialized[TEX_COUNT] = { [0 ... TEX_COUNT - 1] = false };

int LoadTextures(void) {
    textureCount = 0;
    
    // First parse ./Data/textures.txt to see what textures to load to what indices
    parser_init(&texture_parser, valid_texdata_keys, 1, PARSER_DATA);
    if (!parse_data_file(&texture_parser, "./Data/textures.txt")) {
        printf("ERROR: Could not parse ./Data/textures.txt!\n");
        parser_free(&texture_parser);
        return 1;
    }
    
    loadTextureItemInitialized[TEX_PARSER] = true;
    
    int maxIndex = -1;
    for (int k=0;k<texture_parser.count;k++) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != UINT16_MAX) {maxIndex = texture_parser.entries[k].index; }
    }
    
    textureCount = texture_parser.count;
    if (textureCount > 4096) { printf("ERROR: Too many textures in parser count %d, greater than 4096!\n", textureCount); CleanupLoad(true); return 1; } 
    
    if (textureCount == 0) {
        fprintf(stderr, "ERROR: No textures found in textures.txt\n");
        parser_free(&texture_parser);
        return 1;
    }
    
    printf("Parsing %d textures...\n",textureCount);

    textureOffsets = malloc(textureCount * sizeof(uint32_t));
    if (!textureOffsets) { fprintf(stderr, "ERROR: Failed to allocate textureOffsets buffer\n"); CleanupLoad(true); return 1; }
    loadTextureItemInitialized[TEX_OFFSETS] = true;

    textureSizes = malloc(textureCount * 2 * sizeof(int)); // Times 2 for x and y pairs flat packed (e.g. x,y,x,y,x,y for 3 textures)
    if (!textureSizes) { fprintf(stderr, "ERROR: Failed to allocate textureSizes buffer\n"); free(textureOffsets); CleanupLoad(true); return 1; }
    loadTextureItemInitialized[TEX_SIZES] = true;

    texturePaletteOffsets = malloc(textureCount * sizeof(uint32_t));
    if (!texturePaletteOffsets) { fprintf(stderr, "ERROR: Failed to allocate textureOffsets buffer\n"); CleanupLoad(true); return 1; }
    loadTextureItemInitialized[TEX_PALOFFSETS] = true;
    
    // First Pass: Calculate total pixels and offsets
    totalPixels = 0;
    totalPaletteColors = 0;
    for (int i = 0; i < textureCount; i++) {
        textureOffsets[i] = totalPixels;
        texturePaletteOffsets[i] = totalPaletteColors;
        int matchedParserIdx = -1;
        for (int k=0;k<texture_parser.count;k++) {
            if (texture_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }
        
        if (matchedParserIdx < 0) continue;
//         if (!texture_parser.entries[matchedParserIdx].path || texture_parser.entries[matchedParserIdx].path[0] == '\0') continue;
        
        int width, height, channels;
        unsigned char* image_data = stbi_load(texture_parser.entries[matchedParserIdx].path,&width,&height,&channels,STBI_rgb_alpha);
        if (!image_data) { fprintf(stderr, "stbi_load failed for %s: %s\n", texture_parser.entries[matchedParserIdx].path, stbi_failure_reason()); CleanupLoad(true); return 1; }
        
        // Build palette for this texture using uthash
        ColorEntry *color_table = NULL, *entry, *tmp;
        uint32_t palette_size = 0;
        for (int j = 0; j < width * height * 4; j += 4) {
            uint32_t color = ((uint32_t)image_data[j] << 24) | ((uint32_t)image_data[j + 1] << 16) |
                             ((uint32_t)image_data[j + 2] << 8) | (uint32_t)image_data[j + 3];
            HASH_FIND_INT(color_table, &color, entry);
            if (!entry) {
                if (palette_size >= MAX_PALETTE_SIZE) { printf("WARNING: Palette size exceeded for %s\n", texture_parser.entries[matchedParserIdx].path); palette_size = MAX_PALETTE_SIZE - 1; break; }// { fprintf(stderr, "ERROR: Palette size exceeded for %s\n", texture_parser.entries[matchedParserIdx].path); HASH_CLEAR(hh, color_table); stbi_image_free(image_data); CleanupLoad(true); return 1; }
                
                entry = malloc(sizeof(ColorEntry));
                entry->color = color;
                entry->index = palette_size++;
                HASH_ADD_INT(color_table, color, entry);
            }
        }

        totalPaletteColors += palette_size;
        HASH_ITER(hh, color_table, entry, tmp) {
            HASH_DEL(color_table, entry);
            free(entry); // Clean up hash table
        }
        
        totalPixels += width * height;
        textureSizes[i * 2] = width;
        textureSizes[(i * 2) + 1] = height;
        stbi_image_free(image_data);
    }
    
    texturePalettes = malloc(totalPaletteColors * sizeof(uint32_t));
    if (!texturePalettes) { fprintf(stderr, "ERROR: Failed to allocate texturePalettes\n"); CleanupLoad(true); return 1; }
   
    loadTextureItemInitialized[TEX_PALETS] = true;

    // Create SSBO for texture palettes
    glGenBuffers(1, &texturePalettesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePalettesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPaletteColors * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    
    // Create SSBO for color buffer
    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPixels * sizeof(uint32_t), NULL, GL_STATIC_DRAW);

    loadTextureItemInitialized[TEX_SSBOS] = true;

    // Second pass: Load textures into color buffer
    uint32_t pixel_offset = 0;
    uint32_t palette_offset = 0;
    uint32_t maxPalletSize = 0;
    for (int i = 0; i < textureCount; i++) {
        int matchedParserIdx = -1;
        for (int k=0;k<texture_parser.count;k++) { // Find matching index to i that was parsed from file
            if (texture_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }
        
        if (matchedParserIdx < 0) continue;

        int width, height, channels;
        uint8_t * image_data = stbi_load(texture_parser.entries[matchedParserIdx].path,&width,&height,&channels,STBI_rgb_alpha);
        if (!image_data) { fprintf(stderr, "stbi_load failed for %s: %s\n", texture_parser.entries[matchedParserIdx].path, stbi_failure_reason()); CleanupLoad(true); return 1; }
        
        // Populate palette and indices
        ColorEntry *color_table = NULL, *entry, *tmp;
        uint32_t palette_size = 0;
        uint32_t *indices = malloc(width * height * sizeof(uint32_t));
        if (!indices) { fprintf(stderr, "ERROR: Failed to allocate indices buffer\n"); stbi_image_free(image_data); CleanupLoad(true); return 1; }
        
        for (int j = 0; j < width * height * 4; j += 4) {
            uint8_t rval = image_data[j];
            uint8_t gval = image_data[j + 1];
            uint8_t bval = image_data[j + 2];
            uint8_t aval = image_data[j + 3];
//             if (aval < 1) rval = gval = bval = 0;
            uint32_t color = ((uint32_t)rval << 24) | ((uint32_t)gval << 16) |
                             ((uint32_t)bval << 8) | aval;

            HASH_FIND_INT(color_table, &color, entry);
            if (!entry) {
                if (palette_size >= MAX_PALETTE_SIZE) { printf("WARNING: Palette size exceeded for %s\n", texture_parser.entries[matchedParserIdx].path); palette_size = MAX_PALETTE_SIZE - 1; break; }// { fprintf(stderr, "ERROR: Palette size exceeded for %s\n", texture_parser.entries[matchedParserIdx].path); HASH_CLEAR(hh, color_table); stbi_image_free(image_data); CleanupLoad(true); return 1; }

                entry = malloc(sizeof(ColorEntry));
                entry->color = color;
                entry->index = palette_size++;
                HASH_ADD_INT(color_table, color, entry);
                texturePalettes[palette_offset + entry->index] = color;
            }
            
            indices[j / 4] = entry->index;
        }

        // Upload indices to colorBuffer
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, pixel_offset * sizeof(uint32_t), width * height * sizeof(uint32_t), indices);
        pixel_offset += width * height;
        palette_offset += palette_size;
//         if (palette_size > 4096U) printf("Loaded %s with large palette size of %d!\n", texture_parser.entries[matchedParserIdx].path, palette_size);
        if (palette_size > maxPalletSize) maxPalletSize = palette_size; // Keep track of which had the largest.
        
        // Clean up
        HASH_ITER(hh, color_table, entry, tmp) {
            HASH_DEL(color_table, entry);
            free(entry);
        }
        
        free(indices);
        stbi_image_free(image_data);
    }
    
    printf("Largest palette size of %d\n", maxPalletSize);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, colorBufferID); // Set static buffer once for all shaders
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePalettesID);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, totalPaletteColors * sizeof(uint32_t), texturePalettes);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, texturePalettesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    printf("Total pixels in buffer %d (", totalPixels);
    print_bytes_no_newline(totalPixels * 4);
    printf("), total palette colors %d (", totalPaletteColors);
    print_bytes_no_newline(totalPaletteColors * 4);
    printf(")\n");

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
    
    glGenBuffers(1, &texturePaletteOffsetsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePaletteOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, textureCount * sizeof(uint32_t), texturePaletteOffsets, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, texturePaletteOffsetsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    glUniform1ui(textureCountLoc_chunk, textureCount);

    CleanupLoad(false);
    return 0;
}

void CleanupLoad(bool isBad) {
    if (loadTextureItemInitialized[TEX_OFFSETS]) free(textureOffsets);
    if (loadTextureItemInitialized[TEX_SIZES]) free(textureSizes);
    if (loadTextureItemInitialized[TEX_PALOFFSETS]) free(texturePaletteOffsets);
    if (loadTextureItemInitialized[TEX_PALETS]) free(texturePalettes);
    if (loadTextureItemInitialized[TEX_PARSER]) parser_free(&texture_parser);
    
    if (isBad) { // Also free GPU resources if we are handling exit conditions.
        if (loadTextureItemInitialized[TEX_SSBOS]) {
            glDeleteBuffers(1, &colorBufferID);
            glDeleteBuffers(1, &texturePalettesID);
        }
    }
}
