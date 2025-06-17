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

const char *texturePaths[TEXTURE_COUNT] = {
    "./Textures/med1_1.png",
    "./Textures/med1_7.png",
    "./Textures/med1_9.png"
};

int LoadTextures(void) {
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        SDL_Surface *surface = IMG_Load(texturePaths[i]);
        if (!surface) { fprintf(stderr, "IMG_Load failed for %s: %s\n", texturePaths[i], IMG_GetError()); return 1; }
        
        textureSurfaces[i] = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
        SDL_FreeSurface(surface);
        if (!textureSurfaces[i]) { fprintf(stderr, "SDL_ConvertSurfaceFormat failed for %s: %s\n", texturePaths[i], SDL_GetError()); return 1; }
        
        glGenTextures(1, &textureIDs[i]);
        glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSurfaces[i]->w, textureSurfaces[i]->h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, textureSurfaces[i]->pixels);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
        printf("Texture %s: %dx%d\n", texturePaths[i], textureSurfaces[i]->w, textureSurfaces[i]->h);
    }
    
    return 0;
}
