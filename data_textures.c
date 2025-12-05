// data_textures.c - Fast single-threaded texture loader (per-texture 8-bit palettized)
#include "os.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "External/stb_image.h"
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include "voxen.h"
#include "entity.h"
int close (int filedes); // #include <unistd.h>

GLuint colorBufferID = 0;
GLuint textureSizesID = 0;
GLuint textureOffsetsID = 0;
GLuint texturePalettesID = 0;
GLuint texturePaletteOffsetsID = 0;
uint32_t totalPixels = 0u;
uint32_t totalPaletteColors = 0u;
uint16_t loadedTextures = 0u;
bool doubleSidedTexture[MAX_VALID_TEXTURE] = {0};
bool transparentTexture[MAX_VALID_TEXTURE] = {0};

typedef union {
    uint32_t u32;
    uint8_t  u8[4];
} Packed4;

void LoadTextures(void) {
    double start_time = get_time();
    DualLog("Loading textures");
    DebugRAM("start of LoadTextures");
    stbi__arena_init();
    loadedTextures = 0u;
    totalPixels = 0U;
    totalPaletteColors = 0U;
    unsigned char** image_data[MAX_VALID_TEXTURE]; memset(image_data,0,MAX_VALID_TEXTURE * sizeof(char**));
    int32_t widths[MAX_VALID_TEXTURE]; memset(widths,0,MAX_VALID_TEXTURE * sizeof(int32_t));
    int32_t heights[MAX_VALID_TEXTURE]; memset(heights,0,MAX_VALID_TEXTURE * sizeof(int32_t));
    FILE *file = fopen("./Data/textures.txt", "r");
    if (!file) { DualLogError("Cannot open %s: %s\n", "./Data/textures.txt", strerror(errno)); OS_Exit(1); }
    
    size_t offsets_size          = MAX_VALID_TEXTURE * sizeof(uint32_t);
    size_t sizes_size            = MAX_VALID_TEXTURE * 2 * sizeof(int32_t);
    size_t palette_offsets_size  = MAX_VALID_TEXTURE * sizeof(uint32_t);
    uint32_t maxUniqueColors = 80000u; uint32_t maxTotalPixels = 24000000u;
    size_t palettes_size         = maxUniqueColors * sizeof(uint32_t);
    size_t indices_size          = maxTotalPixels * sizeof(uint8_t);
    size_t arena_size = offsets_size + sizes_size + palette_offsets_size + palettes_size + indices_size;
    void* arena = mmap(NULL, arena_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
    if (arena == MAP_FAILED) { DualLogError("Failed to mmap texture arena\n"); OS_Exit(1); }
    uint8_t* cur = (uint8_t*)arena;
    uint32_t* textureOffsets        = (uint32_t*)cur; cur += offsets_size;
    int32_t*  textureSizes          = (int32_t*)cur;  cur += sizes_size;
    uint32_t* texturePaletteOffsets = (uint32_t*)cur; cur += palette_offsets_size;
    uint32_t* texturePalettes       = (uint32_t*)cur; cur += palettes_size;
    uint8_t*  all_indices           = (uint8_t*)cur;
    uint32_t pixel_base = 0u, color_base = 0u, lineNum = 0u;
    char line[1024];
    int32_t currentIndex = -1;
    while (fgets(line, sizeof(line), file)) { // First pass: count entries and find max index
        lineNum++;
        if (lineNum >= 65535) { DualLogError("textures.txt too large!  Exceeds 65535 lines!\n"); OS_Exit(1); }
        
        char *start = line;
        if (strlen(start) < 3) continue; // Must have at least k:v, skip if shorter
        
        while (data_parser_isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && data_parser_isspace((unsigned char)*end)) { *end = '\0'; end--; }
        if (*start == '\0' || (start[0] == '/' && start[1] == '/')) continue;
        
        char filePath[MAX_PATH];
        if (*start == '#') {
            strncpy(filePath, start + 1, MAX_PATH - 1); // start + 1 to not get the # character, minus 1 for null terminator
            filePath[MAX_PATH - 1] = '\0';
            currentIndex = -1;
            continue;
        }

        char *colon = strchr(start, ':');
        if (colon) {
            *colon = '\0';
            char *key = start;
            char *value = colon + 1;
            while (data_parser_isspace((unsigned char)*key)) key++;
            while (data_parser_isspace((unsigned char)*value)) value++;
            if (*key && *value) {
                char trimmed_key[256];
                char trimmed_value[256];
                strncpy(trimmed_key, key, sizeof(trimmed_key) - 1);
                strncpy(trimmed_value, value, sizeof(trimmed_value) - 1);
                trimmed_key[sizeof(trimmed_key) - 1] = '\0';
                trimmed_value[sizeof(trimmed_value) - 1] = '\0';
                char *key_end = trimmed_key + strlen(trimmed_key) - 1;
                char *val_end = trimmed_value + strlen(trimmed_value) - 1;
                while (key_end > trimmed_key && data_parser_isspace((unsigned char)*key_end)) *key_end-- = '\0';
                while (val_end > trimmed_value && data_parser_isspace((unsigned char)*val_end)) *val_end-- = '\0';
            //     sanitize_utf8_ascii(trimmed_key); // If I need to, turn these back on.  My files are clean, however.
            //     sanitize_utf8_ascii(trimmed_value);
                if (strncmp(trimmed_key, "index", sizeof(trimmed_key)) == 0) { // Got the index, now load the actual texture.
                    currentIndex = parse_numberu16(trimmed_value, start, lineNum);
                    if (currentIndex < 0 || currentIndex > MAX_VALID_TEXTURE) { DualLogError("Invalid textures.txt index entry on line %u::%s\n",lineNum,line); OS_Exit(1); }
                    
                    if (currentIndex > loadedTextures) loadedTextures = currentIndex; // Up to the creator of textures.txt to ensure no holes.
                    
                    int fd = open(filePath, O_RDONLY);
                    if (fd < 0) { DualLogError("Failed to open %s: %s\n", filePath, strerror(errno)); OS_Exit(1); }

                    struct stat st;
                    fstat(fd, &st);
                    void* map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
                    close(fd);
                    if (map == MAP_FAILED) { DualLogError("mmap failed for %s\n", filePath); OS_Exit(1); }

                    unsigned char* pixels = stbi_load_from_memory(map, st.st_size, &widths[currentIndex], &heights[currentIndex]);
                    munmap(map, st.st_size);
                    if (!pixels) { DualLogError("stbi_load failed for %s\n", filePath); OS_Exit(1); }

                    totalPixels += widths[currentIndex] * heights[currentIndex];
                    int32_t numPixels = widths[currentIndex] * heights[currentIndex];
                    uint8_t* indices = all_indices + pixel_base;
                    uint32_t palette[256];
                    uint8_t  remap[256] = {0};
                    uint32_t pal_size = 0;
                    for (int p = 0; p < numPixels; p++) {
                        uint32_t color = ((uint32_t*)pixels)[p];
                        uint8_t idx = remap[color & 255];
                        if (idx && palette[idx - 1] == color) { indices[p] = idx - 1; continue; }

                        for (idx = 0; idx < pal_size; idx++) {
                            if (palette[idx] == color) {
                                indices[p] = idx;
                                remap[color & 255] = idx + 1;
                                goto Label_found;
                            }
                        }

                        if (pal_size >= 256) { DualLogError("Texture %d exceeded 256 colors\n", currentIndex); OS_Exit(1); }
                        
                        palette[pal_size] = color;
                        indices[p] = pal_size;
                        remap[color & 255] = pal_size + 1;
                        pal_size++;
                        Label_found:;
                    }
                    
                    totalPaletteColors += pal_size;
                    textureOffsets[currentIndex]        = pixel_base;
                    texturePaletteOffsets[currentIndex] = color_base;
                    textureSizes[currentIndex * 2]      = widths[currentIndex];
                    textureSizes[currentIndex * 2 + 1]  = heights[currentIndex];
                    memcpy(texturePalettes + color_base, palette, pal_size * sizeof(uint32_t));
                    pixel_base += numPixels; if (pixel_base > maxTotalPixels) { DualLogError("Overflowed unique pixels buffer with %u, max size allowed: %u\n",pixel_base,maxTotalPixels); OS_Exit(1); }
                    color_base += pal_size;  if (color_base > maxUniqueColors) { DualLogError("Overflowed palette buffer with %u, max size allowed: %u\n",color_base,maxUniqueColors); OS_Exit(1); }
                    continue;
                } else if (currentIndex < 0) { DualLogError("index wasn't the first key after %s on line %u\n",filePath,lineNum); OS_Exit(1); }

                     if (strcmp(trimmed_key, "doublesided") == 0) doubleSidedTexture[currentIndex] = parse_bool(trimmed_value, start, lineNum);
                else if (strcmp(trimmed_key, "transparent") == 0) transparentTexture[currentIndex] = parse_bool(trimmed_value, start, lineNum);
            } else DualLogWarn("Invalid key-value pair at line %u: %s\n", lineNum, start);
        } else {
            DualLogWarn("No colon found in line %u: %s\n", lineNum, start);
        }
    }

    fclose(file);
    DebugRAM("After loop for load textures");
    if (loadedTextures == 0) { DualLogError("No textures found in textures.txt\n"); OS_Exit(1); }
    
    loadedTextures++; // Increase to be a 1-based count for the SSBO sizes and print reports.
    DualLog("(%u), using stb_image version: 2.28, total palette colors: %u, total pixels: %u...", loadedTextures, totalPaletteColors, totalPixels);
    int32_t packed_size = ((int32_t)totalPixels + 3) / 4 * sizeof(uint32_t);
    colorBufferID = SetupSSBO(colorBufferID, 12, packed_size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBufferID);
    void* dst = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, packed_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
    memcpy(dst, all_indices,packed_size);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    texturePalettesID       = SetupSSBO(texturePalettesID,       16, totalPaletteColors * sizeof(uint32_t), texturePalettes,       GL_STATIC_DRAW);
    textureOffsetsID        = SetupSSBO(textureOffsetsID,        14, loadedTextures * sizeof(uint32_t), textureOffsets,        GL_STATIC_DRAW);
    textureSizesID          = SetupSSBO(textureSizesID,          15, loadedTextures * 2 * sizeof(int32_t), textureSizes,      GL_STATIC_DRAW);
    texturePaletteOffsetsID = SetupSSBO(texturePaletteOffsetsID, 17, loadedTextures * sizeof(uint32_t), texturePaletteOffsets, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glFlush();
    glFinish();
    CHECK_GL_ERROR();
    madvise(arena, arena_size, MADV_DONTNEED); munmap(arena, arena_size); arena = NULL;
    madvise(stbi__arena_base, STBI_ARENA_SIZE, MADV_DONTNEED); munmap(stbi__arena_base, STBI_ARENA_SIZE); stbi__arena_base = NULL; 
    double end_time = get_time();
    DualLog(" took %.6f secs\n", end_time - start_time);
    DebugRAM("After LoadTextures and after munmap of LoadTextures arena and stbi arena");
}
