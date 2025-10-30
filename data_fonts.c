#include <string.h>
#include "voxen.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "External/stb_truetype.h"
#ifdef LOAD_LOCALIZATION_FONTS
#include <fontconfig/fontconfig.h>
#endif

#ifdef LOAD_LOCALIZATION_FONTS
#define MAX_FALLBACK_FONTS 32
static char *xstrdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *p = malloc(len);
    if (p) memcpy(p, s, len);
    return p;
}
#define strdup xstrdup
#endif // LOAD_LOCALIZATION_FONTS
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
#ifdef LOAD_LOCALIZATION_FONTS
static LoadedFont fallbackFonts[MAX_FALLBACK_FONTS];
static FcConfig *fontCfg = NULL;
#endif

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
    for (int32_t i = 0; i < numFontRanges; i++) {
        if (codepoint >= ranges[i].first && codepoint < ranges[i].first + ranges[i].count)
            return ranges[i].startIndex + (codepoint - ranges[i].first);
    }
    return -1;
}

float fixedNumberAdvanceWidth = 0.0f; // Global for fixed-width number spacing
float fixedNumberAdvanceWidthStopD = 0.0f;

float GetTextAdvanceAmount(int32_t packedIdx, uint8_t font) {
    if (font == FONT_STOPD) {
        if (packedIdx < 0 || packedIdx >= numPackedGlyphsStopD) return 0.0f;
        return fontPackedCharStopD[packedIdx].xadvance;
    } else {
        if (packedIdx < 0 || packedIdx >= numPackedGlyphs) return 0.0f;
        return fontPackedChar[packedIdx].xadvance;
    }
}

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
                width += kern * stbtt_ScaleForPixelHeight(&primaryFontInfo, GetScreenRelativeY(genericTextHeightFac));
            } else if (fontID == FONT_STOPD && packedIdx >= 0) {
                int kern = stbtt_GetGlyphKernAdvance(&secondaryFontInfo, prevGlyph, stbtt_FindGlyphIndex(&secondaryFontInfo, cp));
                width += kern * stbtt_ScaleForPixelHeight(&secondaryFontInfo, GetScreenRelativeY(genericTextHeightFacStopD));
            }
        }

        width += advance;
        prevGlyph = (packedIdx >= 0) ? stbtt_FindGlyphIndex((fontID == FONT_STOPD ? &secondaryFontInfo : &primaryFontInfo), cp) : -1;
    }
    
    return width;
}

#ifdef LOAD_LOCALIZATION_FONTS
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
#endif

static int GetGlyphAndFont(uint32_t codepoint, stbtt_fontinfo **outFont, uint8_t fontID) {
    int glyph = stbtt_FindGlyphIndex((fontID == FONT_STOPD ? &secondaryFontInfo : &primaryFontInfo), codepoint);
    if (glyph) { *outFont = &primaryFontInfo; return glyph; }

#ifdef LOAD_LOCALIZATION_FONTS
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
    if (!lf) return 0;
    
    glyph = stbtt_FindGlyphIndex(&lf->info, codepoint);
    if (glyph) { *outFont = &lf->info; return glyph; }
#endif
    return 0;
}

void InitFontAtlasses(void) {
    double font_start_time = get_time();

    // ------------------------------------------------------------------------
    // Load Primary Font: SystemShockText.ttf
    const char* primaryFilename = "./Fonts/SystemShockText.ttf";
    FILE *f = fopen(primaryFilename, "rb");
    if (!f) { DualLogError("Failed to open primary font %s\n", primaryFilename); exit(1); }
    fseek(f, 0, SEEK_END);
    size_t primarySize = ftell(f);
    fseek(f, 0, SEEK_SET);
    primaryFontData = malloc(primarySize);
    if (!primaryFontData) { DualLogError("OOM (primary font)\n"); exit(1); }
    if (fread(primaryFontData, 1, primarySize, f) != primarySize) { DualLogError("Read failed (primary)\n"); exit(1); }
    fclose(f);

    if (!stbtt_InitFont(&primaryFontInfo, primaryFontData, 0)) {
        DualLogError("Primary font init failed\n"); exit(1);
    }

    // ------------------------------------------------------------------------
    // Load Secondary Font: StopD.ttf
    const char* secondaryFilename = "./Fonts/StopD.ttf";
    FILE *f2 = fopen(secondaryFilename, "rb");
    if (!f2) { DualLogError("Failed to open secondary font %s\n", secondaryFilename); exit(1); }
    fseek(f2, 0, SEEK_END);
    size_t secondarySize = ftell(f2);
    fseek(f2, 0, SEEK_SET);
    unsigned char *secondaryFontData = malloc(secondarySize);
    if (!secondaryFontData) { DualLogError("OOM (secondary font)\n"); exit(1); }
    if (fread(secondaryFontData, 1, secondarySize, f2) != secondarySize) { DualLogError("Read failed (secondary)\n"); exit(1); }
    fclose(f2);

    if (!stbtt_InitFont(&secondaryFontInfo, secondaryFontData, 0)) {
        DualLogError("Secondary font init failed\n"); exit(1);
    }

    // ------------------------------------------------------------------------
    // Initialize fontconfig once
#ifdef LOAD_LOCALIZATION_FONTS
    if (!fontCfg) fontCfg = FcInitLoadConfigAndFonts();
#endif
    
    // ------------------------------------------------------------------------
    // Pack Primary Font Atlas
    unsigned char *atlasBitmap = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
    stbtt_pack_context pc;
    stbtt_PackBegin(&pc, atlasBitmap, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 0, 16, NULL);
    stbtt_PackSetOversampling(&pc, 8, 8);
    numPackedGlyphs = 0;
    fixedNumberAdvanceWidth = 0.0f;
    float pixelHeight = GetScreenRelativeY(genericTextHeightFac);
    for (int r = 0; r < numFontRanges; r++) {
        fontRanges[r].startIndex = numPackedGlyphs;
        for (int i = 0; i < fontRanges[r].count; i++) {
            if (numPackedGlyphs >= MAX_GLYPHS) break;

            uint32_t codepoint = fontRanges[r].first + i;
            stbtt_fontinfo *font = NULL;
            int glyph = GetGlyphAndFont(codepoint, &font, FONT_NORMAL);
            if (!glyph) continue;

            float height = pixelHeight;
            unsigned char *fontData = (font == &primaryFontInfo) ? primaryFontData
                : ((LoadedFont*)((char*)font - offsetof(LoadedFont, info)))->data;

            // Scale fallback fonts
            if (font != &primaryFontInfo) {
                float fallbackScale = 1.2f;
                height *= fallbackScale;

                int ascent, descent, lineGap;
                stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);
                float scale = stbtt_ScaleForPixelHeight(font, height);
                float baselineOffset = scale * (ascent - 2);
                fontPackedChar[numPackedGlyphs].y0 -= (int)baselineOffset;
                fontPackedChar[numPackedGlyphs].y1 -= (int)baselineOffset;
            }

            stbtt_PackFontRange(&pc, fontData, 0, height, codepoint, 1, &fontPackedChar[numPackedGlyphs]);
            int idx = numPackedGlyphs++;
            if (codepoint >= '0' && codepoint <= '9') {
                float advance = fontPackedChar[idx].xadvance;
                if (advance > fixedNumberAdvanceWidth) fixedNumberAdvanceWidth = advance;
            }
        }
    }
    stbtt_PackEnd(&pc);

    // Upload Primary Atlas
    glCreateTextures(GL_TEXTURE_2D, 1, &fontAtlasTex);
    glTextureStorage2D(fontAtlasTex, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTex, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, atlasBitmap);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    free(atlasBitmap);
    malloc_trim(0);

    // ------------------------------------------------------------------------
    // Pack Secondary Font Atlas: StopD.ttf
    unsigned char *atlasBitmapStopD = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
    stbtt_pack_context pcStopD;
    stbtt_PackBegin(&pcStopD, atlasBitmapStopD, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 0, 16, NULL);
    stbtt_PackSetOversampling(&pcStopD, 8, 8);  // You can adjust per-font oversampling
    numPackedGlyphsStopD = 0;
    float fixedNumberAdvanceWidthStopD = 0.0f;
    float pixelHeightStopD = GetScreenRelativeY(genericTextHeightFacStopD); // Optional: slightly larger
    for (int r = 0; r < numFontRanges; r++) {
        fontRangesStopD[r].startIndex = numPackedGlyphsStopD;
        for (int i = 0; i < fontRangesStopD[r].count; i++) {
            if (numPackedGlyphsStopD >= MAX_GLYPHS) break;

            uint32_t codepoint = fontRangesStopD[r].first + i;

            // Try secondary font first
            int glyph = stbtt_FindGlyphIndex(&secondaryFontInfo, codepoint);
            stbtt_fontinfo *font = &secondaryFontInfo;
            unsigned char *fontData = secondaryFontData;

            if (!glyph) {
                // Fallback to primary or system fonts
                glyph = GetGlyphAndFont(codepoint, &font, FONT_STOPD);
                if (!glyph) continue;
                fontData = (font == &primaryFontInfo) ? primaryFontData
                    : ((LoadedFont*)((char*)font - offsetof(LoadedFont, info)))->data;
            }

            float height = pixelHeightStopD;
            if (font != &secondaryFontInfo) {
                float fallbackScale = 1.2f;
                height *= fallbackScale;

                int ascent, descent, lineGap;
                stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);
                float scale = stbtt_ScaleForPixelHeight(font, height);
                float baselineOffset = scale * (ascent - 2);
                fontPackedCharStopD[numPackedGlyphsStopD].y0 -= (int)baselineOffset;
                fontPackedCharStopD[numPackedGlyphsStopD].y1 -= (int)baselineOffset;
            }

            stbtt_PackFontRange(&pcStopD, fontData, 0, height, codepoint, 1, &fontPackedCharStopD[numPackedGlyphsStopD]);
            int idx = numPackedGlyphsStopD++;
            if (codepoint >= '0' && codepoint <= '9') {
                float advance = fontPackedCharStopD[idx].xadvance;
                if (advance > fixedNumberAdvanceWidthStopD) fixedNumberAdvanceWidthStopD = advance;
            }
        }
    }
    stbtt_PackEnd(&pcStopD);

    // Upload Secondary Atlas
    glCreateTextures(GL_TEXTURE_2D, 1, &fontAtlasTexStopD);
    glTextureStorage2D(fontAtlasTexStopD, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTexStopD, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, atlasBitmapStopD);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTexStopD, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    free(atlasBitmapStopD);

    // Store fixed width for numbers in StopD (optional global)
    // You can expose this as `fixedNumberAdvanceWidthStopD` if needed

    // ------------------------------------------------------------------------
    // Cleanup
#ifdef LOAD_LOCALIZATION_FONTS
    if (fontCfg) { FcConfigDestroy(fontCfg); fontCfg = NULL; }
    for (int i = 0; i < numFallbackFonts; i++) {
        free(fallbackFonts[i].data);
        free(fallbackFonts[i].path);
    }
#endif
    free(primaryFontData);
    primaryFontData = NULL;
    free(secondaryFontData);
    malloc_trim(0);
    DebugRAM("end of font init");
    DualLog("Loading fonts(%d)... took %f\n", numFallbackFonts + 2, get_time() - font_start_time);
}
