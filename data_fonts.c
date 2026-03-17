// data_fonts.c - Load Font Atlasses
#include "os.h"
#include "gl.h"
#include "voxen.h"
// #define DUMP_FONT_BITMAPS
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
// MAX_GLYPHS is 4096 as is FONT_ATLAS_SIZE which is more than twice as large as needed without hiragana/katakana present
int numPackedGlyphs = 0;
int numPackedGlyphsStopD = 0;
GLuint fontAtlasTex;
GLuint fontAtlasTexStopD;
stbtt_packedchar fontPackedChar[MAX_GLYPHS];
stbtt_packedchar fontPackedCharStopD[MAX_GLYPHS];
float fixedNumberAdvanceWidth = 0.0f; // Global for fixed-width number spacing
float fixedNumberAdvanceWidthStopD = 0.0f;
static const char* const fallbackFontPaths[] = { "./Fonts/FreeSerifBold.ttf", "./Fonts/cambriab.ttf", "./Fonts/NotoSansCJK-Bold.ttc" };
static const char* const fontPaths[] = { "./Fonts/SystemShockText.ttf", "./Fonts/StopD.ttf" };

typedef struct {
    char *path;
    unsigned char *data;
    size_t size;
    stbtt_fontinfo info;
} LoadedFont;

static stbtt_fontinfo fontInfo[5];
static unsigned char *fontData[5];
LoadedFont fallbackFonts[3]; 

typedef struct {
    int32_t first;   // first codepoint in range
    int32_t count;   // number of codepoints
    int32_t startIndex; // index into fontPackedChar where this range starts
} GlyphRange;

GlyphRange fontRanges[] = {
    {0x0020, 0x7E - 0x20+1, 0},       // ASCII 94
    {0x00A0, 0xFF - 0xA0+1, 95},      // Latin-1 95
    {0x0400, 0x04FF - 0x0400+1, 95+96}, // Cyrillic 255
    {0x3040, 0x30FF - 0x3040+1, 95+96+256}, // Hiragana/Katakana 191
};

GlyphRange fontRangesStopD[] = {
    {0x0020, 0x7E - 0x20+1, 0},       // ASCII
    {0x00A0, 0xFF - 0xA0+1, 95},      // Latin-1
    {0x0400, 0x04FF - 0x0400+1, 95+96}, // Cyrillic
    {0x3040, 0x30FF - 0x3040+1, 95+96+256}, // Hiragana/Katakana
};

int32_t numFontRanges = sizeof(fontRanges)/sizeof(fontRanges[0]);

__attribute__((pure)) int32_t CodepointToPackedIndex(int32_t codepoint, int fontID) {
    if (codepoint < 32) codepoint = 32;
    if (codepoint >= 447) codepoint = 446;
    const GlyphRange* ranges = (fontID == FONT_STOPD) ? fontRangesStopD : fontRanges;
    int32_t total_packed = (fontID == FONT_STOPD) ? numPackedGlyphsStopD : numPackedGlyphs;
    for (int32_t i = 0; i < numFontRanges; i++) {
        if (codepoint >= ranges[i].first && codepoint < ranges[i].first + ranges[i].count) {
            int32_t idx = ranges[i].startIndex + vmax((codepoint - ranges[i].first),0);
            if (idx < total_packed) return idx;
        }
    }
    
    return 0;
}

static LoadedFont LoadFallbackFont(const char *path, int fontInfoIdx, int collection_index) {
    OsFileHandle fd; int fontFileSize; fontData[fontInfoIdx] = OS_OpenAndAllocateFileBufferReadonly(path, &fd, &fontFileSize);
    int offset = stbtt_GetFontOffsetForIndex(fontData[fontInfoIdx], collection_index);
    if (offset < 0) { DualLogError("Invalid collection index %d for font %s\n", collection_index, path); OS_Exit(1); }
    if (!stbtt_InitFont_internal(&fontInfo[fontInfoIdx], fontData[fontInfoIdx], offset)) { DualLogError("Failed to init font at index %d in %s\n", collection_index, path); OS_Exit(1); }
    
    return (LoadedFont){ .path = (char*)path, .data = fontData[fontInfoIdx], .size = fontFileSize, .info = fontInfo[fontInfoIdx] };
}

static int GetGlyphAndFont(uint32_t codepoint, stbtt_fontinfo **outFont, uint8_t fontID) {
    int glyph = stbtt_FindGlyphIndex((fontID == FONT_STOPD ? &fontInfo[1] : &fontInfo[0]), codepoint);
    if (glyph) { *outFont = (fontID == FONT_STOPD ? &fontInfo[1] : &fontInfo[0]); return glyph; }

    for (int i = 0; i < 3; i++) {        
        glyph = stbtt_FindGlyphIndex(&fallbackFonts[i].info, codepoint);
        if (glyph) { *outFont = &fallbackFonts[i].info; return glyph; }
    }

    return 0;
}

static void write_font_cache(const char *path, uint32_t expected_glyphs, const uint64_t file_stamp, const stbtt_packedchar *packed, uint32_t actual_packed, float fixed_advance, const unsigned char *bitmap) {
    OsFileHandle fd = OS_OpenWriteonly(path);
    OS_Write(fd, &expected_glyphs, sizeof(uint32_t), path);
    OS_Write(fd, &file_stamp, sizeof(uint64_t), path);
    OS_Write(fd, &actual_packed, sizeof(uint32_t), path);
    OS_Write(fd, &fixed_advance, sizeof(float), path);
    OS_Write(fd, packed, sizeof(stbtt_packedchar) * actual_packed, path);
    OS_Write(fd, bitmap, FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, path);
    OS_Close(fd);
}

static bool load_font_cache(const char *path, int32_t expected_glyphs, const uint64_t file_stamp, stbtt_packedchar *out_packed, int32_t *out_num, float *out_fixed_advance, GLuint *out_tex) {
    OsFileHandle fd; int fontFileSize; uint8_t *map = OS_OpenAndAllocateFileBufferReadonly(path, &fd, &fontFileSize);
    if (!map || fd == OS_INVALID_HANDLE || fontFileSize <= 0) return false;
    if (fontFileSize < 20 + FONT_ATLAS_SIZE * FONT_ATLAS_SIZE) { OS_Close(fd); DualLogWarn("cache too small %s\n", path); return false; }

    const uint8_t *p = map;
    int32_t file_expected = *(int32_t*)p; p += 4;
    if (file_expected != expected_glyphs) { DualLogWarn("range mismatch %s (file:%u exp:%u)\n", path, file_expected, expected_glyphs); OS_DeallocateRAM(map, fontFileSize); return false; }

    uint64_t file_stamp_on_disk;
    __builtin_memcpy(&file_stamp_on_disk, p, sizeof(uint64_t));
    p += sizeof(uint64_t);
    if (file_stamp_on_disk != file_stamp) { OS_DeallocateRAM(map, fontFileSize); DualLogWarn("Filestamp mismatch %s\n", path); return false; }

    uint32_t actual_packed = *(uint32_t*)p; p += 4;
    if (actual_packed > MAX_GLYPHS) { OS_DeallocateRAM(map, fontFileSize); return false; }

    float fixed_advance = *(float*)p;       p += 4;
    __builtin_memcpy(out_packed, p, sizeof(stbtt_packedchar) * actual_packed);
    *out_num = (int32_t)actual_packed;
    *out_fixed_advance = fixed_advance;
    p += sizeof(stbtt_packedchar) * actual_packed;
    glCreateTextures(GL_TEXTURE_2D, 1, out_tex);
    glTextureStorage2D(*out_tex, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(*out_tex, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, p);
    glFinish(); // Ensure transfer finishes prior to dealloc.
    OS_DeallocateRAM(map, fontFileSize);
    glTextureParameteri(*out_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(*out_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(*out_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(*out_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return true;
}

#ifdef DUMP_FONT_BITMAPS
    int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data);
    static void dump_atlas_bmp(const char *bmp_path, const unsigned char *atlas_data) {
        // stbi_write_bmp expects RGB data, so expand R8 -> RGB24
        unsigned char *rgb = OS_AllocateRAM(NULL, FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * 3, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
        for (int i = 0; i < FONT_ATLAS_SIZE * FONT_ATLAS_SIZE; ++i) {
            unsigned char v = atlas_data[i];
            rgb[i*3+0] = v;
            rgb[i*3+1] = v;
            rgb[i*3+2] = v;
        }
        stbi_write_bmp(bmp_path, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 3, rgb);
        OS_DeallocateRAM(rgb, FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * 3);
    }
#endif

void InitFontAtlasses(void) {
    double t0 = get_time();
    DualLog("Loading    5 fonts...");
    OsFileHandle fd1; int sz1; fontData[0] = OS_OpenAndAllocateFileBufferReadonly(fontPaths[0],&fd1,&sz1);
    OsFileHandle fd2; int sz2; fontData[1] = OS_OpenAndAllocateFileBufferReadonly(fontPaths[1],&fd2,&sz2);
    if (!stbtt_InitFont_internal(&fontInfo[0],fontData[0],0)) { DualLogError("%s font init failed\n",fontPaths[0]); OS_Exit(1); }
    if (!stbtt_InitFont_internal(&fontInfo[1],fontData[1],0)) { DualLogError("%s font init failed\n",fontPaths[1]); OS_Exit(1); }

    // Check if either the primary font or secondary font .vfnt cache file is out of date prompting an atlas rebuild.
    FileFingerprint fp1 = {0}, fp2 = {0};
    if (!OS_GetFileFingerprint(fontPaths[0], &fp1)) DualLogError("File change detection failed for %s\n",fontPaths[0]);
    if (!OS_GetFileFingerprint(fontPaths[1], &fp2)) DualLogError("File change detection failed for %s\n",fontPaths[1]);
    uint64_t fbx_stamp1 = OS_GetFilestamp(&fp1);
    uint64_t fbx_stamp2 = OS_GetFilestamp(&fp2);
    int32_t pri_expected = 0, sec_expected = 0;
    for (int i = 0; i < numFontRanges; i++) { pri_expected += fontRanges[i].count; sec_expected += fontRangesStopD[i].count; }
    const char *pri_cache = "./Fonts/SystemShockText.vfnt";
    const char *sec_cache = "./Fonts/StopD.vfnt";
    bool pri_hit = load_font_cache(pri_cache,pri_expected,fbx_stamp1,fontPackedChar,&numPackedGlyphs,&fixedNumberAdvanceWidth,&fontAtlasTex);
    bool sec_hit = load_font_cache(sec_cache,sec_expected,fbx_stamp2,fontPackedCharStopD,&numPackedGlyphsStopD,&fixedNumberAdvanceWidthStopD,&fontAtlasTexStopD);
    if (pri_hit && sec_hit) { DualLog("took %f secs\n", get_time() - t0); return; }

    fallbackFonts[0] = LoadFallbackFont(fallbackFontPaths[0],2,0); // Preload known fallback fonts for Cyrillic, Kanji, etc.
    fallbackFonts[1] = LoadFallbackFont(fallbackFontPaths[1],3,0);
    fallbackFonts[2] = LoadFallbackFont(fallbackFontPaths[2],4,2); // Index 2 for Japanese in NotoSansCJK-Bold.ttc
    DualLog("Font ranges changed or .vfnt files not present – regenerating...");

    // Primary
    unsigned char *bmp = OS_AllocateRAM(NULL,FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * sizeof(unsigned char),PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,OS_INVALID_HANDLE);
    stbtt_pack_context pc;
    stbtt_PackBegin(&pc,bmp,FONT_ATLAS_SIZE,FONT_ATLAS_SIZE,0,16,NULL);
    pc.h_oversample = 4; // STBTT_MAX_OVERSAMPLE = 8 for chunky 2pixel black outline support for readability and to follow System Shock.
    pc.v_oversample = 4;
    pc.skip_missing = 1;
    numPackedGlyphs = 0;
    float h = 20.0f;
    for (int r = 0; r < numFontRanges; r++) {
        fontRanges[r].startIndex = numPackedGlyphs;
        for (int i = 0; i < fontRanges[r].count; i++) {
            if (numPackedGlyphs >= MAX_GLYPHS) break;
            
            uint32_t cp = fontRanges[r].first + i;
            int g = stbtt_FindGlyphIndex(&fontInfo[0], cp);
            stbtt_fontinfo *font = &fontInfo[0];
            unsigned char *data = fontData[0];
            if (!g) {
                g = GetGlyphAndFont(cp, &font, FONT_NORMAL);
                if (!g) continue;
                
                
                data = (font == &fontInfo[0]) ? fontData[0] : ((LoadedFont*)((char*)font - __builtin_offsetof(LoadedFont, info)))->data;
            }
            
            float height = h;
            if (font != &fontInfo[0]) height *= 1.2f;
            stbtt_PackFontRange(&pc,data,0,height,cp,1,&fontPackedChar[numPackedGlyphs]);
            int idx = numPackedGlyphs++;
            if (cp >= '0' && cp <= '9') fixedNumberAdvanceWidth = vmax(fixedNumberAdvanceWidth, fontPackedChar[idx].xadvance);
        }
    }
    
    OS_DeallocateRAM(pc.pack_info,0);
    glCreateTextures(GL_TEXTURE_2D,1,&fontAtlasTex);
    glTextureStorage2D(fontAtlasTex,1,GL_R8,FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTex,0,0,0,FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, bmp);
    glTextureParameteri(fontAtlasTex,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTex,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    write_font_cache(pri_cache,pri_expected,fbx_stamp1,fontPackedChar, numPackedGlyphs,fixedNumberAdvanceWidth, bmp);
    #ifdef DUMP_FONT_BITMAPS
        dump_atlas_bmp("./Fonts/SystemShockText_atlas.bmp", bmp);
    #endif
        
    // Secondary
    __builtin_memset(bmp,0,FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * sizeof(unsigned char));
    stbtt_pack_context pc2;
    stbtt_PackBegin(&pc2, bmp, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 0, 16, NULL);
    pc2.h_oversample = 4; // STBTT_MAX_OVERSAMPLE = 8
    pc2.v_oversample = 4;
    pc2.skip_missing = 1;
    numPackedGlyphsStopD = 0;
    float h2 = 54.0f;
    for (int r = 0; r < numFontRanges; r++) {
        fontRangesStopD[r].startIndex = numPackedGlyphsStopD;
        for (int i = 0; i < fontRangesStopD[r].count; i++) {
            if (numPackedGlyphsStopD >= MAX_GLYPHS) break;
            uint32_t cp = fontRangesStopD[r].first + i;
            int g = stbtt_FindGlyphIndex(&fontInfo[1], cp);
            stbtt_fontinfo *font = &fontInfo[1];
            unsigned char *data = fontData[1];
            if (!g) {
                g = GetGlyphAndFont(cp, &font, FONT_STOPD);
                if (!g) continue;
                
                data = (font == &fontInfo[0]) ? fontData[0] : ((LoadedFont*)((char*)font - __builtin_offsetof(LoadedFont, info)))->data;
            }
            
            float height = h2;
            if (font != &fontInfo[1]) height *= 1.2f;
            stbtt_PackFontRange(&pc2, data, 0, height, cp, 1, &fontPackedCharStopD[numPackedGlyphsStopD]);
            int idx = numPackedGlyphsStopD++;
            if (cp >= '0' && cp <= '9') fixedNumberAdvanceWidthStopD = vmax(fixedNumberAdvanceWidthStopD, fontPackedCharStopD[idx].xadvance);
        }
    }
    
    OS_DeallocateRAM(pc2.pack_info,0);
    glCreateTextures(GL_TEXTURE_2D, 1, &fontAtlasTexStopD);
    glTextureStorage2D(fontAtlasTexStopD, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTexStopD, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, bmp);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    write_font_cache(sec_cache, sec_expected, fbx_stamp2, fontPackedCharStopD, numPackedGlyphsStopD, fixedNumberAdvanceWidthStopD, bmp);
    #ifdef DUMP_FONT_BITMAPS
        dump_atlas_bmp("./Fonts/StopD_atlas.bmp", bmp);
    #endif
    OS_DeallocateRAM(bmp,FONT_ATLAS_SIZE * FONT_ATLAS_SIZE * sizeof(unsigned char));
    DualLog(" took %f s\n", get_time() - t0);
}
