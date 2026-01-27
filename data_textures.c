// data_textures.c - Load textures from raw .png files on disk
#include <malloc.h>
uint32_t totalPixels;
uint32_t totalPaletteColors;
uint16_t loadedTexturesMaxIndex;
bool doubleSidedTexture[MAX_VALID_TEXTURE];
bool transparentTexture[MAX_VALID_TEXTURE];

void LoadTextures(void) {
    if (loadedTexturesMaxIndex > 0) return;

    double start_time = get_time();
    DebugRAM("start of LoadTextures");
    loadedTexturesMaxIndex = totalPixels = totalPaletteColors = 0u;
    DataParser texture_parser;
    if (!parse_data_file(&texture_parser, MAX_VALID_TEXTURE, "./Data/textures.txt")) { DualLogError("Could not parse ./Data/textures.txt!\n"); OS_Exit(1); }
    
    stbi__arena_init();
    int32_t maxIndex = -1;
    for (uint32_t k = 0; k < texture_parser.count; k++) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != UINT16_MAX) maxIndex = texture_parser.entries[k].index;
    }

    loadedTexturesMaxIndex = maxIndex + 1;
    int32_t matchedParserIdxes[MAX_VALID_TEXTURE];
    for (uint16_t i = 0; i < loadedTexturesMaxIndex; ++i) matchedParserIdxes[i] = -1;
    for (uint32_t k = 0; k < texture_parser.count; k++) { // Match parser entries to indices ahead of loops
        if (texture_parser.entries[k].index < loadedTexturesMaxIndex) {
            matchedParserIdxes[texture_parser.entries[k].index] = k;
        }
    }
    
    if (loadedTexturesMaxIndex == 0) { DualLogError("No textures found in textures.txt\n"); OS_Exit(1); }
    DualLog("Loading textures( %u/%u), using stb_image version: 2.28, ", loadedTexturesMaxIndex, loadedTexturesMaxIndex);    
    totalPixels = 0U;
    totalPaletteColors = 0U;
    int32_t widths[MAX_VALID_TEXTURE]; memset(widths,0,MAX_VALID_TEXTURE * sizeof(int32_t));
    int32_t heights[MAX_VALID_TEXTURE]; memset(heights,0,MAX_VALID_TEXTURE * sizeof(int32_t));    
    size_t offsets_size          = loadedTexturesMaxIndex * sizeof(uint32_t);
    size_t sizes_size            = loadedTexturesMaxIndex * 2 * sizeof(int32_t);
    size_t palette_offsets_size  = loadedTexturesMaxIndex * sizeof(uint32_t);
    size_t palettes_size         = MAX_UNIQUE_COLORS * sizeof(uint32_t);
    size_t indices_size          = MAX_TOTAL_PIXELS * sizeof(uint8_t);
    size_t arena_size = offsets_size + sizes_size + palette_offsets_size + palettes_size + indices_size;
    void* arena = OS_AllocateRAM(NULL, arena_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
    uint8_t* cur = (uint8_t*)arena;
    uint32_t* textureOffsets        = (uint32_t*)cur; cur += offsets_size;
    int32_t*  textureSizes          = (int32_t*)cur;  cur += sizes_size;
    uint32_t* texturePaletteOffsets = (uint32_t*)cur; cur += palette_offsets_size;
    uint32_t* texturePalettes       = (uint32_t*)cur; cur += palettes_size;
    uint8_t*  all_indices           = (uint8_t*)cur;
    uint32_t pixel_base = 0u, color_base = 0u;
    for (uint16_t i = 0; i < loadedTexturesMaxIndex; ++i) {
        int32_t currentIndex = matchedParserIdxes[i];
        if (currentIndex < 0) continue;
        
        doubleSidedTexture[currentIndex] = (texture_parser.entries[currentIndex].entflags & ENTFLAG_DOUBLESIDED) > 0 ? 1 : 0;
        transparentTexture[currentIndex] = (texture_parser.entries[currentIndex].entflags & ENTFLAG_TRANSPARENT) > 0 ? 1 : 0;
        OsFileHandle fd; int st_size; void* map = OS_OpenAndAllocateFileBufferReadonly(texture_parser.entries[currentIndex].path, &fd, &st_size);
        unsigned char* pixels = stbi_load_from_memory(map, (size_t)st_size, &widths[currentIndex], &heights[currentIndex]);
        OS_DeallocateRAM(map, (size_t)st_size);
        if (!pixels) { DualLogError("stbi_load failed for %s\n", texture_parser.entries[currentIndex].path); OS_Exit(1); }

        totalPixels += widths[currentIndex] * heights[currentIndex];
        int32_t numPixels = widths[currentIndex] * heights[currentIndex];
        uint8_t* indices = all_indices + pixel_base;
        uint32_t palette[256];
        uint8_t  remap[256] = {0};
        uint32_t pal_size = 0;
        int loopIter0 = 0;
        int loopIter1 = 0;
        for (int p = 0; p < numPixels; p++) {
            loopIter0++;
            if (loopIter0 > 16777216) break;
            
            uint32_t color = ((uint32_t*)pixels)[p];
            uint8_t idx = remap[color & 255];
            if (idx && palette[idx - 1] == color) { indices[p] = idx - 1; continue; }

            for (idx = 0; idx < pal_size; idx++) {
                loopIter1++;
                if (loopIter1 > 16777216) break;
                if (palette[idx] == color) {
                    indices[p] = idx;
                    remap[color & 255] = idx + 1;
                    goto Label_found;
                }
            }
            
            if (pal_size >= 256) { DualLogError("Texture %s exceeded 256 colors %u\n", texture_parser.entries[currentIndex].path, pal_size); /*OS_Exit(1);*/ break; }
                        
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
        pixel_base += numPixels; if (pixel_base > MAX_TOTAL_PIXELS) { DualLogError("Overflowed unique pixels buffer with %u, max size allowed: %u\n", pixel_base, MAX_TOTAL_PIXELS); OS_Exit(1); }
        color_base += pal_size;  if (color_base > MAX_UNIQUE_COLORS) { DualLogError("Overflowed palette buffer with %u, max size allowed: %u\n", color_base, MAX_UNIQUE_COLORS); OS_Exit(1); }
    }

    DebugRAM("After loop for load textures");
    DualLog("total palette colors: %u, total pixels: %u...", totalPaletteColors, totalPixels);
    int32_t packed_size = ((int32_t)totalPixels + 3) / 4 * sizeof(uint32_t);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.colorBufferID);
    void* dst = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, packed_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
    memcpy(dst, all_indices,packed_size);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.texturePalettesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPaletteColors * sizeof(uint32_t), texturePalettes, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.textureOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedTexturesMaxIndex * sizeof(uint32_t), textureOffsets, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.textureSizesID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedTexturesMaxIndex * 2 * sizeof(int32_t), textureSizes, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, Sys_Render.texturePaletteOffsetsID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, loadedTexturesMaxIndex * sizeof(uint32_t), texturePaletteOffsets, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glFlush();
    glFinish();
    free(texture_parser.entries);
    OS_DeallocateRAM(arena, arena_size); arena = NULL;
    OS_DeallocateRAM(stbi__arena_base, STBI_ARENA_SIZE); stbi__arena_base = NULL;
    #ifndef WINDOWS
        malloc_trim(0);
    #endif
    double end_time = get_time();
    DualLog(" took %.6f secs\n", end_time - start_time);
    DebugRAM("After LoadTextures and after deallocation of LoadTextures arena and stbi arena");
}
