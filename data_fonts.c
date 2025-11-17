// data_fonts.c - Load Font Atlasses
// #include "voxen.h" limited includes
#include <malloc.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include "./External/glad/gl.h"
#include "./External/glfw3.h"
#include "voxen.h"
#include "vmath.h"
#define STBTT_ifloor(x)   ((int) vfloor(x))
#define STBTT_iceil(x)    ((int) vceil(x))
#define STBTT_sqrt(x)      vsqrtf(x)
#define STBTT_fabs(x)      vabs(x)
#define STB_TRUETYPE_IMPLEMENTATION
#include "External/stb_truetype.h"
#include <fontconfig/fontconfig.h>
int close (int filedes); // #include <unistd.h>

#define MAX_FALLBACK_FONTS 32
static char *xstrdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *p = malloc(len);
    if (p) memcpy(p, s, len);
    return p;
}
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
static int32_t numFallbackFonts = 0;
static LoadedFont fallbackFonts[MAX_FALLBACK_FONTS];
static FcConfig *fontCfg = NULL;

typedef struct {
    int32_t first;   // first codepoint in range
    int32_t count;   // number of codepoints
    int32_t startIndex; // index into fontPackedChar where this range starts
} GlyphRange;

GlyphRange fontRanges[] = {
    {0x0020, 0x7E - 0x20+1, 0},       // ASCII
    {0x00A0, 0xFF - 0xA0+1, 95},      // Latin-1
    {0x0400, 0x04FF - 0x0400+1, 95+96}, // Cyrillic
    {0x3040, 0x30FF - 0x3040+1, 95+96+256}, // Hiragana/Katakana
};

GlyphRange fontRangesStopD[] = {
    {0x0020, 0x7E - 0x20+1, 0},       // ASCII
    {0x00A0, 0xFF - 0xA0+1, 95},      // Latin-1
    {0x0400, 0x04FF - 0x0400+1, 95+96}, // Cyrillic
    {0x3040, 0x30FF - 0x3040+1, 95+96+256}, // Hiragana/Katakana
};

int32_t numFontRanges = sizeof(fontRanges)/sizeof(fontRanges[0]);

int32_t CodepointToPackedIndex(int32_t codepoint, int fontID) {
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
                float kernScaleForPixelHeight = (float) GetScreenRelativeY(genericTextHeightFac) / fheight;
                width += kern * kernScaleForPixelHeight;
            } else if (fontID == FONT_STOPD && packedIdx >= 0) {
                int kern = stbtt_GetGlyphKernAdvance(&secondaryFontInfo, prevGlyph, stbtt_FindGlyphIndex(&secondaryFontInfo, cp));
                int fheight = ttSHORT(secondaryFontInfo.data + secondaryFontInfo.hhea + 4) - ttSHORT(secondaryFontInfo.data + secondaryFontInfo.hhea + 6);
                float kernScaleForPixelHeight = (float) GetScreenRelativeY(genericTextHeightFacStopD) / fheight;
                width += kern * kernScaleForPixelHeight;
            }
        }

        width += advance;
        prevGlyph = (packedIdx >= 0) ? stbtt_FindGlyphIndex((fontID == FONT_STOPD ? &secondaryFontInfo : &primaryFontInfo), cp) : -1;
    }
    
    return width;
}

static LoadedFont *LoadFallbackFont(const char *path) {
    for (int i = 0; i < numFallbackFonts; i++) { // Check cache first
        if (strcmp(fallbackFonts[i].path, path) == 0) return &fallbackFonts[i];
    }

    if (numFallbackFonts >= MAX_FALLBACK_FONTS) return NULL;

    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *data = malloc(size);
    if (!data) { fclose(f); return NULL; }
    if (fread(data, 1, size, f) != size) { fclose(f); free(data); return NULL; }
    
    fclose(f);
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, data, 0)) { free(data); return NULL; }

    LoadedFont *lf = &fallbackFonts[numFallbackFonts++];
    lf->path = strdup(path);
    lf->data = data;
    lf->size = size;
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
        if (FcPatternGetString(match, FC_FILE, 0, &file8) == FcResultMatch) fontfile = strdup((const char*)file8);
        FcPatternDestroy(match);
    }
    
    FcPatternDestroy(pat);
    FcCharSetDestroy(cs);
    if (!fontfile) return 0;
    
    LoadedFont *lf = LoadFallbackFont(fontfile);
    free(fontfile);
    malloc_trim(0);
    if (!lf) return 0;
    
    glyph = stbtt_FindGlyphIndex(&lf->info, codepoint);
    if (glyph) { *outFont = &lf->info; return glyph; }
    return 0;
}

static void write_font_cache(const char *path, uint32_t expected_glyphs, const uint8_t ttf_md5[16], const stbtt_packedchar *packed, uint32_t actual_packed, float fixed_advance, const unsigned char *bitmap) {
    FILE *f = fopen(path, "wb");
    if (!f) { DualLogError("Failed to write font cache %s\n", path); exit(1); }

    fwrite(&expected_glyphs, 1, 4, f);
    fwrite(ttf_md5, 1, 16, f);
    fwrite(&actual_packed, 1, 4, f);
    fwrite(&fixed_advance, 1, 4, f);  // ← store
    fwrite(packed, sizeof(stbtt_packedchar), actual_packed, f);
    fwrite(bitmap, 1, FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, f);
    fclose(f);
}

static bool load_font_cache(const char *path, uint32_t expected_glyphs, const uint8_t expected_md5[16], stbtt_packedchar *out_packed, int32_t *out_num, float *out_fixed_advance, GLuint *out_tex) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return false;

    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); DualLogWarn("fstat failed %s\n", path); return false; }
    size_t sz = (size_t)st.st_size;
    if (sz < 28 + FONT_ATLAS_SIZE * FONT_ATLAS_SIZE) { close(fd); DualLogWarn("cache too small %s\n", path); return false; }

    uint8_t *map = mmap(NULL, sz, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (map == MAP_FAILED) { DualLogError("mmap failed %s\n", path); return false; }

    const uint8_t *p = map;
    uint32_t file_expected = *(uint32_t*)p; p += 4;
    if (file_expected != expected_glyphs) {
        munmap(map, sz);
        DualLogWarn("range mismatch %s (file:%u exp:%u)\n", path, file_expected, expected_glyphs);
        return false;
    }

    const uint8_t *md5 = p; p += 16;
    if (memcmp(md5, expected_md5, 16) != 0) {
        munmap(map, sz);
        DualLogWarn("MD5 mismatch %s\n", path);
        return false;
    }

    uint32_t actual_packed = *(uint32_t*)p; p += 4;
    float fixed_advance = *(float*)p; p += 4;
    if (actual_packed > MAX_GLYPHS) { munmap(map, sz); return false; }

    memcpy(out_packed, p, sizeof(stbtt_packedchar) * actual_packed);
    *out_num = (int32_t)actual_packed;
    *out_fixed_advance = fixed_advance;
    p += sizeof(stbtt_packedchar) * actual_packed;
    glCreateTextures(GL_TEXTURE_2D, 1, out_tex);
    glTextureStorage2D(*out_tex, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(*out_tex, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, p);
    glTextureParameteri(*out_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(*out_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(*out_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(*out_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    munmap(map, sz);
    return true;
}

void InitFontAtlasses(void) {
    double t0 = get_time();
    DualLog("Loaded    2 fonts...");
    const char *pri_path = "./Fonts/SystemShockText.ttf";
    const char *sec_path = "./Fonts/StopD.ttf";
    FILE *f = fopen(pri_path, "rb"); if (!f) { DualLogError("Missing %s\n", pri_path); exit(1); }
    
    fseek(f, 0, SEEK_END); size_t pri_sz = ftell(f); fseek(f, 0, SEEK_SET);
    primaryFontData = malloc(pri_sz);
    size_t read = fread(primaryFontData, 1, pri_sz, f); fclose(f);
    if (read != (size_t)pri_sz) { DualLogError("Failed to read full SystemShockText.ttf at: %s\n", pri_path); exit(1); }
    f = fopen(sec_path, "rb"); if (!f) { DualLogError("Missing %s\n", sec_path); exit(1); }
    
    fseek(f, 0, SEEK_END); size_t sec_sz = ftell(f); fseek(f, 0, SEEK_SET);
    unsigned char *sec_data = malloc(sec_sz);
    read = fread(sec_data, 1, sec_sz, f); fclose(f);
    if (read != (size_t)sec_sz) { DualLogError("Failed to read full StopD.ttf at: %s\n", sec_path); exit(1); }
    if (!stbtt_InitFont(&primaryFontInfo, primaryFontData, 0)) { DualLogError("Primary font init failed\n"); exit(1); }
    if (!stbtt_InitFont(&secondaryFontInfo, sec_data, 0)) { DualLogError("Secondary font init failed\n"); exit(1); }

    uint8_t pri_md5[16], sec_md5[16];
    md5(primaryFontData, pri_sz, pri_md5);
    md5(sec_data, sec_sz, sec_md5);
    uint32_t pri_expected = 0, sec_expected = 0;
    for (int i = 0; i < numFontRanges; i++) { pri_expected += fontRanges[i].count; sec_expected += fontRangesStopD[i].count; }
    const char *pri_cache = "./Fonts/SystemShockText.vfnt";
    const char *sec_cache = "./Fonts/StopD.vfnt";
    bool pri_hit = load_font_cache(pri_cache, pri_expected, pri_md5, fontPackedChar, &numPackedGlyphs, &fixedNumberAdvanceWidth, &fontAtlasTex);
    bool sec_hit = load_font_cache(sec_cache, sec_expected, sec_md5, fontPackedCharStopD, &numPackedGlyphsStopD, &fixedNumberAdvanceWidthStopD, &fontAtlasTexStopD);
    if (pri_hit && sec_hit) {
        free(primaryFontData); free(sec_data);
        malloc_trim(0);
        DualLog("in %.3f s\n", get_time() - t0);
        return;
    }

    DualLog("Font ranges changed or .vfnt files not present – regenerating...\n");

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
    malloc_trim(0);
    glCreateTextures(GL_TEXTURE_2D, 1, &fontAtlasTex);
    glTextureStorage2D(fontAtlasTex, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTex, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, bmp);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    write_font_cache(pri_cache, pri_expected, pri_md5, fontPackedChar, numPackedGlyphs,fixedNumberAdvanceWidth, bmp);
    free(bmp);
    malloc_trim(0);

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
    malloc_trim(0);
    glCreateTextures(GL_TEXTURE_2D, 1, &fontAtlasTexStopD);
    glTextureStorage2D(fontAtlasTexStopD, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTexStopD, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, bmp);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    write_font_cache(sec_cache, sec_expected, sec_md5, fontPackedCharStopD, numPackedGlyphsStopD, fixedNumberAdvanceWidthStopD, bmp);
    free(bmp);
    free(primaryFontData);
    free(sec_data);
    malloc_trim(0);
    DualLog(" regenerated in %.3f s\n", get_time() - t0);
}

uint32_t DecodeUTF8(const char **p) {
    const unsigned char *s = (const unsigned char *)*p;
    uint32_t codepoint = 0;
    if (*s < 0x80) {          // 1-byte ASCII
        codepoint = *s++;
    } else if ((*s & 0xE0) == 0xC0) { // 2-byte
        codepoint  = (*s & 0x1F) << 6;
        codepoint |= (s[1] & 0x3F);
        s += 2;
    } else if ((*s & 0xF0) == 0xE0) { // 3-byte
        codepoint  = (*s & 0x0F) << 12;
        codepoint |= (s[1] & 0x3F) << 6;
        codepoint |= (s[2] & 0x3F);
        s += 3;
    } else if ((*s & 0xF8) == 0xF0) { // 4-byte
        codepoint  = (*s & 0x07) << 18;
        codepoint |= (s[1] & 0x3F) << 12;
        codepoint |= (s[2] & 0x3F) << 6;
        codepoint |= (s[3] & 0x3F);
        s += 4;
    } else {
        s++; // invalid byte
    }
    *p = (const char *)s;
    return codepoint;
}
