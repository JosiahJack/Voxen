// voxel.c
// Description: Defines the voxel space invisible overlay data structure that
//              is positioned over the world at a regular interval and used by
//              both lighting and physics systems for all visibility and
//              collision checks.
//
// Voxels are in the following hierarchy:
// World cells 64x64x32
// Each cell contains 8x8x8 voxels.  World cells not currently utilized in
// VXGI, just raw voxels so 1024x1024x512.
//
// A voxel is 0.32f x 0.32f x 0.32f
// Each world cell is 2.56f x 2.56f x 2.56f
//
// Side note:
// In the physics engine, each cell is 2560 x 2560 x 2560 units wide (* 1000)
// and each voxel is 320 x 320 x 320 for 0.001f precision in world space
// Voxels do not contain voxel size 1x1x1 in physics due to RAM constraints.
//
// All voxels in voxel space = 64x64x32x8x8x8 = 67108864 voxels total (aka 2^26).

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdio.h>
#include "voxel.h"
#include "debug.h"
#include "lights.h"
#include "data_models.h"
#include "render.h"

VXGIBuffer vxgi_buffers[2];
int current_vxgi_upload_index = 0;

SDL_Thread* vxgi_thread = NULL;
_Atomic bool vxgi_running = true;
GLuint vxgiID;

void SVO_Init(SVO* svo, int index) {
    svo->nodeCapacity = SVO_INITIAL_NODE_CAPACITY;
    svo->voxelCapacity = SVO_INITIAL_VOXEL_CAPACITY;
    svo->nodes = malloc(sizeof(SVONode) * svo->nodeCapacity);
    svo->voxels = malloc(sizeof(VXGIVoxel) * svo->voxelCapacity);
    svo->nodeCount = 1;  // Root node
    svo->voxelCount = 0;
    svo->nodes[0].childrenMask = 0;
    memset(svo->nodes[0].children, 0, sizeof(uint32_t) * SVO_NODE_CHILDREN);
    DualLog("SVO buffer %d initialized with ",index);
    print_bytes_no_newline(sizeof(VXGIVoxel) * svo->voxelCapacity + sizeof(SVONode) * svo->nodeCapacity);
    DualLog("\n");
}

void VXGI_Init(void) {
    DualLog("VXGI Init\n");
    for (int i = 0; i < 2; ++i) {
        SVO_Init(&vxgi_buffers[i].svo,i);
        atomic_store(&vxgi_buffers[i].ready, false);
        DualLog("Buffer %d initialized\n", i);
    }
    
    DualLog("Spawning VXGI worker thread... ");
    VXGIWorkerData* workerData = malloc(sizeof(VXGIWorkerData));
    workerData->lights = lights;
    workerData->lightCount = LIGHT_COUNT;
    vxgi_thread = SDL_CreateThread(VXGI_Worker, "VXGIThread", workerData);
    
    glGenBuffers(1, &vxgiID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vxgiID);
    
    // Allocate initial buffer size for voxels
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(VXGIVoxel) * SVO_INITIAL_VOXEL_CAPACITY, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, vxgiID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUniform1i(vxgiEnabledLoc_deferred, 0); // Mark as unusable until first vxgi thread iteration completes.
    glUniform1i(voxelCountLoc_deferred, 0);
    DualLog("DONE\n");
    DebugRAM("after initializing all voxels");
}

bool vxgiEnabled = true;

void VXGI_UpdateGL(void) {
    static int last_write_index = -1;
    int read_index = 1 - current_vxgi_upload_index;  // Read from the buffer the worker just wrote

    VXGIBuffer* buffer = &vxgi_buffers[read_index];
    if (!atomic_load(&buffer->ready) || read_index == last_write_index) {
        return;  // No new data or already processed
    }

    // Resize SSBO if voxel count exceeds current capacity
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vxgiID);
    if (buffer->svo.voxelCount > SVO_INITIAL_VOXEL_CAPACITY) {  // Check against current buffer size
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(VXGIVoxel) * buffer->svo.voxelCount, NULL, GL_DYNAMIC_DRAW);
    }

    // Upload voxel data
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(VXGIVoxel) * buffer->svo.voxelCount, buffer->svo.voxels);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, vxgiID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Update uniforms
    glUniform1i(vxgiEnabledLoc_deferred, vxgiEnabled); // Enable VXGI
    glUniform1i(voxelCountLoc_deferred, buffer->svo.voxelCount);

    last_write_index = read_index;  // Track processed buffer
}

//     if (vxgiEnabled) {
//         for (int i = 0; i < 2; ++i) {
//             if (atomic_load(&vxgi_buffers[i].ready)) {            
//                 // Upload buffer to SSBO binding 19
//                 glBindBuffer(GL_SHADER_STORAGE_BUFFER, vxgiID);
//                 glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(VXGIVoxel) * VXGI_BUFFER_SIZE, vxgi_buffers[i].data);
//                 glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, vxgiID);
//                 glUniform1i(vxgiEnabledLoc_deferred, 1);  // Tell shader GI is active
//                 current_vxgi_upload_index = i;
// //                 atomic_store(&vxgi_buffers[i].ready, false);
//                 break;
//             }
//         }
//     }

// Helper to map world position to voxel grid coordinates
void WorldToVoxel(float x, float y, float z, int* vx, int* vy, int* vz) {
    *vx = (int)(x / 0.32f);
    *vy = (int)(y / 0.32f);
    *vz = (int)(z / 0.32f);
}

// Insert a voxel into the SVO at the given grid coordinates
void SVO_InsertVoxel(SVO* svo, int vx, int vy, int vz, VXGIVoxel voxel) {
    if (vx < 0 || vx >= 1024 || vy < 0 || vy >= 1024 || vz < 0 || vz >= 512) return;

    uint32_t nodeIdx = 0;  // Start at root
    int minX = 0, minY = 0, minZ = 0;
    int size = 1024;  // Max size x,y

    for (int depth = 0; depth < SVO_MAX_DEPTH; ++depth) {
        SVONode* node = &svo->nodes[nodeIdx];
        int halfSize = size / 2;
        int childX = (vx >= minX + halfSize) ? 1 : 0;
        int childY = (vy >= minY + halfSize) ? 1 : 0;
        int childZ = (vz >= minZ + halfSize) ? 1 : 0;
        int childBit = (childZ << 2) | (childY << 1) | childX;

        if (depth == SVO_MAX_DEPTH - 1) {  // Leaf level
            if (!(node->childrenMask & (1 << childBit))) {
                if (svo->voxelCount >= svo->voxelCapacity) {
                    svo->voxelCapacity *= 2;
                    svo->voxels = realloc(svo->voxels, sizeof(VXGIVoxel) * svo->voxelCapacity);
                }
                
                node->children[childBit] = svo->voxelCount;
                svo->voxels[svo->voxelCount++] = voxel;
                node->childrenMask |= (1 << childBit);
            }
            return;
        }

        // Non-leaf: create child node if needed
        if (!(node->childrenMask & (1 << childBit))) {
            if (svo->nodeCount >= svo->nodeCapacity) {
                svo->nodeCapacity *= 2;
                svo->nodes = realloc(svo->nodes, sizeof(SVONode) * svo->nodeCapacity);
            }
            
            node->children[childBit] = svo->nodeCount++;
            svo->nodes[node->children[childBit]].childrenMask = 0;
            memset(svo->nodes[node->children[childBit]].children, 0, sizeof(uint32_t) * SVO_NODE_CHILDREN);
            node->childrenMask |= (1 << childBit);
        }

        nodeIdx = node->children[childBit];
        minX += childX * halfSize;
        minY += childY * halfSize;
        minZ += childZ * halfSize;
        size = halfSize;
    }
}

void InsertModelInstanceVoxels(int modelIdx, int texIdx, float posX, float posY, float posZ) {
    SVO* svo = &vxgi_buffers[current_vxgi_upload_index].svo;

    // Get vertex data for the model
    if (vertexDataArrays[modelIdx] == NULL) { DualLogError("Vertex data null for model index %i\n",modelIdx); return; }
    
    float* vertices = vertexDataArrays[modelIdx];
    int vertexCount = modelVertexCounts[modelIdx];

    for (int i = 0; i < vertexCount; i += VERTEX_ATTRIBUTES_COUNT) {
        float x = vertices[i] + posX;
        float y = vertices[i + 1] + posY;
        float z = vertices[i + 2] + posZ;

        int vx, vy, vz;
        WorldToVoxel(x, y, z, &vx, &vy, &vz);

        float lit = texIdx == 881 ? 1.0f : 0.0f; // TODO specify via textures.txt into array of emissive indices and then create function to check against the list
                                                 // TODO get rgb average at texture load time and use the average texture color here (or average uv color!).
        VXGIVoxel voxel = { .direct_light = {lit, lit, lit}, .indirect_light = {0.0f, 0.0f, 0.0f}, .occupancy = lit > 0.0f ? 0.5f : 1.0f }; // TODO: Hack!  Use bit packing or something instead.
        SVO_InsertVoxel(svo, vx, vy, vz, voxel);
    }

    atomic_store(&vxgi_buffers[current_vxgi_upload_index].ready, true);
}

int VXGI_Worker(__attribute__((unused)) void* data) {
    VXGIWorkerData* workerData = (VXGIWorkerData*)data;
    int write_index = 0;
    while (vxgi_running) {
        VXGIBuffer* buffer = &vxgi_buffers[write_index];
        if (!atomic_load(&buffer->ready)) {
            SDL_Delay(1);  // Avoid busy-wait
            continue;
        }

        // Process voxels with lights
        for (uint32_t i = 0; i < buffer->svo.voxelCount; ++i) {
            VXGIVoxel* voxel = &buffer->svo.voxels[i];  // Fixed typo
            // Placeholder: Accumulate light contributions
            float r = voxel->direct_light[0];
            float g = voxel->direct_light[1];
            float b = voxel->direct_light[2];
            for (uint32_t j = 0; j < workerData->lightCount; ++j) {
                int lightBaseIdx = j * LIGHT_DATA_SIZE;
                // TODO: Compute light influence (e.g., distance-based attenuation)
                float intensity = workerData->lights[lightBaseIdx + 3];  // intensity
                r += intensity * 0.1f;  // Placeholder
                g += intensity * 0.1f;
                b += intensity * 0.1f;
            }
            voxel->direct_light[0] = fminf(r, 1.0f);
            voxel->direct_light[1] = fminf(g, 1.0f);
            voxel->direct_light[2] = fminf(b, 1.0f);
        }

        atomic_store(&buffer->ready, true);
        write_index = 1 - write_index;  // Swap buffers
    }
    free(data);  // Clean up workerData
    return 0;
}

void VXGI_Shutdown(void) {
    vxgi_running = false;
    SDL_WaitThread(vxgi_thread, NULL);

    for (int i = 0; i < 2; ++i) {
        free(vxgi_buffers[i].svo.nodes);
        free(vxgi_buffers[i].svo.voxels);
    }
}
