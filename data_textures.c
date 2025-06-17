#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include "data_textures.h"
#include "constants.h"

SDL_Surface *textureSurfaces[TEXTURE_COUNT];
GLuint textureIDs[TEXTURE_COUNT];
GLuint64 textureHandles[TEXTURE_COUNT];
GLuint colorBufferID; // Single color buffer
uint32_t textureOffsets[TEXTURE_COUNT]; // Pixel offsets
uint32_t totalPixels; // Total pixels
int textureSizes[TEXTURE_COUNT * 2];

const char *texturePaths[TEXTURE_COUNT] = {
    "./Textures/med1_1.png",
    "./Textures/med1_7.png",
    "./Textures/med1_9.png"
};

int LoadTextures(void) {
    // Calculate total pixels and offsets
    totalPixels = 0;
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        textureOffsets[i] = totalPixels;
        SDL_Surface *surface = IMG_Load(texturePaths[i]);
        if (!surface) { fprintf(stderr, "IMG_Load failed for %s: %s\n", texturePaths[i], IMG_GetError()); return 1; }
        totalPixels += surface->w * surface->h;
        textureSizes[i * 2] = surface->w;
        textureSizes[(i * 2) + 1] = surface->h;
        SDL_FreeSurface(surface);
    }

    // Allocate color buffer data
    float *colorData = (float *)malloc(totalPixels * 4 * sizeof(float));
    if (!colorData) { fprintf(stderr, "Failed to allocate color buffer\n"); return 1; }

    // Load textures into color buffer
    uint32_t pixelIndex = 0;
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        SDL_Surface *surface = IMG_Load(texturePaths[i]);
        if (!surface) { fprintf(stderr, "IMG_Load failed for %s: %s\n", texturePaths[i], IMG_GetError()); free(colorData); return 1; }
        textureSurfaces[i] = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
        SDL_FreeSurface(surface);
        if (!textureSurfaces[i]) { fprintf(stderr, "SDL_ConvertSurfaceFormat failed for %s: %s\n", texturePaths[i], SDL_GetError()); free(colorData); return 1; }

        uint8_t *pixels = (uint8_t *)textureSurfaces[i]->pixels;
        for (int j = 0; j < textureSurfaces[i]->w * textureSurfaces[i]->h; j++) {
            colorData[pixelIndex++] = pixels[j * 4 + 0] / 255.0f; // R
            colorData[pixelIndex++] = pixels[j * 4 + 1] / 255.0f; // G
            colorData[pixelIndex++] = pixels[j * 4 + 2] / 255.0f; // B
            colorData[pixelIndex++] = pixels[j * 4 + 3] / 255.0f; // A
        }
        SDL_FreeSurface(textureSurfaces[i]);
        textureSurfaces[i] = NULL;
    }

    // Create SSBO for color buffer
    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPixels * 4 * sizeof(float), colorData, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    free(colorData);

    // Delete individual texture IDs
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        if (textureIDs[i]) glDeleteTextures(1, &textureIDs[i]);
        textureIDs[i] = 0;
    }

    return 0;
}
