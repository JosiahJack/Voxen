#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include "text.h"

GLuint textShaderProgram;
TTF_Font* font = NULL;
GLuint textVAO, textVBO;
GLuint fontAtlasTexture;
int fontAtlasWidth = 512, fontAtlasHeight = 512; // Adjust as needed

// Quad for text (2 triangles, positions and tex coords)
float textQuadVertices[] = {
    // Positions   // Tex Coords
    0.0f, 0.0f,    0.0f, 0.0f, // Bottom-left
    1.0f, 0.0f,    1.0f, 0.0f, // Bottom-right
    1.0f, 1.0f,    1.0f, 1.0f, // Top-right
    0.0f, 1.0f,    0.0f, 1.0f  // Top-left
};

int InitializeTextAndFonts(void) {
    SetupTextQuad();
    font = TTF_OpenFont("./Fonts/SystemShockText.ttf", 12);
    if (!font) { fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError()); return 1; }
    SetupFontAtlas();
    return 0;
}

void SetupTextQuad(void) {
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textQuadVertices), textQuadVertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Tex Coord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SetupFontAtlas(void) {
//     // Create temporary surface for atlas
//     SDL_Surface *atlasSurface = SDL_CreateRGBSurface(0, fontAtlasWidth, fontAtlasHeight, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);
//     if (!atlasSurface) { fprintf(stderr, "SDL_CreateRGBSurface failed: %s\n", SDL_GetError()); return; }
// 
//     // Clear surface to transparent
//     SDL_FillRect(atlasSurface, NULL, SDL_MapRGBA(atlasSurface->format, 0, 0, 0, 0));
// 
//     // Render glyphs with outline
//     SDL_Color white = {255, 255, 255, 255};
//     int x = 0, y = 0, maxHeight = 0;
//     for (int c = 32; c < 128; c++) { // ASCII printable chars
//         SDL_Surface *glyph = TTF_RenderGlyph_Solid(font, (char)c, white);
//         if (!glyph) continue;
// 
//         // Create outline surface (1-pixel larger in each direction)
//         SDL_Surface *outline = SDL_CreateRGBSurface(0, glyph->w + 2, glyph->h + 2, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);
//         SDL_FillRect(outline, NULL, SDL_MapRGBA(outline->format, 0, 0, 0, 0));
// 
//         // Copy glyph to outline surface (offset by 1 pixel)
//         SDL_Rect dstRect = {1, 1, glyph->w, glyph->h};
//         SDL_BlitSurface(glyph, NULL, outline, &dstRect);
// 
//         // Add black outline
//         Uint32 *pixels = (Uint32 *)outline->pixels;
//         Uint32 whitePixel = SDL_MapRGBA(outline->format, 255, 255, 255, 255);
//         Uint32 blackPixel = SDL_MapRGBA(outline->format, 0, 0, 0, 255);
//         Uint32 transparentPixel = SDL_MapRGBA(outline->format, 0, 0, 0, 0);
//         SDL_Surface *temp = SDL_CreateRGBSurface(0, outline->w, outline->h, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);
//         SDL_FillRect(temp, NULL, transparentPixel);
//         Uint32 *tempPixels = (Uint32 *)temp->pixels;
// 
//         for (int gy = 0; gy < outline->h; gy++) {
//             for (int gx = 0; gx < outline->w; gx++) {
//                 int idx = gy * outline->w + gx;
//                 if (pixels[idx] == whitePixel) {
//                     tempPixels[idx] = whitePixel;
//                     continue;
//                 }
//                 bool isOutline = false;
//                 for (int dy = -1; dy <= 1 && !isOutline; dy++) {
//                     for (int dx = -1; dx <= 1; dx++) {
//                         if (dx == 0 && dy == 0) continue;
//                         int nx = gx + dx, ny = gy + dy;
//                         if (nx >= 0 && nx < outline->w && ny >= 0 && ny < outline->h) {
//                             if (pixels[ny * outline->w + nx] == whitePixel) {
//                                 isOutline = true;
//                                 break;
//                             }
//                         }
//                     }
//                 }
//                 tempPixels[idx] = isOutline ? blackPixel : transparentPixel;
//             }
//         }
// 
//         // Copy to atlas
//         if (x + outline->w > fontAtlasWidth) { x = 0; y += maxHeight + 2; maxHeight = 0; }
//         SDL_Rect atlasRect = {x, y, outline->w, outline->h};
//         SDL_BlitSurface(temp, NULL, atlasSurface, &atlasRect);
//         // Store texture offset and size (update textureOffsets, textureSizes)
//         textureOffsets[c - 32] = (y * fontAtlasWidth + x) * 4; // Store byte offset
//         textureSizes[c - 32 * 2] = outline->w; // Store width
//         textureSizes[c - 32 * 2 + 1] = outline->h; // Store height
//         x += outline->w + 2;
//         maxHeight = outline->h > maxHeight ? outline->h : maxHeight;
// 
//         SDL_FreeSurface(glyph);
//         SDL_FreeSurface(outline);
//         SDL_FreeSurface(temp);
//     }
// 
//     // Upload atlas to OpenGL
//     glGenTextures(1, &fontAtlasTexture);
//     glBindTexture(GL_TEXTURE_2D, fontAtlasTexture);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fontAtlasWidth, fontAtlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasSurface->pixels);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glBindTexture(GL_TEXTURE_2D, 0);
// 
//     SDL_FreeSurface(atlasSurface);
//     TTF_CloseFont(font);
}
