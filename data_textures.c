#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include "data_definitions.h"
#include "data_textures.h"
#include "constants.h"
#include "debug.h"
#include "render.h"

SDL_Surface ** textureSurfaces = NULL;
GLuint * textureIDs = NULL;
GLuint64 * textureHandles = NULL;
GLuint colorBufferID = 0; // Single color buffer
uint32_t * textureOffsets = NULL; // Pixel offsets
uint32_t totalPixels = 0; // Total pixels across all textures
int * textureSizes = NULL; // Needs to be textureCount * 2
int textureCount;

const char *valid_texdata_keys[] = {"index"};

int LoadTextures(void) {
    textureCount = 0;
    
    // First parse ./Data/textures.txt to see what textures to load to what indices
    DataParser texture_parser;
    parser_init(&texture_parser, valid_texdata_keys, 1);
    if (!parse_data_file(&texture_parser, "./Data/textures.txt")) {
        printf("ERROR: Could not parse ./Data/textures.txt!\n");
        parser_free(&texture_parser);
        return 1;
    }
    
    textureCount = texture_parser.count;
    textureSurfaces = malloc(textureCount * sizeof(SDL_Surface *));
    textureIDs = malloc(textureCount * sizeof(GLuint));
    textureHandles = malloc(textureCount * sizeof(GLuint64));
    textureOffsets = malloc(textureCount * sizeof(uint32_t));
    textureSizes = malloc(textureCount * 2 * sizeof(int)); // Times 2 for x and y pairs flat packed (e.g. x,y,x,y,x,y for 3 textures)
    
    // Now load each one
    // Calculate total pixels and offsets
    totalPixels = 0;
    for (int i = 0; i < textureCount; i++) {
        textureOffsets[i] = totalPixels;
        int matchedParserIdx = -1;
        for (int k=0;k<texture_parser.count;k++) {
            if (texture_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }
        
        if (matchedParserIdx < 0) { printf("Texture index %d not specified in textures.txt definitions file, skipped\n", i); continue; }
        
        SDL_Surface *surface = IMG_Load(texture_parser.entries[matchedParserIdx].path);
        if (!surface) { fprintf(stderr, "IMG_Load failed for %s: %s\n", texture_parser.entries[matchedParserIdx].path, IMG_GetError()); free(textureSurfaces); free(textureIDs); free(textureHandles); free(textureOffsets); free(textureSizes); return 1; }
        totalPixels += surface->w * surface->h;
        textureSizes[i * 2] = surface->w;
        textureSizes[(i * 2) + 1] = surface->h;
        SDL_FreeSurface(surface);
        printf("Texture %s loaded with %d pixels, %d wide by %d tall\n", texture_parser.entries[matchedParserIdx].path,  surface->w * surface->h, surface->w, surface->h);
    }

    // Allocate color buffer data
    float *colorData = (float *)malloc(totalPixels * 4 * sizeof(float));
    if (!colorData) { fprintf(stderr, "Failed to allocate color buffer\n"); free(textureSurfaces); free(textureIDs); free(textureHandles); free(textureOffsets); free(textureSizes); return 1; }

//     uint32_t *uniqueColors = malloc(totalPixels * sizeof(uint32_t));
//     if (!uniqueColors) { fprintf(stderr, "Failed to allocate unique color buffer\n"); free(colorData); free(textureSurfaces); free(textureIDs); free(textureHandles); free(textureOffsets); free(textureSizes); return 1; }
//     uint32_t uniqueColorHead = 0;
    
    // Load textures into color buffer
    uint32_t pixelIndex = 0;
    for (int i = 0; i < textureCount; i++) {
        // Find matching index to i that was parsed from file
        int matchedParserIdx = -1;
        for (int k=0;k<texture_parser.count;k++) {
            if (texture_parser.entries[k].index == i) {matchedParserIdx = k; break; }
        }
        
        if (matchedParserIdx < 0) { printf("Texture index %d not specified in textures.txt definitions file, skipped\n", i); continue; }

        SDL_Surface *surface = IMG_Load(texture_parser.entries[matchedParserIdx].path);
        if (!surface) { fprintf(stderr, "IMG_Load failed for %s: %s\n", texture_parser.entries[matchedParserIdx].path, IMG_GetError()); free(colorData); free(textureSurfaces); free(textureIDs); free(textureHandles); free(textureOffsets); free(textureSizes);  return 1; }
        textureSurfaces[i] = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
        SDL_FreeSurface(surface);
        if (!textureSurfaces[i]) { fprintf(stderr, "SDL_ConvertSurfaceFormat failed for %s: %s\n", texture_parser.entries[matchedParserIdx].path, SDL_GetError()); free(colorData); free(textureSurfaces); free(textureIDs); free(textureHandles); free(textureOffsets); free(textureSizes);  return 1; }

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
    
    printf("Last pixel: r %f, g %f, b %f, a %f\n",colorData[(totalPixels * 4) - 4],colorData[(totalPixels * 4) - 3],colorData[(totalPixels * 4) - 2],colorData[(totalPixels * 4) - 1]);
    printf("Total pixels in buffer %d (",totalPixels);
    print_bytes_no_newline(totalPixels * 4 * 4); // rgba = 4, 4 bytes per float = 4x4
    printf(")\n");
    
    // Create SSBO for color buffer
    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalPixels * 4 * sizeof(float), colorData, GL_STATIC_DRAW);
    
          // Set static buffer once for all shaders
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, colorBufferID);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    free(colorData);
    
    // Send static uniforms to chunk shader
    glUniform1uiv(textureOffsetsLoc_chunk, textureCount, textureOffsets);
    glUniform2iv(textureSizesLoc_chunk, textureCount, textureSizes);
    glUniform1ui(textureCountLoc_chunk, textureCount);

    // Send static uniforms to deferred lighting shader
    glUniform1uiv(textureOffsetsLoc_deferred, textureCount, textureOffsets);
    glUniform2iv(textureSizesLoc_deferred, textureCount, textureSizes);
    
    // Delete individual texture IDs
    for (int i = 0; i < textureCount; i++) {
        if (textureIDs[i]) glDeleteTextures(1, &textureIDs[i]);
        textureIDs[i] = 0;
    }
    
    free(textureSurfaces); // Free temporary buffers for texture loading
    free(textureIDs);
    free(textureHandles); 
    // Do not free textureOffsets, used by the program during runtime!  !!!
    // Do not free textureSizes, used by the program during runtime!    !!!
    // Note that textureOffsets and textureSizes are freed if this function returns 1 since the program will error and exit from those.
    return 0;
}
