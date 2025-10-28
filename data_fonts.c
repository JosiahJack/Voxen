#include <string.h>
#include "voxen.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "External/stb_truetype.h"
#include <fontconfig/fontconfig.h>

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
GLuint fontAtlasTex;
GLuint fontAtlasTexStopD;
stbtt_packedchar fontPackedChar[MAX_GLYPHS];
stbtt_packedchar fontPackedCharStopD[MAX_GLYPHS];

typedef struct {
    int first;   // first codepoint in range
    int count;   // number of codepoints
    int startIndex; // index into fontPackedChar where this range starts
} GlyphRange;

static GlyphRange fontRanges[] = {
    {0x0020, 0x7E - 0x20+1, 0},       // ASCII
    {0x00A0, 0xFF - 0xA0+1, 95},      // Latin-1
    {0x0400, 0x04FF - 0x0400+1, 95+96}, // Cyrillic
    {0x3040, 0x30FF - 0x3040+1, 95+96+256}, // Hiragana/Katakana
    // add other ranges here
};

int numFontRanges = sizeof(fontRanges)/sizeof(fontRanges[0]);

int CodepointToPackedIndex(int codepoint) {
    for (int i = 0; i < numFontRanges; i++) {
        if (codepoint >= fontRanges[i].first && codepoint < fontRanges[i].first + fontRanges[i].count) return fontRanges[i].startIndex + (codepoint - fontRanges[i].first);
    }
    return -1; // not found
}

float fixedNumberAdvanceWidth = 0.0f; // Global for fixed-width number spacing
float fixedNumberAdvanceWidthStopD = 0.0f;

#define MAX_FALLBACK_FONTS 32

typedef struct {
    char *path;
    unsigned char *data;
    size_t size;
    stbtt_fontinfo info;
} LoadedFont;

static stbtt_fontinfo primaryFontInfo;
static unsigned char *primaryFontData;
static LoadedFont fallbackFonts[MAX_FALLBACK_FONTS];
static int numFallbackFonts = 0;

static FcConfig *fontCfg = NULL;

static char *FindFontFileForCodepoint(uint32_t codepoint) {
    if (!fontCfg) fontCfg = FcInitLoadConfigAndFonts();
    FcCharSet *cs = FcCharSetCreate();
    FcCharSetAddChar(cs, (FcChar32)codepoint);
    FcPattern *pat = FcPatternCreate();
    FcPatternAddCharSet(pat, FC_CHARSET, cs);
    FcPatternAddInteger(pat, FC_WEIGHT, FC_WEIGHT_BOLD);
    FcConfigSubstitute(NULL, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    FcResult result;
    FcPattern *match = FcFontMatch(NULL, pat, &result);
    char *file = NULL;
    if (match) {
        FcChar8 *file8 = NULL;
        if (FcPatternGetString(match, FC_FILE, 0, &file8) == FcResultMatch) file = strdup((const char*)file8);
        FcPatternDestroy(match);
    }
    
    FcPatternDestroy(pat);
    FcCharSetDestroy(cs);
    return file;
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

static int GetGlyphAndFont(uint32_t codepoint, stbtt_fontinfo **outFont) {
    int glyph = stbtt_FindGlyphIndex(&primaryFontInfo, codepoint);
    if (glyph) { *outFont = &primaryFontInfo; return glyph; }

    for (int i = 0; i < numFallbackFonts; i++) {
        glyph = stbtt_FindGlyphIndex(&fallbackFonts[i].info, codepoint);
        if (glyph) { *outFont = &fallbackFonts[i].info; return glyph; }
    }

    char *fontfile = FindFontFileForCodepoint(codepoint);
    if (!fontfile) return 0;
    
    LoadedFont *lf = LoadFallbackFont(fontfile);
    free(fontfile);
    if (!lf) return 0;

    glyph = stbtt_FindGlyphIndex(&lf->info, codepoint);
    if (glyph) { *outFont = &lf->info; return glyph; }
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

    stbtt_fontinfo secondaryFontInfo;
    if (!stbtt_InitFont(&secondaryFontInfo, secondaryFontData, 0)) {
        DualLogError("Secondary font init failed\n"); exit(1);
    }

    // ------------------------------------------------------------------------
    // Initialize fontconfig once
    if (!fontCfg) fontCfg = FcInitLoadConfigAndFonts();

    // ------------------------------------------------------------------------
    // Pack Primary Font Atlas
    unsigned char *atlasBitmap = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
    stbtt_pack_context pc;
    stbtt_PackBegin(&pc, atlasBitmap, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 0, 16, NULL);
    stbtt_PackSetOversampling(&pc, 8, 8);

    int numPackedGlyphs = 0;
    fixedNumberAdvanceWidth = 0.0f;

    float pixelHeight = GetScreenRelativeY(genericTextHeightFac);

    for (int r = 0; r < numFontRanges; r++) {
        fontRanges[r].startIndex = numPackedGlyphs;
        for (int i = 0; i < fontRanges[r].count; i++) {
            if (numPackedGlyphs >= MAX_GLYPHS) break;

            uint32_t codepoint = fontRanges[r].first + i;
            stbtt_fontinfo *font = NULL;
            int glyph = GetGlyphAndFont(codepoint, &font);
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

    int numPackedGlyphsStopD = 0;
    float fixedNumberAdvanceWidthStopD = 0.0f;
    float pixelHeightStopD = GetScreenRelativeY(genericTextHeightFacStopD); // Optional: slightly larger

    // Reuse same glyph ranges for StopD
    GlyphRange fontRangesStopD[sizeof(fontRanges)/sizeof(fontRanges[0])];
    memcpy(fontRangesStopD, fontRanges, sizeof(fontRanges));

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
                glyph = GetGlyphAndFont(codepoint, &font);
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
    if (fontCfg) { FcConfigDestroy(fontCfg); fontCfg = NULL; }
    for (int i = 0; i < numFallbackFonts; i++) {
        free(fallbackFonts[i].data);
        free(fallbackFonts[i].path);
    }
    
    numFallbackFonts = 0;
    free(primaryFontData);
    primaryFontData = NULL;
    free(secondaryFontData);
    malloc_trim(0);
    DebugRAM("end of font init (dual)");
    DualLog("font init (dual) took %f\n", get_time() - font_start_time);
}
