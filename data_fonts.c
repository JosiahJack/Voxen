// data_fonts.c - Load Font Atlasses
// #include "voxen.h" limited includes
#define FONT_GEN // Turn on when wanting to rebuild Font Atlases
#include "os.h"
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include "./External/glad/gl.h"
#include "./External/glfw3.h"
#include "voxen.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "External/stb_truetype.h"
#ifdef FONT_GEN
    #include <fontconfig/fontconfig.h>
#endif
int close (int filedes); // #include <unistd.h>

#define MAX_FALLBACK_FONTS 32
#ifdef FONT_GEN
    static char *xstrdup(const char *s) {
        size_t len = strlen(s) + 1;
        char *p = malloc(len);
        if (p) memcpy(p, s, len);
        return p;
    }
#endif
#define strdup xstrdup
// ----------------------------------------------------------------------------
// Text
float genericTextHeightFac = 0.025f;
float genericTextWidthFac = 0.0125f;
float genericTextHeightFacStopD = 0.07f;
float genericTextWidthFacStopD = 0.0182f;
int numPackedGlyphs = 0;
int numPackedGlyphsStopD = 0;
GLuint fontAtlasTex;
GLuint fontAtlasTexStopD;
stbtt_packedchar fontPackedChar[MAX_GLYPHS];
stbtt_packedchar fontPackedCharStopD[MAX_GLYPHS];

typedef struct {
    char *path;
    unsigned char *data;
    size_t size;
    stbtt_fontinfo info;
} LoadedFont;

static stbtt_fontinfo primaryFontInfo;
static stbtt_fontinfo secondaryFontInfo;
static unsigned char *primaryFontData;
static unsigned char *sec_data;
#ifdef FONT_GEN
    static int32_t numFallbackFonts = 0;
    static LoadedFont fallbackFonts[MAX_FALLBACK_FONTS];
    static FcConfig *fontCfg = NULL;
#endif

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
    const GlyphRange* ranges = (fontID == FONT_STOPD) ? fontRangesStopD : fontRanges;
    int32_t total_packed = (fontID == FONT_STOPD) ? numPackedGlyphsStopD : numPackedGlyphs;
    for (int32_t i = 0; i < numFontRanges; i++) {
        if (codepoint >= ranges[i].first && codepoint < ranges[i].first + ranges[i].count) {
            int32_t idx = ranges[i].startIndex + (codepoint - ranges[i].first);
            if (idx < total_packed) return idx;
        }
    }
    return -1;
}

float fixedNumberAdvanceWidth = 0.0f; // Global for fixed-width number spacing
float fixedNumberAdvanceWidthStopD = 0.0f;

static int Utf8ToCodepoint(const char **p) {
    const unsigned char *s = (const unsigned char*)*p;
    int cp = 0;
    if (s[0] < 0x80) { cp = s[0]; ++*p; }
    else if ((s[0] & 0xE0) == 0xC0) { cp = ((s[0]&0x1F)<<6)|(s[1]&0x3F); *p += 2; }
    else if ((s[0] & 0xF0) == 0xE0) { cp = ((s[0]&0x0F)<<12)|((s[1]&0x3F)<<6)|(s[2]&0x3F); *p += 3; }
    else if ((s[0] & 0xF8) == 0xF0) { cp = ((s[0]&0x07)<<18)|((s[1]&0x3F)<<12)|((s[2]&0x3F)<<6)|(s[3]&0x3F); *p += 4; }
    else { ++*p; } // invalid
    return cp;
}

// Returns the *pixel* width of the string in the current font scale.
float TextWidth(const char *utf8, int fontID) {
    if (!utf8) return 0.0f;

    float width = 0.0f;
    const char *p = utf8;
    int prevGlyph = -1;
    while (*p) {
        int cp = Utf8ToCodepoint(&p);
        int packedIdx = CodepointToPackedIndex(cp,fontID);
        float advance = 0.0f;
        if (fontID == FONT_STOPD) {
            if (packedIdx >= 0 && packedIdx < numPackedGlyphsStopD) advance = fontPackedCharStopD[packedIdx].xadvance;
        } else {
            if (packedIdx >= 0 && packedIdx < numPackedGlyphs) advance = fontPackedChar[packedIdx].xadvance;   
        }

        if (prevGlyph != -1 && advance > 0.0f) { // Kerning
            if (fontID == FONT_NORMAL && packedIdx >= 0) {
                int kern = stbtt_GetGlyphKernAdvance(&primaryFontInfo, prevGlyph, stbtt_FindGlyphIndex(&primaryFontInfo, cp));
                
                int fheight = ttSHORT(primaryFontInfo.data + primaryFontInfo.hhea + 4) - ttSHORT(primaryFontInfo.data + primaryFontInfo.hhea + 6);
                float kernScaleForPixelHeight = (float)GetScreenRelativeY(genericTextHeightFac) / (float)fheight;
                width += (float)kern * kernScaleForPixelHeight;
            } else if (fontID == FONT_STOPD && packedIdx >= 0) {
                int kern = stbtt_GetGlyphKernAdvance(&secondaryFontInfo, prevGlyph, stbtt_FindGlyphIndex(&secondaryFontInfo, cp));
                int fheight = ttSHORT(secondaryFontInfo.data + secondaryFontInfo.hhea + 4) - ttSHORT(secondaryFontInfo.data + secondaryFontInfo.hhea + 6);
                float kernScaleForPixelHeight = (float)GetScreenRelativeY(genericTextHeightFacStopD) / (float)fheight;
                width += (float)kern * kernScaleForPixelHeight;
            }
        }

        width += advance;
        prevGlyph = (packedIdx >= 0) ? stbtt_FindGlyphIndex((fontID == FONT_STOPD ? &secondaryFontInfo : &primaryFontInfo), cp) : -1;
    }
    
    return width;
}

#ifdef FONT_GEN
static LoadedFont *LoadFallbackFont(char *path) {
    for (int i = 0; i < numFallbackFonts; i++) { // Check cache first
        if (strcmp(fallbackFonts[i].path, path) == 0) return &fallbackFonts[i];
    }

    if (numFallbackFonts >= MAX_FALLBACK_FONTS) return NULL;

    int fd; int fontFileSize; uint8_t *data = OS_OpenAndAllocateFileBufferReadonly(path, &fd, &fontFileSize);
    if (fontFileSize <= 0 || data == NULL || fd < 0) { DualLogError("Could not find fallback font for %s\n", path); return NULL;}

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, data, 0)) { OS_DeallocateRAM(data, fontFileSize); return NULL; }

    LoadedFont *lf = &fallbackFonts[numFallbackFonts++];
    lf->path = strdup(path);
    lf->data = data;
    lf->size = fontFileSize;
    lf->info = info;
    return lf;
}

static int GetGlyphAndFont(uint32_t codepoint, stbtt_fontinfo **outFont, uint8_t fontID) {
    int glyph = stbtt_FindGlyphIndex((fontID == FONT_STOPD ? &secondaryFontInfo : &primaryFontInfo), codepoint);
    if (glyph) { *outFont = &primaryFontInfo; return glyph; }

    for (int i = 0; i < numFallbackFonts; i++) {
        glyph = stbtt_FindGlyphIndex(&fallbackFonts[i].info, codepoint);
        if (glyph) { *outFont = &fallbackFonts[i].info; return glyph; }
    }

    if (!fontCfg) fontCfg = FcInitLoadConfigAndFonts();
    FcCharSet *cs = FcCharSetCreate();
    FcCharSetAddChar(cs, (FcChar32)codepoint);
    FcPattern *pat = FcPatternCreate();
    FcPatternAddCharSet(pat, FC_CHARSET, cs);
    FcPatternAddInteger(pat, FC_WEIGHT, FC_WEIGHT_BOLD);
    FcConfigSubstitute(NULL, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    FcResult result;
    FcPattern *match = FcFontMatch(NULL, pat, &result); // Takes ~0.21secs
    char *fontfile = NULL;
    if (match) {
        FcChar8 *file8 = NULL;
        if (FcPatternGetString(match, FC_FILE, 0, &file8) == FcResultMatch) fontfile = strdup((char*)file8);
        FcPatternDestroy(match);
    }
    
    FcPatternDestroy(pat);
    FcCharSetDestroy(cs);
    if (!fontfile) return 0;
    
    LoadedFont *lf = LoadFallbackFont(fontfile);
    free(fontfile);
    #if defined(LINUX) || defined(ANDROID)
        malloc_trim(0);
    #endif
    if (!lf) return 0;
    
    glyph = stbtt_FindGlyphIndex(&lf->info, codepoint);
    if (glyph) { *outFont = &lf->info; return glyph; }
    return 0;
}

static void write_font_cache(const char *path, uint32_t expected_glyphs, const uint64_t file_stamp, const stbtt_packedchar *packed, uint32_t actual_packed, float fixed_advance, const unsigned char *bitmap) {
    int fd = OS_OpenWriteonly(path);
    OS_Write(fd, &expected_glyphs, sizeof(uint32_t), path);
    OS_Write(fd, &file_stamp, sizeof(uint64_t), path);
    OS_Write(fd, &actual_packed, sizeof(uint32_t), path);
    OS_Write(fd, &fixed_advance, sizeof(float), path);
    OS_Write(fd, packed, sizeof(stbtt_packedchar) * actual_packed, path);
    OS_Write(fd, bitmap, FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, path);
    OS_Close(fd);
}
#endif

static bool load_font_cache(const char *path, int32_t expected_glyphs, const uint64_t file_stamp, stbtt_packedchar *out_packed, int32_t *out_num, float *out_fixed_advance, GLuint *out_tex) {
    int fd; int fontFileSize; uint8_t *map = OS_OpenAndAllocateFileBufferReadonly(path, &fd, &fontFileSize);
    if (!map || fd <= 0 || fontFileSize <= 0) return false;
    if (fontFileSize < 20 + FONT_ATLAS_SIZE * FONT_ATLAS_SIZE) { close(fd); DualLogWarn("cache too small %s\n", path); return false; }

    const uint8_t *p = map;
    int32_t file_expected = *(int32_t*)p; p += 4;
    if (file_expected != expected_glyphs) {
        DualLogWarn("range mismatch %s (file:%u exp:%u)\n", path, file_expected, expected_glyphs);
        OS_DeallocateRAM(map, fontFileSize);
        return false;
    }

    uint64_t file_stamp_on_disk;
    memcpy(&file_stamp_on_disk, p, sizeof(uint64_t));
    p += sizeof(uint64_t);
    if (file_stamp_on_disk != file_stamp) { OS_DeallocateRAM(map, fontFileSize); DualLogWarn("Filestamp mismatch %s\n", path); return false; }

    uint32_t actual_packed = *(uint32_t*)p; p += 4;
    float fixed_advance = *(float*)p; p += 4;
    if (actual_packed > MAX_GLYPHS) { OS_DeallocateRAM(map, fontFileSize); return false; }

    memcpy(out_packed, p, sizeof(stbtt_packedchar) * actual_packed);
    *out_num = (int32_t)actual_packed;
    *out_fixed_advance = fixed_advance;
    p += sizeof(stbtt_packedchar) * actual_packed;
    glCreateTextures(GL_TEXTURE_2D, 1, out_tex);
    glTextureStorage2D(*out_tex, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(*out_tex, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, p);
    glFinish();
    OS_DeallocateRAM(map, fontFileSize);
    glTextureParameteri(*out_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(*out_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(*out_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(*out_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return true;
}

void InitFontAtlasses(void) {
    double t0 = get_time();
    DualLog("Loaded    2 fonts...");
    const char *pri_path = "./Fonts/SystemShockText.ttf";
    const char *sec_path = "./Fonts/StopD.ttf";
    int fd1; int pri_sz; primaryFontData = OS_OpenAndAllocateFileBufferReadonly(pri_path, &fd1, &pri_sz);
    int fd2; int sec_sz; sec_data = OS_OpenAndAllocateFileBufferReadonly(sec_path, &fd2, &sec_sz);
    if (fd1 <= 0 || fd2 <= 0 || pri_sz <= 0 || sec_sz <= 0 || primaryFontData == NULL || sec_data == NULL) { DualLogError("Could not open primary or secondary fonts\n"); OS_Exit(1); }
    if (!stbtt_InitFont(&primaryFontInfo, primaryFontData, 0)) { DualLogError("Primary font init failed\n"); OS_Exit(1); }
    if (!stbtt_InitFont(&secondaryFontInfo, sec_data, 0)) { DualLogError("Secondary font init failed\n"); OS_Exit(1); }

    FileFingerprint fp1, fp2;
    if (!get_file_fingerprint(pri_path, &fp1)) DualLogError("File change detection failed for %s\n", pri_path);
    if (!get_file_fingerprint(sec_path, &fp2)) DualLogError("File change detection failed for %s\n", sec_path);
    uint64_t fbx_stamp1 = file_stamp(&fp1);
    uint64_t fbx_stamp2 = file_stamp(&fp2);
    int32_t pri_expected = 0, sec_expected = 0;
    for (int i = 0; i < numFontRanges; i++) { pri_expected += fontRanges[i].count; sec_expected += fontRangesStopD[i].count; }
    const char *pri_cache = "./Fonts/SystemShockText.vfnt";
    const char *sec_cache = "./Fonts/StopD.vfnt";
    bool pri_hit = load_font_cache(pri_cache, pri_expected, fbx_stamp1, fontPackedChar, &numPackedGlyphs, &fixedNumberAdvanceWidth, &fontAtlasTex);
    bool sec_hit = load_font_cache(sec_cache, sec_expected, fbx_stamp2, fontPackedCharStopD, &numPackedGlyphsStopD, &fixedNumberAdvanceWidthStopD, &fontAtlasTexStopD);
    if (pri_hit && sec_hit) { DualLog("in %.3f s\n", get_time() - t0); return; }

    DualLog("Font ranges changed or .vfnt files not present – regenerating...\n");

#ifdef FONT_GEN
    // Primary
    unsigned char *bmp = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
    stbtt_pack_context pc;
    stbtt_PackBegin(&pc, bmp, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 0, 16, NULL);
    pc.h_oversample = 8; // STBTT_MAX_OVERSAMPLE = 8
    pc.v_oversample = 8;
    numPackedGlyphs = 0;
    float h = GetScreenRelativeY(genericTextHeightFac);
    for (int r = 0; r < numFontRanges; r++) {
        fontRanges[r].startIndex = numPackedGlyphs;
        for (int i = 0; i < fontRanges[r].count; i++) {
            if (numPackedGlyphs >= MAX_GLYPHS) break;
            uint32_t cp = fontRanges[r].first + i;
            stbtt_fontinfo *font = NULL;
            int g = GetGlyphAndFont(cp, &font, FONT_NORMAL);
            if (!g) continue;
            unsigned char *data = (font == &primaryFontInfo) ? primaryFontData
                : ((LoadedFont*)((char*)font - offsetof(LoadedFont, info)))->data;
            float height = h;
            if (font != &primaryFontInfo) height *= 1.2f;
            stbtt_PackFontRange(&pc, data, 0, height, cp, 1, &fontPackedChar[numPackedGlyphs]);
            int idx = numPackedGlyphs++;
            if (cp >= '0' && cp <= '9') fixedNumberAdvanceWidth = vmax(fixedNumberAdvanceWidth, fontPackedChar[idx].xadvance);
        }
    }
    
    free(pc.nodes);
    free(pc.pack_info);
    glCreateTextures(GL_TEXTURE_2D, 1, &fontAtlasTex);
    glTextureStorage2D(fontAtlasTex, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTex, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, bmp);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    write_font_cache(pri_cache, pri_expected, fbx_stamp1, fontPackedChar, numPackedGlyphs,fixedNumberAdvanceWidth, bmp);
    free(bmp);

    // Secondary
    bmp = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
    stbtt_pack_context pc2;
    stbtt_PackBegin(&pc2, bmp, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 0, 16, NULL);
    pc2.h_oversample = 8; // STBTT_MAX_OVERSAMPLE = 8
    pc2.v_oversample = 8;
    numPackedGlyphsStopD = 0;
    float h2 = GetScreenRelativeY(genericTextHeightFacStopD);
    for (int r = 0; r < numFontRanges; r++) {
        fontRangesStopD[r].startIndex = numPackedGlyphsStopD;
        for (int i = 0; i < fontRangesStopD[r].count; i++) {
            if (numPackedGlyphsStopD >= MAX_GLYPHS) break;
            uint32_t cp = fontRangesStopD[r].first + i;
            int g = stbtt_FindGlyphIndex(&secondaryFontInfo, cp);
            stbtt_fontinfo *font = &secondaryFontInfo;
            unsigned char *data = sec_data;
            if (!g) {
                g = GetGlyphAndFont(cp, &font, FONT_STOPD);
                if (!g) continue;
                
                data = (font == &primaryFontInfo) ? primaryFontData : ((LoadedFont*)((char*)font - offsetof(LoadedFont, info)))->data;
            }
            
            float height = h2;
            if (font != &secondaryFontInfo) height *= 1.2f;
            stbtt_PackFontRange(&pc2, data, 0, height, cp, 1, &fontPackedCharStopD[numPackedGlyphsStopD]);
            int idx = numPackedGlyphsStopD++;
            if (cp >= '0' && cp <= '9') fixedNumberAdvanceWidthStopD = vmax(fixedNumberAdvanceWidthStopD, fontPackedCharStopD[idx].xadvance);
        }
    }
    
    free(pc2.nodes);
    free(pc2.pack_info);
    glCreateTextures(GL_TEXTURE_2D, 1, &fontAtlasTexStopD);
    glTextureStorage2D(fontAtlasTexStopD, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTexStopD, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, bmp);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    write_font_cache(sec_cache, sec_expected, fbx_stamp2, fontPackedCharStopD, numPackedGlyphsStopD, fixedNumberAdvanceWidthStopD, bmp);
    free(bmp);
    #if defined(LINUX) || defined(ANDROID)
        malloc_trim(0);
    #endif
    DualLog(" regenerated in %.3f s\n", get_time() - t0);
#else
    DualLog("Font config not turned on, go set FONT_GEN at top of data_fonts.c\n");
#endif
}
