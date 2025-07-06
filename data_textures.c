#include <stdint.h>
#include <stdbool.h>
#define STB_IMAGE_IMPLEMENTATION // Indicate to stb_image to compile it in.
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#define STBI_MAX_DIMENSIONS 4096
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
    DebugRAM("before LoadTextures");
    textureCount = 0;
    
    // First parse ./Data/textures.txt to see what textures to load to what indices
    parser_init(&texture_parser, valid_texdata_keys, 1, PARSER_DATA);
    if (!parse_data_file(&texture_parser, "./Data/textures.txt")) { DualLogError("Could not parse ./Data/textures.txt!\n"); parser_free(&texture_parser); return 1; }
    loadTextureItemInitialized[TEX_PARSER] = true;
    
    int maxIndex = -1;
    for (int k=0;k<texture_parser.count;k++) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != UINT16_MAX) {maxIndex = texture_parser.entries[k].index; }
    }
    
    textureCount = texture_parser.count;
    if (textureCount > 4096) { DualLogError("Too many textures in parser count %d, greater than 4096!\n", textureCount); CleanupLoad(true); return 1; } 
    if (textureCount == 0) { DualLog("No textures found in textures.txt\n"); parser_free(&texture_parser); return 1; }
    
    DualLog("Parsing %d textures...\n",textureCount);
    textureOffsets = malloc(textureCount * sizeof(uint32_t));
    if (!textureOffsets) { DualLogError("Failed to allocate textureOffsets buffer\n"); CleanupLoad(true); return 1; }
    loadTextureItemInitialized[TEX_OFFSETS] = true;
    DebugRAM("after textureOffsets malloc");
    
    textureSizes = malloc(textureCount * 2 * sizeof(int)); // Times 2 for x and y pairs flat packed (e.g. x,y,x,y,x,y for 3 textures)
    if (!textureSizes) { DualLogError("Failed to allocate textureSizes buffer\n"); free(textureOffsets); CleanupLoad(true); return 1; }
    loadTextureItemInitialized[TEX_SIZES] = true;
    DebugRAM("after textureSizes malloc");

    texturePaletteOffsets = malloc(textureCount * sizeof(uint32_t));
    if (!texturePaletteOffsets) { DualLogError("Failed to allocate textureOffsets buffer\n"); CleanupLoad(true); return 1; }
    loadTextureItemInitialized[TEX_PALOFFSETS] = true;
    DebugRAM("after texturePaletteOffsets mallocn");
    
    size_t maxFileSize = 3662509;//2048 * 2048 * 4 * sizeof(uint8_t);
    uint8_t * file_buffer = malloc(maxFileSize); // Reused buffer for loading .png files.  64MB for 4096 * 4096 image.
    if (!file_buffer) { DualLogError("Failed to allocate file buffer for loading png files\n"); CleanupLoad(true); return 1; }
    
    DebugRAM("after file_buffer malloc");

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
        
        FILE* fp = fopen(texture_parser.entries[matchedParserIdx].path, "rb");
        if (!fp) { DualLogError("Failed to open %s\n", texture_parser.entries[matchedParserIdx].path); CleanupLoad(true); return 1; }
        
        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        if (file_size > maxFileSize) { DualLogError("PNG file %s too large (%zu bytes)\n", texture_parser.entries[matchedParserIdx].path, file_size); fclose(fp); CleanupLoad(true); return 1; }
        
        fseek(fp, 0, SEEK_SET);
        size_t read_size = fread(file_buffer, 1, file_size, fp);
        fclose(fp);
        if (read_size != file_size) { DualLogError("Failed to read %s\n", texture_parser.entries[matchedParserIdx].path); CleanupLoad(true);  return 1; }

        // Decode PNG from shared buffer
        int width, height, channels;
        unsigned char* image_data = stbi_load_from_memory(file_buffer, file_size, &width, &height, &channels, STBI_rgb_alpha);
        if (!image_data) { DualLogError("stbi_load failed for %s: %s\n", texture_parser.entries[matchedParserIdx].path, stbi_failure_reason()); CleanupLoad(true); return 1; }
        
        // Build palette for this texture using uthash
        ColorEntry *color_table = NULL, *entry, *tmp;
        uint32_t palette_size = 0;
        for (int j = 0; j < width * height * 4; j += 4) {
            uint32_t color = ((uint32_t)image_data[j] << 24) | ((uint32_t)image_data[j + 1] << 16) |
                             ((uint32_t)image_data[j + 2] << 8) | (uint32_t)image_data[j + 3];
            HASH_FIND_INT(color_table, &color, entry);
            if (!entry) {
                if (palette_size >= MAX_PALETTE_SIZE) { DualLog("\033[33mWARNING: Palette size exceeded for %s\033[0m\n", texture_parser.entries[matchedParserIdx].path); palette_size = MAX_PALETTE_SIZE - 1; break; }
                
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
    if (!texturePalettes) { DualLogError("Failed to allocate texturePalettes\n"); CleanupLoad(true); return 1; }
    loadTextureItemInitialized[TEX_PALETS] = true;
    DebugRAM("after texturePalettes malloc");

    // Create SSBO for texture palettes
    glGenBuffers(1, &texturePalettesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePalettesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPaletteColors * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    DebugRAM("after glGenBuffers texturePalettesID");
    
    // Create SSBO for color buffer
    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBufferID);
//     glBufferData(GL_SHADER_STORAGE_BUFFER, totalPixels * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPixels * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
//     glBufferStorage(GL_SHADER_STORAGE_BUFFER, totalPixels * sizeof(uint32_t), NULL, GL_MAP_WRITE_BIT);
//     void *mapped_buffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, totalPixels * sizeof(uint32_t),
//                                            GL_MAP_WRITE_BIT);
    
    
    loadTextureItemInitialized[TEX_SSBOS] = true;
    DebugRAM("after glGenBuffers colorBufferID");

    // Second pass: Load textures into color buffer
    uint32_t pixel_offset = 0;
    uint32_t palette_offset = 0;
    uint32_t maxPalletSize = 0;
    uint32_t *indices = malloc(4096 * 2048 * sizeof(uint32_t));
    if (!indices) { DualLogError("Failed to allocate indices buffer\n"); CleanupLoad(true); return 1; }
    
    DebugRAM("after indices malloc");
    
    GLuint stagingBuffer;
    glGenBuffers(1, &stagingBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4096 * 2048 * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY); // Max texture size
    DebugRAM("after glGenBuffers stagingBuffer");
    
    for (int i = 0; i < textureCount; i++) {
        int matchedParserIdx = -1;
        for (int k=0;k<texture_parser.count;k++) { // Find matching index to i that was parsed from file
            if (texture_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }
        
        if (matchedParserIdx < 0) continue;

        FILE* fp = fopen(texture_parser.entries[matchedParserIdx].path, "rb");
        if (!fp) { DualLogError("Failed to open %s\n", texture_parser.entries[matchedParserIdx].path); CleanupLoad(true); return 1; }
        
        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        if (file_size > maxFileSize) { DualLogError("PNG file %s too large (%zu bytes)\n", texture_parser.entries[matchedParserIdx].path, file_size); fclose(fp); CleanupLoad(true); return 1; }
        
        fseek(fp, 0, SEEK_SET);
        size_t read_size = fread(file_buffer, 1, file_size, fp);
        fclose(fp);
        if (read_size != file_size) { DualLogError("Failed to read %s\n", texture_parser.entries[matchedParserIdx].path); CleanupLoad(true);  return 1; }

        // Decode PNG from shared buffer
        int width, height, channels;
        unsigned char* image_data = stbi_load_from_memory(file_buffer, file_size, &width, &height, &channels, STBI_rgb_alpha);
        if (!image_data) { DualLogError("stbi_load failed for %s: %s\n", texture_parser.entries[matchedParserIdx].path, stbi_failure_reason()); CleanupLoad(true); return 1; }
        
        DebugRAM("after stbi_load_from_memory for %s", texture_parser.entries[matchedParserIdx].path);
        
        // Populate palette and indices
        ColorEntry *color_table = NULL, *entry, *tmp;
        uint32_t palette_size = 0;
        for (int j = 0; j < width * height * 4; j += 4) {
            uint8_t rval = image_data[j];
            uint8_t gval = image_data[j + 1];
            uint8_t bval = image_data[j + 2];
            uint8_t aval = image_data[j + 3];
//             if (aval < 1) rval = gval = bval = 0; // CAUSED WEIRD MISSING BLACK REGIONS, may revisit later
            uint32_t color = ((uint32_t)rval << 24) | ((uint32_t)gval << 16) |
                             ((uint32_t)bval << 8) | aval;

            HASH_FIND_INT(color_table, &color, entry);
            if (!entry) {
                if (palette_size >= MAX_PALETTE_SIZE) { DualLog("\033[33mWARNING: Palette size exceeded for %s\033[0m\n", texture_parser.entries[matchedParserIdx].path); palette_size = MAX_PALETTE_SIZE - 1; break; }

                entry = malloc(sizeof(ColorEntry));
                entry->color = color;
                entry->index = palette_size++;
                HASH_ADD_INT(color_table, color, entry);
                texturePalettes[palette_offset + entry->index] = color;
            }
            
            indices[j / 4] = entry->index;
        }

        // Upload indices to colorBuffer
//         glBufferSubData(GL_SHADER_STORAGE_BUFFER, pixel_offset * sizeof(uint32_t), width * height * sizeof(uint32_t), indices);
//         glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT); // Drops RAM by 1MB in btop report
//         GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
//         glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED); // Drops RAM by 1MB in btop report.  Upload to GPU RIGHT NOW!  please?
//         glDeleteSync(fence);
//         glFlush(); // Drops RAM by 1MB in btop report.  Purty please?
//         glFinish(); // Drops RAM by 5MB in btop report.  Oh for the love of all that is holy JUST UPLOAD IT TO THE GPU AND FREE IT YOU STUPID OPENGL DRIVER!!!
//         DebugRAM("after glFinish for %s", texture_parser.entries[matchedParserIdx].path);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
        void *mapped_buffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, width * height * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        if (!mapped_buffer) { DualLogError("Failed to map stagingBuffer for %s\n", texture_parser.entries[matchedParserIdx].path); free(indices); glDeleteBuffers(1, &stagingBuffer); CleanupLoad(true); return 1; }
        memcpy(mapped_buffer, indices, width * height * sizeof(uint32_t));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_COPY_READ_BUFFER, stagingBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, colorBufferID);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, pixel_offset * sizeof(uint32_t), width * height * sizeof(uint32_t));
        glFlush();
        glFinish();
        DebugRAM("after copy to colorBufferID for %s (index %d)", texture_parser.entries[matchedParserIdx].path, i);

//         memcpy((uint32_t *)mapped_buffer + pixel_offset, indices, width * height * sizeof(uint32_t));
//         glFlushMappedBufferRange(GL_SHADER_STORAGE_BUFFER, pixel_offset, width * height * sizeof(uint32_t));
        pixel_offset += width * height;
        palette_offset += palette_size;
//         if (palette_size > 4096U) DualLog("Loaded %s with large palette size of \033[33m%d!\033[0m\n", texture_parser.entries[matchedParserIdx].path, palette_size);
        if (palette_size > maxPalletSize) maxPalletSize = palette_size; // Keep track of which had the largest.
        
        // Clean up
        HASH_ITER(hh, color_table, entry, tmp) {
            HASH_DEL(color_table, entry);
            free(entry);
        }
        
//         DebugRAM("before stbi_image_free for %s", texture_parser.entries[matchedParserIdx].path);
        stbi_image_free(image_data);
//         DebugRAM("after stbi_image_free for %s", texture_parser.entries[matchedParserIdx].path);
    }
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);  
    glDeleteBuffers(1, &stagingBuffer);
    free(indices);
    free(file_buffer);
    DebugRAM("freeing indices and file_buffer");
//     glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    glFlush();
    glFinish();
    DebugRAM("after SSBO upload");
    DualLog("Largest palette size of %d\n", maxPalletSize);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, colorBufferID); // Set static buffer once for all shaders
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePalettesID);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, totalPaletteColors * sizeof(uint32_t), texturePalettes);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, texturePalettesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    DualLog("Total pixels in buffer %d (", totalPixels);
    print_bytes_no_newline(totalPixels * 4);
    DualLog("), total palette colors %d (", totalPaletteColors);
    print_bytes_no_newline(totalPaletteColors * 4);
    DualLog(")\n");

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
    DebugRAM("CleanupLoad for LoadTextures");
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
