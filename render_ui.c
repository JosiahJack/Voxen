#include <malloc.h>
#include <stdint.h>
#include <stdbool.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "External/stb_truetype.h"
#include <GL/glew.h>
#include "voxen.h"
#include "citadel.h"

// Cursor
bool cursorVisible = false;
int32_t cursorPosition_x = 680, cursorPosition_y = 384;

// Text
GLuint textShaderProgram;
GLuint textVAO, textVBO;
Color textColors[6] = {
    {         1.0f,         1.0f,          1.0f, 1.0f}, // 0 White 1.0f, 1.0f, 1.0f
    { 0.890196078f, 0.874509804f,          0.0f, 1.0f}, // 1 Yellow 0.8902f, 0.8745f, 0f
    { 0.623529412f, 0.611764706f,          0.0f, 1.0f}, // 2 Dark Yellow 0.8902f * 0.7f, 0.8745f * 0.7f, 0f
    { 0.372549020f, 0.654901961f,  0.168627451f, 1.0f}, // 3 Green 0.3725f, 0.6549f, 0.1686f
    { 0.917647059f, 0.137254902f,  0.168627451f, 1.0f}, // 4 Red 0.9176f, 0.1373f, 0.1686f
    {         1.0f, 0.498039216f,          0.0f, 1.0f}  // 5 Orange 1f, 0.498f, 0f
};

float uiOrthoProjection[16];
char uiTextBuffer[TEXT_BUFFER_SIZE];
GLint projectionLoc_text = -1, textColorLoc_text = -1, textTextureLoc_text = -1, texelSizeLoc_text = -1; // uniform locations

bool consoleActive = false;
char consoleEntryText[TEXT_BUFFER_SIZE] = "Enter a command...";
char statusText[TEXT_BUFFER_SIZE];
int statusTextLengthWithoutNullTerminator = 6;
float statusTextDecayFinished = 0.0f;
float genericTextHeightFac = 0.025f;
int32_t currentEntryLength = 0;

// Diagnostics
double lastFrameSecCountTime = 0.00;
uint32_t lastFrameSecCount = 0;
uint32_t framesPerLastSecond = 0;
uint32_t worstFPS = UINT32_MAX;
double screenshotTimeout = 0.0;
double time_PhysicsStep = 0.0;

#define FONT_ATLAS_SIZE 2048
#define MAX_GLYPHS 8192      // Rough estimate for all ranges

GLuint fontAtlasTex;
stbtt_packedchar fontPackedChar[MAX_GLYPHS];
int numPackedGlyphs = 0;
float textTexelWidth;

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
static int numFontRanges = sizeof(fontRanges)/sizeof(fontRanges[0]);

static int CodepointToPackedIndex(int codepoint) {
    for (int i = 0; i < numFontRanges; i++) {
        if (codepoint >= fontRanges[i].first && codepoint < fontRanges[i].first + fontRanges[i].count) {
            return fontRanges[i].startIndex + (codepoint - fontRanges[i].first);
        }
    }
    return -1; // not found
}

float fixedNumberAdvanceWidth = 0.0f; // Global for fixed-width number spacing

void InitFontAtlasses() {
    const char* filename = "./Fonts/SystemShockText.ttf";
    FILE *f = fopen(filename, "rb");
    if (!f) { DualLogError("Failed to open font %s\n", filename); exit(1); }

    fseek(f, 0, SEEK_END);
    size_t ttf_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *ttf_buffer = malloc(ttf_size);
    size_t readSize = fread(ttf_buffer, 1, ttf_size, f);
    if (readSize != ttf_size) { DualLogError("Could not read font %s\n", filename); exit(1); }
    
    fclose(f);
    unsigned char *atlasBitmap = calloc(FONT_ATLAS_SIZE * FONT_ATLAS_SIZE, 1);
    stbtt_pack_context pc;
    if (!stbtt_PackBegin(&pc, atlasBitmap, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, 0, 4, NULL)) { DualLogError("Failed to initialize font packer\n"); exit(1); }
    
    stbtt_PackSetOversampling(&pc, 2, 2);
    numPackedGlyphs = 0;
    for (int r = 0; r < numFontRanges; r++) {
        fontRanges[r].startIndex = numPackedGlyphs;
        for (int i = 0; i < fontRanges[r].count; i++) {
            if (numPackedGlyphs >= MAX_GLYPHS) break;
            stbtt_PackFontRange(&pc, ttf_buffer, 0, GetScreenRelativeY(genericTextHeightFac), fontRanges[r].first + i, 1, &fontPackedChar[numPackedGlyphs]);
            numPackedGlyphs++;
        }
    }

    // Calculate fixed advance width for digits (0-9)
    fixedNumberAdvanceWidth = 0.0f;
    for (int codepoint = '0'; codepoint <= '9'; codepoint++) {
        int idx = CodepointToPackedIndex(codepoint);
        if (idx >= 0) {
            float advance = fontPackedChar[idx].xadvance;
            if (advance > fixedNumberAdvanceWidth) {
                fixedNumberAdvanceWidth = advance;
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
    free(ttf_buffer);
    free(atlasBitmap);
    textTexelWidth = 1.0f / (float)FONT_ATLAS_SIZE;
    DebugRAM("end of font init");
}

bool inventoryModeWasActivePriorToConsole = false;
void ToggleConsole(void) {
    if (!consoleActive) inventoryModeWasActivePriorToConsole = inventoryMode;
    consoleActive = !consoleActive; // Tilde
    if (consoleActive) inventoryMode = true;
    else if (!inventoryModeWasActivePriorToConsole && inventoryMode) {
        inventoryMode = false;
    } 
}

bool CursorVisible(void) {
    return (inventoryMode || menuActive || gamePaused);
}

void ProcessConsoleCommand(const char* command) {
    if (strcmp(command, "noclip") == 0) {
        noclip = !noclip;
        CenterStatusPrint("Noclip %s", noclip ? "enabled" : "disabled");
        ToggleConsole();
    }  else if (strcmp(command, "quit") == 0) {
        EnqueueEvent_Simple(EV_QUIT);
    } else {
        CenterStatusPrint("Unknown command: %s", command);
    }
    
    consoleEntryText[0] = '\0'; // Clear the input
    currentEntryLength = 0;
}

void ConsoleEmulator(int32_t scancode) {
    if (scancode == SDL_SCANCODE_U && keys[SDL_SCANCODE_LCTRL]) {
        consoleEntryText[0] = '\0'; // Clear the input
        currentEntryLength = 0;
        return;
    }
    
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) { // Handle alphabet keys (SDL scancodes 4-29 correspond to 'a' to 'z')
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) { // Ensure we don't overflow the buffer
            char c = 'a' + (scancode - SDL_SCANCODE_A); // Map scancode to character
            consoleEntryText[currentEntryLength] = c;
            consoleEntryText[currentEntryLength + 1] = '\0'; // Null-terminate
            currentEntryLength++;
        }
    } else if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0) { // Handle number keys (SDL scancodes 30-39 correspond to '0' to '9')
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) {
            char c;
            if (scancode == SDL_SCANCODE_0) c = '0'; // Special case for '0'
            else c = '1' + (scancode - SDL_SCANCODE_1); // Map 1-9 to '1'-'9'

            consoleEntryText[currentEntryLength] = c;
            consoleEntryText[currentEntryLength + 1] = '\0'; // Null-terminate
            currentEntryLength++;
        }
    } else if (scancode == SDL_SCANCODE_BACKSPACE && currentEntryLength > 0) { // Handle backspace
        currentEntryLength--;
        consoleEntryText[currentEntryLength] = '\0'; // Null-terminate
    } else if (scancode == SDL_SCANCODE_SPACE) { // Handle other keys as needed (e.g., enter, space, etc.)
        if (currentEntryLength < (TEXT_BUFFER_SIZE - 1)) {
            consoleEntryText[currentEntryLength] = ' ';
            consoleEntryText[currentEntryLength + 1] = '\0';
            currentEntryLength++;
        }
    } else if (scancode == SDL_SCANCODE_RETURN) {
        // Handle command execution or clear the console
        DualLog("Console command: %s\n", consoleEntryText);
        ProcessConsoleCommand(consoleEntryText);
    }
}

static uint32_t DecodeUTF8(const char **p) {
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

float textVertexData[4096]; // Reusable buffer for text vertices.  Most text only needs ~3000

void RenderText(float x, float y, const char *text, int32_t colorIdx) {
    glUseProgram(textShaderProgram);
    glProgramUniformMatrix4fv(textShaderProgram, projectionLoc_text, 1, GL_FALSE, uiOrthoProjection);
    glProgramUniform4f(textShaderProgram, textColorLoc_text, textColors[colorIdx].r, textColors[colorIdx].g, textColors[colorIdx].b, textColors[colorIdx].a);
    glBindTextureUnit(6, fontAtlasTex);
    glProgramUniform2f(textShaderProgram, texelSizeLoc_text, 1.0f / (float)FONT_ATLAS_SIZE, 1.0f / (float)FONT_ATLAS_SIZE);
    glProgramUniform1i(textShaderProgram, textTextureLoc_text, 6);
    glBindVertexArray(textVAO);

    // Batch vertices for all glyphs
    size_t vertexCount = 0;
    const char* p = text;
    float xpos = x, ypos = y + GetScreenRelativeY(0.0211f);
    float lineSpacing = GetScreenRelativeY(0.03f); // Match RenderUI
    stbtt_aligned_quad q;
    int characterCount = 0;
    while (*p) {
        uint32_t codepoint = DecodeUTF8(&p);
        characterCount++;
        if (codepoint == '\n' || characterCount > 120) { // Handle newline
            xpos = x;
            ypos += lineSpacing;
            characterCount = 0;
            continue;
        }
        int idx = CodepointToPackedIndex(codepoint);
        if (idx < 0) continue;
        stbtt_GetPackedQuad(fontPackedChar, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, idx, &xpos, &ypos, &q, 1);
        float borderWidthPixels = 2.0f * textTexelWidth;
        float borderTexels = borderWidthPixels * textTexelWidth;
        float textVertices[24] = {
            // Triangle 1: Bottom-left, Top-right, Top-left
            q.x0 - borderWidthPixels, q.y0 - borderWidthPixels, q.s0 - borderTexels, q.t0 - borderTexels,
            q.x1 + borderWidthPixels, q.y1 + borderWidthPixels, q.s1 + borderTexels, q.t1 + borderTexels,
            q.x1 + borderWidthPixels, q.y0 - borderWidthPixels, q.s1 + borderTexels, q.t0 - borderTexels,
            
            // Triangle 2: Bottom-left, Top-right, Bottom-right
            q.x0 - borderWidthPixels, q.y0 - borderWidthPixels, q.s0 - borderTexels, q.t0 - borderTexels,
            q.x0 + borderWidthPixels, q.y1 + borderWidthPixels, q.s0 - borderTexels, q.t1 + borderTexels,
            q.x1 + borderWidthPixels, q.y1 + borderWidthPixels, q.s1 + borderTexels, q.t1 + borderTexels
        };
        memcpy(textVertexData + vertexCount * 24, textVertices, sizeof(textVertices));
        vertexCount++;
        // Use fixed width for digits
        if (codepoint >= '0' && codepoint <= '9') {
            xpos = q.x0 + fixedNumberAdvanceWidth;
        }
    }
    if (vertexCount > 0) {
        glNamedBufferData(textVBO, vertexCount * 24 * sizeof(float), textVertexData, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount * 6);
        drawCallsRenderedThisFrame++;
        verticesRenderedThisFrame += vertexCount * 24;
    }
    
    glBindVertexArray(0);
}

void RenderFormattedText(int32_t x, int32_t y, uint32_t color, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(uiTextBuffer, TEXT_BUFFER_SIZE, format, args);
    va_end(args);
    RenderText(x, y, uiTextBuffer, color);
}

int32_t GetScreenRelativeX(float percentage) { return (int32_t)floorf((float)screen_width * percentage); }
int32_t GetScreenRelativeY(float percentage) { return (int32_t)floorf((float)screen_height * percentage); }
int32_t GetTextHCenter(int32_t pointToCenterOn, int32_t numCharactersNoNullTerminator) {
    float characterWidth = genericTextHeightFac * 0.75f * screen_height; // Measured some and found between 0.6 and 0.82 in Gimp for width to height ratio.
    return (pointToCenterOn - (int32_t)( (float)numCharactersNoNullTerminator * 0.5f) * characterWidth); // This could be mid character ;)
}

void CenterStatusPrint(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    statusTextLengthWithoutNullTerminator = vsnprintf(statusText, TEXT_BUFFER_SIZE, fmt, args);
    va_end(args);
    DualLog("%s\n",statusText);
    statusTextDecayFinished = get_time() + 2.0f; // 2 second decay time before text dissappears.
}

static const char* debugViewNames[] = {
    "standard render", // 0
    "unlit",           // 1
    "surface normals", // 2
    "depth",           // 3
    "shadows",         // 4
    "worldpos",        // 5
    "lightview",       // 6
    "reflections"      // 7
};

void RenderUI(void) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    uint32_t drawCallsNormal = drawCallsRenderedThisFrame;

    // UI Common References
    int screenCenterX = screen_width / 2;
    int screenCenterY = screen_height / 2;
    int32_t lineSpacing = GetScreenRelativeY(genericTextHeightFac * 1.0f);
    int32_t characterWidth = (int32_t)floorf(genericTextHeightFac * 0.75f * screen_height);
    int32_t characterHeight = (int32_t)floorf(genericTextHeightFac * screen_height);
    int32_t characterWidthHalf = characterWidth * 0.5f;
    int32_t characterHeightHalf = characterHeight * 0.5f;
    
    // 8. Render UI Images
    //    Cursor
    if (gamePaused) RenderFormattedText(screenCenterX - (genericTextHeightFac * lineSpacing), screenCenterY - GetScreenRelativeY(0.30f), TEXT_RED, "PAUSED");
    
    // 9. Render UI Text;
    int32_t debugTextStartY = GetScreenRelativeY(0.0583333f);
    int32_t leftPad = GetScreenRelativeX(0.0125f);
//     RenderFormattedText(leftPad, debugTextStartY, TEXT_WHITE, "x: %.4f, y: %.4f, z: %.4f", cam_x, cam_y, cam_z);
    RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 1), TEXT_WHITE, "cam yaw: %.2f, cam pitch: %.2f, cam roll: %.2f", cam_yaw, cam_pitch, cam_roll);
//     RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 2), TEXT_WHITE, "Peak frame queue count: %d", maxEventCount_debug);
    RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 3), TEXT_WHITE, "DebugView: %d (%s), DebugValue: %d", debugView, debugViewNames[debugView], debugValue);
//     RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 4), TEXT_WHITE, "Num cells: %d, Player cell(%d):: x: %d, y: %d, z: %d", numCellsVisible, playerCellIdx, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z);
//     RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 5), TEXT_WHITE, "Character set test: ! % ^ ö ü é ó る。エレベーターでレベルを離れよ низкой гравитацией");
    if (consoleActive) RenderFormattedText(leftPad, 0, TEXT_WHITE, "] %s",consoleEntryText);
    if (statusTextDecayFinished > current_time) RenderFormattedText(GetTextHCenter(screenCenterX,statusTextLengthWithoutNullTerminator), screenCenterY - GetScreenRelativeY(0.30f + (genericTextHeightFac * 2.0f)), TEXT_WHITE, "%s",statusText);

    if (CursorVisible()) RenderFormattedText(cursorPosition_x - characterWidthHalf, cursorPosition_y - characterHeightHalf, TEXT_RED, "+");
    else RenderFormattedText(screenCenterX - characterWidthHalf, screenCenterY - characterHeightHalf, TEXT_GREEN, "+");

    // Frame stats
    double time_now = get_time();
    drawCallsRenderedThisFrame++; // Add one more for this text render ;)
    RenderFormattedText(leftPad, debugTextStartY - lineSpacing, TEXT_WHITE, "Frame time: %.6f (FPS: %d), CPU time: %.6f, Draw calls: %d [Geo %d, UI %d], Verts: %d, Worst FPS: %d",
                        (time_now - last_time) * 1000.0f,framesPerLastSecond, cpuTime * 1000.0f,drawCallsRenderedThisFrame,drawCallsNormal, drawCallsRenderedThisFrame - drawCallsNormal,verticesRenderedThisFrame,worstFPS);
    last_time = time_now;
    if ((time_now - lastFrameSecCountTime) >= 1.00) {
        lastFrameSecCountTime = time_now;
        framesPerLastSecond = globalFrameNum - lastFrameSecCount;
        if (framesPerLastSecond < worstFPS && globalFrameNum > 2000) worstFPS = framesPerLastSecond; // After startup, keep track of worst framerate seen.
        lastFrameSecCount = globalFrameNum;
    }
    
    if (keys[SDL_SCANCODE_F12]) {
        if (time_now > screenshotTimeout) {
            Screenshot();
            screenshotTimeout = time_now + 1.0; // Prevent saving more than 1 per second for sanity purposes.
        }
    }
    
    glUseProgram(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}
