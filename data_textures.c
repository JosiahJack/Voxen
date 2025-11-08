#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_MAX_DIMENSIONS 2048
#include "External/stb_image.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <uthash.h>
#include <omp.h>
#include "voxen.h"
int malloc_trim(size_t pad); // #include <malloc.h>
int close (int filedes); // #include <unistd.h>
size_t read(int fd, void* buf, size_t count); // #include <unistd.h>
int  open(const char *, int, ...); // #include <fcntl.h>
#define O_RDONLY 0

DataParser texture_parser;

// Textures
typedef struct {
    uint32_t color;
    uint8_t index;
    UT_hash_handle hh;
} ColorEntry;

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
uint16_t loadedTextures = 0;
bool* doubleSidedTexture = NULL;
bool* transparentTexture = NULL;
unsigned char** image_data = NULL;

//-----------------------------------------------------------------------------
// Load all Textures
void LoadTextures(void) {
    malloc_trim(0);
    double start_time = get_time();
    DualLog("Loading textures");
    DebugRAM("start of LoadTextures");
    loadedTextures = 0;
    if (!parse_data_file(&texture_parser, "./Data/textures.txt")) { DualLogError("Could not parse ./Data/textures.txt!\n"); exit(1); }

    int32_t maxIndex = -1;
    for (int32_t k = 0; k < texture_parser.count; k++) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != UINT16_MAX) maxIndex = texture_parser.entries[k].index;
    }

    loadedTextures = maxIndex + 1;
    if (loadedTextures == 0) { DualLogError("No textures found in textures.txt\n"); exit(1); }

    DualLog("(%d) with max index %d, using stb_image version:  2.28...", loadedTextures, maxIndex);
    image_data            =   malloc(loadedTextures * sizeof(unsigned char*));
    textureOffsets        = calloc(loadedTextures, sizeof(uint32_t));
    textureSizes          = calloc(loadedTextures * 2, sizeof(int));
    texturePaletteOffsets = calloc(loadedTextures, sizeof(uint32_t));
    doubleSidedTexture    = calloc(loadedTextures,sizeof(bool));
    transparentTexture    = calloc(loadedTextures,sizeof(bool));
    uint32_t totalPaletteColorsExtraSized = 80000;
    texturePalettes             = malloc(totalPaletteColorsExtraSized * sizeof(uint32_t));
    int32_t* widths             = malloc(loadedTextures * sizeof(int32_t));
    int32_t* heights            = malloc(loadedTextures * sizeof(int32_t));
    int32_t* matchedParserIdxes = malloc(loadedTextures * sizeof(int32_t));
    for (int32_t i = 0; i < loadedTextures; i++) {
        image_data[i] = NULL;
        widths[i] = heights[i] = 0;
        matchedParserIdxes[i] = -1;
    }

    for (int32_t k = 0; k < texture_parser.count; k++) { // Match parser entries to indices ahead of loops
        if (texture_parser.entries[k].index < loadedTextures) matchedParserIdxes[texture_parser.entries[k].index] = k;
    }
    
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (int32_t i = 0; i < loadedTextures; i++) {
            if (matchedParserIdxes[i] < 0) continue;
            int fp = open(texture_parser.entries[matchedParserIdxes[i]].path, O_RDONLY);
            if (!fp) { DualLogError("Failed to open %s: %s\n", texture_parser.entries[matchedParserIdxes[i]].path, strerror(errno)); exit(1); }
            
            struct stat file_stat;
            fstat(fp, &file_stat);
            size_t file_size = file_stat.st_size;            
            uint8_t* file_buffer = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fp, 0);
            close(fp);
            if (file_buffer == MAP_FAILED) { DualLogError("Failed to mmap %s\n", texture_parser.entries[matchedParserIdxes[i]].path); exit(1); }
                        
            int w = 1, h = 1, n = 1;
            image_data[i] = stbi_load_from_memory(file_buffer, file_size, &w, &h, &n, 4);
            if (!image_data[i]) { DualLogError("stbi_load failed for %s\n", texture_parser.entries[matchedParserIdxes[i]].path); exit(1); }
            
            widths[matchedParserIdxes[i]] = w;
            heights[matchedParserIdxes[i]] = h;
            doubleSidedTexture[matchedParserIdxes[i]] = texture_parser.entries[matchedParserIdxes[i]].doublesided > 0 ? 1 : 0;
            transparentTexture[matchedParserIdxes[i]] = texture_parser.entries[matchedParserIdxes[i]].transparent > 0 ? 1 : 0;
            munmap(file_buffer, file_size);
        }
    }

    malloc_trim(0);
    GLuint stagingBuffer;
    glGenBuffers(1, &stagingBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (((MAX_TEXTURE_DIMENSION * MAX_TEXTURE_DIMENSION) + 3) / 4) * sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
    uint32_t max_total_pixels = 33438148; // From colorBufferSize
    int32_t colorBufferSize = (((int32_t)max_total_pixels + 3) / 4) * sizeof(uint32_t);
    ColorEntry* color_pool = malloc(loadedTextures * MAX_PALETTE_SIZE * sizeof(ColorEntry));
    uint32_t* pool_indices = malloc(loadedTextures * sizeof(uint32_t));
    memset(pool_indices, 0, loadedTextures * sizeof(uint32_t));
    uint32_t** per_texture_palettes = malloc(loadedTextures * sizeof(uint32_t*));
    uint32_t* per_texture_palette_sizes = malloc(loadedTextures * sizeof(uint32_t));
    uint8_t* all_indices = malloc(max_total_pixels * sizeof(uint8_t));
    uint32_t* index_offsets = malloc(loadedTextures * sizeof(uint32_t));
    uint32_t current_index_offset = 0;
    for (uint16_t i = 0; i < loadedTextures; i++) {
        per_texture_palettes[i] = malloc(MAX_PALETTE_SIZE * sizeof(uint32_t));
        per_texture_palette_sizes[i] = 0;
        index_offsets[i] = current_index_offset;
        if (matchedParserIdxes[i] >= 0 && image_data[i]) current_index_offset += widths[i] * heights[i];
    }

    malloc_trim(0);
    
    // Parallel loop for palette construction
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (uint16_t i = 0; i < loadedTextures; i++) {
            if (matchedParserIdxes[i] < 0 || !image_data[i]) continue;
            ColorEntry* color_table = NULL;
            uint32_t palette_size = 0; // Oversized larger than max pallete size for catching overflows.
            uint8_t* texture_indices = &all_indices[index_offsets[i]];
            uint32_t pool_start = i * MAX_PALETTE_SIZE;
            for (int32_t j = 0; j < widths[i] * heights[i] * 4; j += 4) {
                uint32_t color = ((uint32_t)image_data[i][j] << 24) | ((uint32_t)image_data[i][j + 1] << 16) |
                                ((uint32_t)image_data[i][j + 2] << 8) | (uint32_t)image_data[i][j + 3];
                ColorEntry* entry;
                HASH_FIND_INT(color_table, &color, entry);
                if (!entry) {
                    if (palette_size >= MAX_PALETTE_SIZE) { DualLogError("Palette size exceeded for %s\n", texture_parser.entries[matchedParserIdxes[i]].path); exit(1); }
                    
                    entry = &color_pool[pool_start + palette_size];
                    entry->color = color;
                    entry->index = (uint8_t)palette_size++;
                    HASH_ADD_INT(color_table, color, entry);
                    per_texture_palettes[i][entry->index] = color;
                }
                texture_indices[j / 4] = entry->index;
            }
            
            per_texture_palette_sizes[i] = palette_size;
            HASH_CLEAR(hh, color_table); // No free needed, as entries are from color_pool
        }
    }
    
    colorBufferID = SetupSSBO(colorBufferID, 12, colorBufferSize, NULL, GL_STATIC_DRAW);
    uint32_t pixel_offset = 0, palette_offset = 0;
    for (uint16_t i = 0; i < loadedTextures; i++) {
        if (matchedParserIdxes[i] < 0 || !image_data[i]) continue;
        textureOffsets[i] = totalPixels;
        texturePaletteOffsets[i] = totalPaletteColors;
        textureSizes[i * 2] = widths[i];
        textureSizes[(i * 2) + 1] = heights[i];
        uint32_t palette_size = per_texture_palette_sizes[i];
        memcpy(&texturePalettes[palette_offset], per_texture_palettes[i], palette_size * sizeof(uint32_t));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, stagingBuffer);
        int32_t numberOfPixelsForThisTexture = widths[i] * heights[i];
        uint32_t* mapped_buffer = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ((numberOfPixelsForThisTexture + 3) / 4) * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        uint8_t* texture_indices = &all_indices[index_offsets[i]];
        for (int32_t j = 0; j < numberOfPixelsForThisTexture; j += 4) {
            uint32_t packed = (uint32_t)texture_indices[j];
            if (j + 1 < numberOfPixelsForThisTexture) packed |= (uint32_t)texture_indices[j + 1] << 8;
            if (j + 2 < numberOfPixelsForThisTexture) packed |= (uint32_t)texture_indices[j + 2] << 16;
            if (j + 3 < numberOfPixelsForThisTexture) packed |= (uint32_t)texture_indices[j + 3] << 24;
            mapped_buffer[j / 4] = packed;
        }

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_COPY_READ_BUFFER, stagingBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, colorBufferID);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, ((pixel_offset + 3) / 4) * sizeof(uint32_t), ((numberOfPixelsForThisTexture + 3) / 4) * sizeof(uint32_t));
        pixel_offset += numberOfPixelsForThisTexture;
        palette_offset += palette_size;
        totalPixels += numberOfPixelsForThisTexture;
        totalPaletteColors += palette_size;
        free(image_data[i]);
        malloc_trim(0);
    }

    DualLog(" total pallete colors: %u, totalPixels was: %u... ", totalPaletteColors, totalPixels);
    free(all_indices);
    free(index_offsets);
    for (uint16_t i = 0; i < loadedTextures; i++) free(per_texture_palettes[i]);
    free(per_texture_palettes);
    free(per_texture_palette_sizes);
    free(color_pool);
    free(pool_indices);
    free(image_data);
    free(widths);
    free(heights);
    free(matchedParserIdxes);
    glDeleteBuffers(1, &stagingBuffer);
    malloc_trim(0);
    texturePalettesID = SetupSSBO(texturePalettesID, 16, totalPaletteColors * sizeof(uint32_t), texturePalettes, GL_STATIC_DRAW);
    free(texturePalettes);
    textureOffsetsID = SetupSSBO(textureOffsetsID, 14, loadedTextures * sizeof(uint32_t), textureOffsets, GL_STATIC_DRAW);
    free(textureOffsets);
    textureSizesID = SetupSSBO(textureSizesID, 15, loadedTextures * 2 * sizeof(int32_t), textureSizes, GL_STATIC_DRAW);
    free(textureSizes);
    texturePaletteOffsetsID = SetupSSBO(texturePaletteOffsetsID, 17, loadedTextures * sizeof(uint32_t), texturePaletteOffsets, GL_STATIC_DRAW);
    free(texturePaletteOffsets);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glFlush();
    glFinish();
    CHECK_GL_ERROR();
    malloc_trim(0);
    double end_time = get_time();
    DualLog(" took %f seconds\n", end_time - start_time);
    DebugRAM("After LoadTextures");
}
