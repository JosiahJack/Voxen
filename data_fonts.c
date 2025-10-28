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
GLuint fontAtlasTex;
GLuint fontAtlasTexStopD;
stbtt_packedchar fontPackedChar[MAX_GLYPHS];
stbtt_packedchar fontPackedCharStopD[MAX_GLYPHS];
int numPackedGlyphs = 0;
float textTexelWidth;
float textTexelWidthStopD;

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
    const char* filename = "./Fonts/SystemShockText.ttf";
    FILE *f = fopen(filename, "rb");
    if (!f) { DualLogError("Failed to open font %s\n", filename); exit(1); }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    primaryFontData = malloc(size);
    if (!primaryFontData) { DualLogError("OOM\n"); exit(1); }
    if (fread(primaryFontData, 1, size, f) != size) { DualLogError("Read failed\n"); exit(1); }
    
    fclose(f);
    if (!stbtt_InitFont(&primaryFontInfo, primaryFontData, 0)) { DualLogError("Font init failed\n"); exit(1); }

    unsigned char *atlasBitmap = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
    stbtt_pack_context pc;
    stbtt_PackBegin(&pc, atlasBitmap, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 0, 16, NULL);
    stbtt_PackSetOversampling(&pc, 8, 8);
    numPackedGlyphs = 0;
    for (int r = 0; r < numFontRanges; r++) {
        fontRanges[r].startIndex = numPackedGlyphs;
        for (int i = 0; i < fontRanges[r].count; i++) {
            if (numPackedGlyphs >= MAX_GLYPHS) break;

            uint32_t codepoint = fontRanges[r].first + i;
            stbtt_fontinfo *font = NULL;
            int glyph = GetGlyphAndFont(codepoint, &font);
            if (!glyph) continue;

            float pixelHeight = GetScreenRelativeY(genericTextHeightFac);
            unsigned char *fontData = (font == &primaryFontInfo) ? primaryFontData
                : ((LoadedFont*)((char*)font - offsetof(LoadedFont, info)))->data;

            // Scale fallback fonts
            if (font != &primaryFontInfo) {
                float fallbackScale = 1.2f; // slightly bigger than primary
                pixelHeight *= fallbackScale;

                // Compute baseline offset
                int ascent, descent, lineGap;
                stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);
                float scale = stbtt_ScaleForPixelHeight(font, pixelHeight);
                float baselineOffset = scale * (ascent - 2); // lift up by ~2px
                fontPackedChar[numPackedGlyphs].y0 -= (int)baselineOffset;
                fontPackedChar[numPackedGlyphs].y1 -= (int)baselineOffset;
            }

            stbtt_PackFontRange(&pc, fontData, 0, pixelHeight, codepoint, 1, &fontPackedChar[numPackedGlyphs]);
            int idx = numPackedGlyphs;
            numPackedGlyphs++;
            if (codepoint >= '0' && codepoint <= '9') {
                float advance = fontPackedChar[idx].xadvance;
                if (advance > fixedNumberAdvanceWidth) fixedNumberAdvanceWidth = advance;
            }
        }
    }

    stbtt_PackEnd(&pc);
    glCreateTextures(GL_TEXTURE_2D, 1, &fontAtlasTex);
    glTextureStorage2D(fontAtlasTex, 1, GL_R8, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE);
    glTextureSubImage2D(fontAtlasTex, 0, 0, 0, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, GL_RED, GL_UNSIGNED_BYTE, atlasBitmap);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(fontAtlasTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    free(atlasBitmap);
    if (fontCfg) { FcConfigDestroy(fontCfg); fontCfg = NULL; }
    for (int i = 0; i < numFallbackFonts; i++) {
        free(fallbackFonts[i].data);
        free(fallbackFonts[i].path);
    }
    numFallbackFonts = 0;
    free(primaryFontData);
    primaryFontData = NULL;
    malloc_trim(0);
    textTexelWidth = 1.0f / (float)FONT_ATLAS_SIZE;
    DebugRAM("end of font init");
    DualLog("font init took %f\n",get_time() - font_start_time);
}
