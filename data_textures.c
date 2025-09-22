#include <malloc.h>
#include <stdint.h>
#include <stdbool.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_MAX_DIMENSIONS 2048
#include "External/stb_image.h"
#include <GL/glew.h>
#include <sys/stat.h>
#include <errno.h>
#include <uthash.h>
#include <omp.h>
#include "voxen.h"

DataParser texture_parser;
GLuint colorBufferID = 0;
GLuint textureSizesID = 0;
GLuint textureOffsetsID = 0;
GLuint texturePalettesID = 0;
GLuint texturePaletteOffsetsID = 0;
uint32_t* textureOffsets = NULL;
uint32_t* texturePaletteOffsets = NULL;
uint32_t* texturePalettes = NULL;
uint32_t totalPixels = 0;
uint32_t totalPaletteColors = 0;
int* textureSizes = NULL;
uint16_t textureCount;
bool* doubleSidedTexture = NULL;
bool* transparentTexture = NULL;
unsigned char** image_data = NULL;

bool isDoubleSided(uint32_t texIndexToCheck) { return doubleSidedTexture[texIndexToCheck]; }
bool isTransparent(uint32_t texIndexToCheck) { return transparentTexture[texIndexToCheck]; }

typedef struct {
    uint32_t color;
    uint16_t index;
    UT_hash_handle hh;
} ColorEntry;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
int32_t LoadTextures(void) {
    double start_time = get_time();
    DualLog("Loading textures");
    DebugRAM("start of LoadTextures");
    textureCount = 0;

    // Parse textures.txt
    parser_init(&texture_parser);
    if (!parse_data_file(&texture_parser, "./Data/textures.txt", 0)) { DualLogError("Could not parse ./Data/textures.txt!\n"); return 1; }

    int32_t maxIndex = -1;
    for (int32_t k = 0; k < texture_parser.count; k++) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != UINT16_MAX) {
            maxIndex = texture_parser.entries[k].index;
        }
    }

    textureCount = (uint16_t)texture_parser.count;
    if (textureCount > MAX_TEXTURE_COUNT) { DualLogError("Too many textures in parser count %d, greater than 2048!\n", textureCount); return 1; }
    if (textureCount == 0) { DualLogError("No textures found in textures.txt\n"); return 1; }

    DualLog("(%d) with max index %d, using stb_image version: 2.28...", textureCount, maxIndex);
    image_data = malloc(textureCount * sizeof(unsigned char*));
    textureOffsets = malloc(textureCount * sizeof(uint32_t));
    textureSizes = malloc(textureCount * 2 * sizeof(int));
    texturePaletteOffsets = malloc(textureCount * sizeof(uint32_t));
    doubleSidedTexture = malloc(textureCount * sizeof(bool));
    transparentTexture = malloc(textureCount * sizeof(bool));
    size_t maxFileSize = 2760000;
    uint32_t totalPaletteColorsExtraSized = 180000;
    texturePalettes = malloc(totalPaletteColorsExtraSized * sizeof(uint32_t));

    int32_t* widths = malloc(textureCount * sizeof(int32_t));
    int32_t* heights = malloc(textureCount * sizeof(int32_t));
    int32_t* matchedParserIdxes = malloc(textureCount * sizeof(int32_t));

    // Initialize arrays
    for (int32_t i = 0; i < textureCount; i++) {
        image_data[i] = NULL;
        widths[i] = 0;
        heights[i] = 0;
        matchedParserIdxes[i] = -1;
    }

    // Match parser entries to indices
    for (int32_t k = 0; k < texture_parser.count; k++) {
        if (texture_parser.entries[k].index < textureCount) {
            matchedParserIdxes[texture_parser.entries[k].index] = k;
        }
    }
    
    int num_threads = omp_get_max_threads();
    uint8_t** file_buffer_pool = malloc(num_threads * sizeof(uint8_t*));
    for (int i = 0; i < num_threads; i++) file_buffer_pool[i] = malloc(maxFileSize);

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        uint8_t* file_buffer = file_buffer_pool[thread_id];
        #pragma omp for schedule(dynamic)
        for (int32_t i = 0; i < textureCount; i++) {
            if (matchedParserIdxes[i] < 0) continue;
            struct stat file_stat;
            if (stat(texture_parser.entries[matchedParserIdxes[i]].path, &file_stat) != 0) { DualLogError("Failed to stat %s: %s\n", texture_parser.entries[matchedParserIdxes[i]].path, strerror(errno)); continue; }
                
            size_t file_size = file_stat.st_size;
            if (file_size > maxFileSize) { DualLogError("PNG file %s too large (%zu bytes)\n", texture_parser.entries[matchedParserIdxes[i]].path, file_size); continue; }
            
            FILE* fp = fopen(texture_parser.entries[matchedParserIdxes[i]].path, "rb");
            if (!fp) { DualLogError("Failed to open %s: %s\n", texture_parser.entries[matchedParserIdxes[i]].path, strerror(errno)); continue; }
            
            fread(file_buffer, 1, file_size, fp);
            fclose(fp);
            int w, h, n;
            image_data[i] = stbi_load_from_memory(file_buffer, file_size, &w, &h, &n, STBI_rgb_alpha);
            if (!image_data[i]) { DualLogError("stbi_load failed for %s: %s\n", texture_parser.entries[matchedParserIdxes[i]].path, stbi_failure_reason()); continue; }
            
            widths[i] = w;
            heights[i] = h;
            doubleSidedTexture[i] = texture_parser.entries[matchedParserIdxes[i]].doublesided;
            transparentTexture[i] = texture_parser.entries[matchedParserIdxes[i]].transparent;
        }
    }

    for (int i = 0; i < num_threads; i++) free(file_buffer_pool[i]);
    free(file_buffer_pool);
    malloc_trim(0);

    // Initialize OpenGL buffers
    GLuint stagingBuffer;
    glGenBuffers(1, &stagingBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (((MAX_TEXTURE_DIMENSION * MAX_TEXTURE_DIMENSION) + 1) / 2) * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);

    glGenBuffers(1, &texturePalettesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePalettesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPaletteColorsExtraSized * sizeof(uint32_t), NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBufferID);
    int32_t colorBufferSize = ((58000000 + 1) / 2) * sizeof(uint32_t);
    glBufferData(GL_SHADER_STORAGE_BUFFER, colorBufferSize, NULL, GL_STATIC_DRAW);

    ColorEntry* color_pool = malloc(textureCount * MAX_PALETTE_SIZE * sizeof(ColorEntry));
    uint32_t* pool_indices = malloc(textureCount * sizeof(uint32_t));
    memset(pool_indices, 0, textureCount * sizeof(uint32_t));
    uint16_t* indices = malloc(MAX_TEXTURE_DIMENSION * MAX_TEXTURE_DIMENSION * sizeof(uint16_t));
    uint32_t pixel_offset = 0;
    uint32_t palette_offset = 0;
    uint32_t maxPalletSize = 0;

    uint32_t** per_texture_palettes = malloc(textureCount * sizeof(uint32_t*));
    uint16_t** per_texture_indices = malloc(textureCount * sizeof(uint16_t*));
    uint32_t* per_texture_palette_sizes = malloc(textureCount * sizeof(uint32_t));
    uint32_t max_total_pixels = 55000000; // From colorBufferSize
    uint16_t* all_indices = malloc(max_total_pixels * sizeof(uint16_t));
    uint32_t* index_offsets = malloc(textureCount * sizeof(uint32_t));
    uint32_t current_index_offset = 0;
    for (int32_t i = 0; i < textureCount; i++) {
        per_texture_palettes[i] = malloc(MAX_PALETTE_SIZE * sizeof(uint32_t));
        per_texture_indices[i] = malloc(MAX_TEXTURE_DIMENSION * MAX_TEXTURE_DIMENSION * sizeof(uint16_t));
        per_texture_palette_sizes[i] = 0;
        
        index_offsets[i] = current_index_offset;
        if (matchedParserIdxes[i] >= 0 && image_data[i]) {
            current_index_offset += widths[i] * heights[i];
        }
    }

    // Parallel loop for palette construction
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (int32_t i = 0; i < textureCount; i++) {
            if (matchedParserIdxes[i] < 0 || !image_data[i]) continue;
            ColorEntry* color_table = NULL;
            uint32_t palette_size = 0;
            uint16_t* texture_indices = &all_indices[index_offsets[i]];
            uint32_t pool_start = i * MAX_PALETTE_SIZE;
            for (int32_t j = 0; j < widths[i] * heights[i] * 4; j += 4) {
                uint32_t color = ((uint32_t)image_data[i][j] << 24) | ((uint32_t)image_data[i][j + 1] << 16) |
                                ((uint32_t)image_data[i][j + 2] << 8) | (uint32_t)image_data[i][j + 3];
                ColorEntry* entry;
                HASH_FIND_INT(color_table, &color, entry);
                if (!entry) {
                    if (palette_size >= MAX_PALETTE_SIZE) {
                        DualLogError("Palette size exceeded for %s\n", texture_parser.entries[matchedParserIdxes[i]].path);
                        palette_size = MAX_PALETTE_SIZE - 1;
                        break;
                    }
                    entry = &color_pool[pool_start + palette_size];
                    entry->color = color;
                    entry->index = (uint16_t)palette_size++;
                    HASH_ADD_INT(color_table, color, entry);
                    per_texture_palettes[i][entry->index] = color;
                }
                texture_indices[j / 4] = entry->index;
            }
            per_texture_palette_sizes[i] = palette_size;
            HASH_CLEAR(hh, color_table); // No free needed, as entries are from color_pool
        }
    }

    for (int32_t i = 0; i < textureCount; i++) {
        if (matchedParserIdxes[i] < 0 || !image_data[i]) continue;
        textureOffsets[i] = totalPixels;
        texturePaletteOffsets[i] = totalPaletteColors;
        textureSizes[i * 2] = widths[i];
        textureSizes[(i * 2) + 1] = heights[i];
        uint32_t palette_size = per_texture_palette_sizes[i];
        memcpy(&texturePalettes[palette_offset], per_texture_palettes[i], palette_size * sizeof(uint32_t));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
        uint32_t* mapped_buffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                                                ((widths[i] * heights[i] + 1) / 2) * sizeof(uint32_t),
                                                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        uint16_t* texture_indices = &all_indices[index_offsets[i]];
        for (int32_t j = 0; j < widths[i] * heights[i]; j += 2) {
            uint32_t packed = (uint32_t)texture_indices[j];
            if (j + 1 < widths[i] * heights[i]) {
                packed |= (uint32_t)texture_indices[j + 1] << 16;
            }
            mapped_buffer[j / 2] = packed;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_COPY_READ_BUFFER, stagingBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, colorBufferID);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0,
                            ((pixel_offset + 1) / 2) * sizeof(uint32_t),
                            ((widths[i] * heights[i] + 1) / 2) * sizeof(uint32_t));
        pixel_offset += widths[i] * heights[i];
        palette_offset += palette_size;
        totalPixels += widths[i] * heights[i];
        totalPaletteColors += palette_size;
        if (palette_size > maxPalletSize) maxPalletSize = palette_size;
        stbi_image_free(image_data[i]);
        image_data[i] = NULL;
    }

    free(all_indices);
    free(index_offsets);
    for (int32_t i = 0; i < textureCount; i++) {
        free(per_texture_palettes[i]);
        free(per_texture_indices[i]);
    }
    
    free(per_texture_palettes);
    free(per_texture_indices);
    free(per_texture_palette_sizes);
    free(indices);
    free(color_pool);
    free(pool_indices);
    free(image_data);
    free(widths);
    free(heights);
    free(matchedParserIdxes);
    glDeleteBuffers(1, &stagingBuffer);
    malloc_trim(0);

    #ifdef DEBUG_TEXTURE_LOAD_DATA
    DualLog("Largest palette size of %d\n", maxPalletSize);
    #endif

    // Upload texture palettes
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePalettesID);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, totalPaletteColors * sizeof(uint32_t), texturePalettes);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, texturePalettesID);
    free(texturePalettes);

    // Upload texture offsets
    glGenBuffers(1, &textureOffsetsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, textureCount * sizeof(uint32_t), textureOffsets, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, textureOffsetsID);
    free(textureOffsets);

    // Upload texture sizes
    glGenBuffers(1, &textureSizesID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSizesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, textureCount * 2 * sizeof(int32_t), textureSizes, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, textureSizesID);
    free(textureSizes);

    // Upload texture palette offsets
    glGenBuffers(1, &texturePaletteOffsetsID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texturePaletteOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, textureCount * sizeof(uint32_t), texturePaletteOffsets, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, texturePaletteOffsetsID);
    free(texturePaletteOffsets);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, colorBufferID);
    glFlush();
    CHECK_GL_ERROR();
    malloc_trim(0);
    double end_time = get_time();
    DualLog(" took %f seconds\n", end_time - start_time);
    DebugRAM("After LoadTextures");
    return 0;
}
#pragma GCC diagnostic pop
