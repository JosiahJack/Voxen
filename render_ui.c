#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include "voxen.h"
#include "citadel.h"

// Cursor
bool cursorVisible = false;
int32_t cursorPosition_x = 680, cursorPosition_y = 384;

// Text
GLuint textShaderProgram;
TTF_Font* font = NULL;
GLuint textVAO, textVBO;
SDL_Color textColWhite = {255, 255, 255, 255};
SDL_Color textColRed = {255, 0, 0, 255};
SDL_Color textColGreen = {20, 255, 30, 255};
SDL_Color textColors[6] = {
    {255, 255, 255, 255}, // 0 White 1.0f, 1.0f, 1.0f
    {227, 223,   0, 255}, // 1 Yellow 0.8902f, 0.8745f, 0f
    {159, 156,   0, 255}, // 2 Dark Yellow 0.8902f * 0.7f, 0.8745f * 0.7f, 0f
    { 95, 167,  43, 255}, // 3 Green 0.3725f, 0.6549f, 0.1686f
    {234,  35,  43, 255}, // 4 Red 0.9176f, 0.1373f, 0.1686f
    {255, 127,   0, 255}  // 5 Orange 1f, 0.498f, 0f
};

float uiOrthoProjection[16];
char uiTextBuffer[TEXT_BUFFER_SIZE];
GLint projectionLoc_text = -1, textColorLoc_text = -1, textTextureLoc_text = -1, texelSizeLoc_text = -1; // uniform locations

bool consoleActive = false;
#define STATUS_TEXT_MAX_LENGTH 1024
char consoleEntryText[STATUS_TEXT_MAX_LENGTH] = "Enter a command...";
char statusText[STATUS_TEXT_MAX_LENGTH];
int statusTextLengthWithoutNullTerminator = 6;
float statusTextDecayFinished = 0.0f;
float genericTextHeightFac = 0.02f;
int32_t currentEntryLength = 0;

// Diagnostics
double lastFrameSecCountTime = 0.00;
uint32_t lastFrameSecCount = 0;
uint32_t framesPerLastSecond = 0;
uint32_t worstFPS = UINT32_MAX;
double screenshotTimeout = 0.0;
double time_PhysicsStep = 0.0;

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
        if (currentEntryLength < (STATUS_TEXT_MAX_LENGTH - 1)) { // Ensure we don't overflow the buffer
            char c = 'a' + (scancode - SDL_SCANCODE_A); // Map scancode to character
            consoleEntryText[currentEntryLength] = c;
            consoleEntryText[currentEntryLength + 1] = '\0'; // Null-terminate
            currentEntryLength++;
        }
    } else if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0) { // Handle number keys (SDL scancodes 30-39 correspond to '0' to '9')
        if (currentEntryLength < (STATUS_TEXT_MAX_LENGTH - 1)) {
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
        if (currentEntryLength < (STATUS_TEXT_MAX_LENGTH - 1)) {
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

// Renders text at x,y coordinates specified using pointer to the string array.
void RenderText(float x, float y, const char *text, int32_t colorIdx) {
    glDisable(GL_CULL_FACE); // Disable backface culling
    if (!font) { DualLogError("Font is NULL\n"); return; }
    if (!text) { DualLogError("Text is NULL\n"); return; }
    
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColors[colorIdx]);
    if (!surface) { DualLogError("TTF_RenderText_Solid failed: %s\n", TTF_GetError()); return; }
    
    SDL_Surface *rgba_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba_surface) { DualLogError("SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError()); return; }

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, GL_RGBA8, rgba_surface->w, rgba_surface->h);
    glTextureSubImage2D(texture, 0, 0, 0, rgba_surface->w, rgba_surface->h, GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUseProgram(textShaderProgram);
    glProgramUniformMatrix4fv(textShaderProgram, projectionLoc_text, 1, GL_FALSE, uiOrthoProjection);
    float r = textColors[colorIdx].r / 255.0f;
    float g = textColors[colorIdx].g / 255.0f;
    float b = textColors[colorIdx].b / 255.0f;
    float a = textColors[colorIdx].a / 255.0f;
    glProgramUniform4f(textShaderProgram, textColorLoc_text, r, g, b, a);
    glBindTextureUnit(0,texture);
    glProgramUniform1i(textShaderProgram, textTextureLoc_text, 0);
    float scaleX = (float)rgba_surface->w;
    float scaleY = (float)rgba_surface->h;
    glProgramUniform2f(textShaderProgram, texelSizeLoc_text, 1.0f / scaleX, 1.0f / scaleY);
    glEnable(GL_BLEND); // Enable blending for text transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // Disable depth test for 2D overlay
    glBindVertexArray(textVAO);
    float vertices[] = {
        x,          y,          0.0f, 0.0f, // Bottom-left
        x + scaleX, y,          1.0f, 0.0f, // Bottom-right
        x + scaleX, y + scaleY, 1.0f, 1.0f, // Top-right
        x,          y + scaleY, 0.0f, 1.0f  // Top-left
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Render quad (two triangles)
    drawCallsRenderedThisFrame++;
    verticesRenderedThisFrame+=6;

    // Cleanup
    glBindVertexArray(0);
    glBindTextureUnit(0, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); // Re-enable depth test for 3D rendering
    glUseProgram(0);
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(rgba_surface);
    glEnable(GL_CULL_FACE); // Reenable backface culling
    CHECK_GL_ERROR();
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
    statusTextLengthWithoutNullTerminator = vsnprintf(statusText, STATUS_TEXT_MAX_LENGTH, fmt, args);
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
        uint32_t drawCallsNormal = drawCallsRenderedThisFrame;

        // UI Common References
        int screenCenterX = screen_width / 2;
        int screenCenterY = screen_height / 2;
        int32_t lineSpacing = GetScreenRelativeY(0.03f);
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
        RenderFormattedText(leftPad, debugTextStartY, TEXT_WHITE, "x: %.4f, y: %.4f, z: %.4f", cam_x, cam_y, cam_z);
        RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 1), TEXT_WHITE, "cam yaw: %.2f, cam pitch: %.2f, cam roll: %.2f", cam_yaw, cam_pitch, cam_roll);
        RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 2), TEXT_WHITE, "Peak frame queue count: %d", maxEventCount_debug);
        RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 3), TEXT_WHITE, "DebugView: %d (%s), DebugValue: %d", debugView, debugViewNames[debugView], debugValue);
        RenderFormattedText(leftPad, debugTextStartY + (lineSpacing * 4), TEXT_WHITE, "Num cells: %d, Player cell(%d):: x: %d, y: %d, z: %d", numCellsVisible, playerCellIdx, playerCellIdx_x, playerCellIdx_y, playerCellIdx_z);

        if (consoleActive) RenderFormattedText(leftPad, 0, TEXT_WHITE, "] %s",consoleEntryText);
        if (statusTextDecayFinished > current_time) RenderFormattedText(GetTextHCenter(screenCenterX,statusTextLengthWithoutNullTerminator), screenCenterY - GetScreenRelativeY(0.30f + (genericTextHeightFac * 2.0f)), TEXT_WHITE, "%s",statusText);

        if (CursorVisible()) RenderFormattedText(cursorPosition_x - characterWidthHalf, cursorPosition_y - characterHeightHalf, TEXT_RED, "+");
        else RenderFormattedText(screenCenterX - characterWidthHalf, screenCenterY - characterHeightHalf, TEXT_GREEN, "+");

        // Frame stats
        double time_now = get_time();
        drawCallsRenderedThisFrame++; // Add one more for this text render ;)
        RenderFormattedText(leftPad, debugTextStartY - lineSpacing, TEXT_WHITE, "Frame time: %.6f (FPS: %d), Draw calls: %d [Geo %d, UI %d Shad %d], Verts: %d, Worst FPS: %d, Phys T: %.3f",
                            (time_now - last_time) * 1000.0f,framesPerLastSecond,drawCallsRenderedThisFrame,drawCallsNormal, drawCallsRenderedThisFrame - drawCallsNormal, drawCallsRenderedThisFrame - drawCallsNormal - 0,verticesRenderedThisFrame,worstFPS,time_PhysicsStep * 1000.0f);
        last_time = time_now;
        if ((time_now - lastFrameSecCountTime) >= 1.00) {
            lastFrameSecCountTime = time_now;
            framesPerLastSecond = globalFrameNum - lastFrameSecCount;
            if (framesPerLastSecond < worstFPS && globalFrameNum > 10) worstFPS = framesPerLastSecond; // After startup, keep track of worst framerate seen.
            lastFrameSecCount = globalFrameNum;
        }
        
        if (keys[SDL_SCANCODE_F12]) {
            if (time_now > screenshotTimeout) {
                Screenshot();
                screenshotTimeout = time_now + 1.0; // Prevent saving more than 1 per second for sanity purposes.
            }
        }
}
