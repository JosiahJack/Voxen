// data_models.c - Load 3D Models
#include "os.h"
#include "gl.h"
#include "voxen.h"
#include <string.h>
#include <omp.h>

float** modelVertices = NULL;
uint32_t** modelTriangles = NULL;
uint32_t modelVertexCounts[MODEL_IDX_MAX] = {0};
uint32_t modelTriangleCounts[MODEL_IDX_MAX] = {0};
bool modelHasAnimation[MODEL_IDX_MAX] = {0};
float modelBounds[MODEL_IDX_MAX * BOUNDS_ATTRIBUTES_COUNT] = {0};
uint16_t loadedModelsMaxIndex = 0;
GLuint SetupSSBO(GLuint* id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage);
#define MAX_VERT_ELEMENT_SIZE 6964  // From max verts for a model: 6135, Max normals: 3774, Max UVS: 6962
#define MAX_OUTPUT_VERTS      20892 // 6964 faces * 3
static float**    thread_temp_pos   = NULL;
static float**    thread_temp_nrm   = NULL;
static float**    thread_temp_uv    = NULL;
static float**    thread_out_verts  = NULL;
static uint32_t** thread_out_tris   = NULL;
static int        num_parse_threads = 0;

static inline __attribute__((always_inline)) uint32_t parse_numberu32_pure(const char* str) {
    uint32_t result = 0;
    while (*str >= '0' && *str <= '9') { result = result * 10 + (*str - '0'); ++str; }
    return result;
}

static inline __attribute__((always_inline)) float parse_float_pure(const char* str) {
    float sign = 1.0f;
    if (*str == '-') { sign = -1.0f; ++str; }
    float value = 0.0f;
    while (*str >= '0' && *str <= '9') { value = value * 10.0f + (*str - '0'); ++str; }
    if (*str == '.') {
        ++str;
        float place = 0.1f;
        while (*str >= '0' && *str <= '9') {
            value += (*str - '0') * place;
            place *= 0.1f;
            ++str;
        }
    }
    return sign * value;
}

static __attribute__((hot)) __attribute__((optimize("O3"))) __attribute__((flatten)) bool ParseOBJ(const char* __restrict data, int file_size, float* __restrict temp_pos, float* __restrict temp_nrm, float* __restrict temp_uv, float* __restrict scratch_verts, uint32_t* __restrict scratch_tris, float** out_vertices, uint32_t* out_vertex_count, uint32_t** out_triangles, uint32_t* out_triangle_count, float* out_minx, float* out_miny, float* out_minz, float* out_maxx, float* out_maxy, float* out_maxz) {
    *out_vertices = NULL; *out_triangles = NULL;
    *out_vertex_count = *out_triangle_count = 0;
    if (unlikely(!data || file_size <= 0)) return false;

    uint32_t vi = 0, ni = 0, ui = 0;
    uint32_t vert_idx = 0;
    float minx = 1e9f, miny = 1e9f, minz = 1e9f;
    float maxx = -1e9f, maxy = -1e9f, maxz = -1e9f;
    const char* p = data;
    const char* const end = data + file_size;
    while (likely(p < end)) {
        const char c = *p;
        if (likely(c == 'v')) {
            if (p[1] == ' ') {
                if (unlikely(vi >= MAX_VERT_ELEMENT_SIZE)) return false;
                const char* num = p + 2;
                temp_pos[vi*3]   = parse_float_pure(num); while (*num && *num != ' ' && *num != '\t') { ++num; } ++num;
                temp_pos[vi*3+1] = parse_float_pure(num); while (*num && *num != ' ' && *num != '\t') { ++num; } ++num;
                temp_pos[vi*3+2] = parse_float_pure(num);
                ++vi;
            } else if (p[1] == 'n' && p[2] == ' ') {
                if (unlikely(ni >= MAX_VERT_ELEMENT_SIZE)) return false;
                const char* num = p + 3;
                temp_nrm[ni*3]   = parse_float_pure(num); while (*num && *num != ' ' && *num != '\t') { ++num; } ++num;
                temp_nrm[ni*3+1] = parse_float_pure(num); while (*num && *num != ' ' && *num != '\t') { ++num; } ++num;
                temp_nrm[ni*3+2] = parse_float_pure(num);
                ++ni;
            } else if (p[1] == 't' && p[2] == ' ') {
                if (unlikely(ui >= MAX_VERT_ELEMENT_SIZE)) return false;
                const char* num = p + 3;
                temp_uv[ui*2]   = parse_float_pure(num); while (*num && *num != ' ' && *num != '\t') { ++num; } ++num;
                temp_uv[ui*2+1] = parse_float_pure(num);
                ++ui;
            }
        } else if (likely(c == 'f' && p[1] == ' ')) {
            const char* num = p + 2;
            while (*num == ' ' || *num == '\t') ++num;
            uint32_t v0=0, v1=0, v2=0, vt0=0, vt1=0, vt2=0, vn0=0, vn1=0, vn2=0;
            #define PARSE_IDX(I) do { \
                int32_t raw = 0; \
                if (*num == '-') { ++num; raw = -(int32_t)parse_numberu32_pure(num); } \
                else raw = (int32_t)parse_numberu32_pure(num); \
                if (raw > 0) I = (uint32_t)raw; \
                else if (raw < 0) I = (uint32_t)((int32_t)vi + raw); /* relative */ \
                else I = 0; \
                while (*num && *num != '/' && *num != ' ' && *num != '\t') ++num; \
            } while(0)
            PARSE_IDX(v0); if (*num == '/') { ++num; if (*num != '/') { PARSE_IDX(vt0); } if (*num == '/') { ++num; PARSE_IDX(vn0); } }
            while (*num && *num != ' ' && *num != '\t') ++num;
            while (*num == ' ' || *num == '\t') ++num;
            PARSE_IDX(v1); if (*num == '/') { ++num; if (*num != '/') { PARSE_IDX(vt1); } if (*num == '/') { ++num; PARSE_IDX(vn1); } }
            while (*num && *num != ' ' && *num != '\t') ++num;
            while (*num == ' ' || *num == '\t') ++num;
            PARSE_IDX(v2); if (*num == '/') { ++num; if (*num != '/') { PARSE_IDX(vt2); } if (*num == '/') { ++num; PARSE_IDX(vn2); } }
            #undef PARSE_IDX
            if (likely(v0 && v1 && v2)) {
                if (unlikely(vert_idx + 3 > MAX_OUTPUT_VERTS)) return false;

                #define EMIT(V, VT, VN) do { \
                    uint32_t vi_idx = (V)-1; \
                    uint32_t ti_idx = ((VT) && (VT) <= ui) ? (VT)-1 : 0; \
                    uint32_t ni_idx = ((VN) && (VN) <= ni) ? (VN)-1 : 0; \
                    float* dst = scratch_verts + (vert_idx << 3); \
                    dst[0] = temp_pos[vi_idx*3]; dst[1] = temp_pos[vi_idx*3+1]; dst[2] = temp_pos[vi_idx*3+2]; \
                    dst[3] = (ni_idx < ni) ? temp_nrm[ni_idx*3]   : 0.0f; \
                    dst[4] = (ni_idx < ni) ? temp_nrm[ni_idx*3+1] : 0.0f; \
                    dst[5] = (ni_idx < ni) ? temp_nrm[ni_idx*3+2] : 0.0f; \
                    dst[6] = (ti_idx < ui) ? temp_uv[ti_idx*2]    : 0.0f; \
                    dst[7] = (ti_idx < ui) ? temp_uv[ti_idx*2+1]  : 0.0f; \
                    float x = dst[0], y = dst[1], z = dst[2]; \
                    minx = x < minx ? x : minx; maxx = x > maxx ? x : maxx; \
                    miny = y < miny ? y : miny; maxy = y > maxy ? y : maxy; \
                    minz = z < minz ? z : minz; maxz = z > maxz ? z : maxz; \
                    scratch_tris[vert_idx] = vert_idx; \
                    ++vert_idx; \
                } while(0)

                EMIT(v0, vt0, vn0);
                EMIT(v1, vt1, vn1);
                EMIT(v2, vt2, vn2);
                #undef EMIT
            }
        }

        p = (const char*)memchr(p, '\n', (size_t)(end - p));
        if (unlikely(!p)) break;
        
        ++p;
        if (p < end && *p == '\r') ++p;
    }

    if (unlikely(vert_idx == 0)) return false;
    
    #define HASH_SIZE 32768
    uint32_t hash_table[HASH_SIZE];
    SetMemoryToValueForNBytes(hash_table, 0xFF, sizeof(hash_table)); // 0xFFFFFFFF = empty
    float* unique_verts = scratch_verts;  // reuse scratch
    uint32_t* remap      = (uint32_t*)scratch_tris; // reuse scratch for remap
    uint32_t unique_cnt  = 0;
    for (uint32_t i = 0; i < vert_idx; ++i) {
        const float* v = scratch_verts + (i << 3);
        uint64_t h = *(uint64_t*)(v+0) ^ *(uint64_t*)(v+2) ^ *(uint64_t*)(v+4) ^ *(uint64_t*)(v+6);
        uint32_t slot = (uint32_t)(h ^ (h >> 32)) & (HASH_SIZE-1);
        while (hash_table[slot] != 0xFFFFFFFF) {
            const float* candidate = unique_verts + (hash_table[slot] << 3);
            if (__builtin_memcmp(candidate, v, 32) == 0) { // exact match
                remap[i] = hash_table[slot];
                goto next_vertex;
            }
            slot = (slot + 1) & (HASH_SIZE-1);
        }

        hash_table[slot] = unique_cnt;
        remap[i] = unique_cnt;
        __builtin_memcpy(unique_verts + (unique_cnt << 3), v, 32);
        ++unique_cnt;
    next_vertex:;
    }

    size_t vbytes = (size_t)unique_cnt * 8 * sizeof(float);
    float* final_verts = (float*)OS_AllocateRAM(NULL, vbytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    __builtin_memcpy(final_verts, unique_verts, vbytes);
    size_t ibytes = (size_t)vert_idx * sizeof(uint32_t); // still need full index list
    uint32_t* final_tris = (uint32_t*)OS_AllocateRAM(NULL, ibytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    for (uint32_t i = 0; i < vert_idx; ++i) final_tris[i] = remap[i];
    *out_vertices       = final_verts;
    *out_vertex_count   = unique_cnt;
    *out_triangles      = final_tris;
    *out_triangle_count = vert_idx / 3;
    *out_minx = minx; *out_miny = miny; *out_minz = minz;
    *out_maxx = maxx; *out_maxy = maxy; *out_maxz = maxz;
    return true;
}

void LoadModels(void) {
    if (unlikely(loadedModelsMaxIndex > 0)) return;

    double start_time = get_time();
    DataParser mpars;
    if (unlikely(!parse_data_file(&mpars, MODEL_IDX_MAX, "./Data/models.txt"))) { DualLogError("Could not parse ./Data/models.txt!\n"); OS_Exit(1); }

    int32_t maxIndex = -1;
    for (uint32_t k = 0; k < mpars.count; k++) {
        if (mpars.entries[k].index > maxIndex && mpars.entries[k].index != UINT16_MAX) maxIndex = mpars.entries[k].index;
    }

    loadedModelsMaxIndex = (uint16_t)maxIndex + 1U;
    DualLog("Loading models (%d/%d) with max index %d ...", loadedModelsMaxIndex, mpars.count, maxIndex);
    modelVertices   = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(float*),    PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    modelTriangles  = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(uint32_t*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    size_t indexToParser_size = loadedModelsMaxIndex * sizeof(int32_t);
    int32_t* indexToParser = OS_AllocateRAM(NULL, indexToParser_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
    SetMemoryToValueForNBytes(indexToParser, -1, indexToParser_size);
    for (uint32_t k = 0; k < mpars.count; k++) {
        if (likely(mpars.entries[k].index != UINT16_MAX)) indexToParser[mpars.entries[k].index] = (int32_t)k;
    }

    typedef struct { const char* data; int size; } RawOBJ;
    RawOBJ* rawModels = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(RawOBJ), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    SetMemoryToValueForNBytes(rawModels, 0, loadedModelsMaxIndex * sizeof(RawOBJ));
    #pragma omp parallel for schedule(dynamic)
    for (uint32_t i = 0; i < loadedModelsMaxIndex; ++i) {
        int32_t parserIdx = indexToParser[i];
        if (unlikely(parserIdx < 0 || parserIdx >= (int32_t)mpars.count)) continue;

        const char* path = mpars.entries[parserIdx].path;
        OsFileHandle dummy_fd;
        int size = 0;
        rawModels[i].data = (const char*)OS_OpenAndAllocateFileBufferReadonly(path, &dummy_fd, &size);
        rawModels[i].size = size;
    }

    double timeAtStartOfSecondLoop = get_time();
    num_parse_threads = omp_get_max_threads();
    if (unlikely(num_parse_threads < 1)) num_parse_threads = 1;
    thread_temp_pos  = (float**)OS_AllocateRAM(NULL, (size_t)num_parse_threads * sizeof(float*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    thread_temp_nrm  = (float**)OS_AllocateRAM(NULL, (size_t)num_parse_threads * sizeof(float*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    thread_temp_uv   = (float**)OS_AllocateRAM(NULL, (size_t)num_parse_threads * sizeof(float*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    thread_out_verts = (float**)OS_AllocateRAM(NULL, (size_t)num_parse_threads * sizeof(float*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    thread_out_tris  = (uint32_t**)OS_AllocateRAM(NULL, (size_t)num_parse_threads * sizeof(uint32_t*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    for (int t = 0; t < num_parse_threads; ++t) {
        thread_temp_pos[t]  = (float*)OS_AllocateRAM(NULL, MAX_VERT_ELEMENT_SIZE * 3 * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
        thread_temp_nrm[t]  = (float*)OS_AllocateRAM(NULL, MAX_VERT_ELEMENT_SIZE * 3 * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
        thread_temp_uv[t]   = (float*)OS_AllocateRAM(NULL, MAX_VERT_ELEMENT_SIZE * 2 * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
        thread_out_verts[t] = (float*)OS_AllocateRAM(NULL, MAX_OUTPUT_VERTS * 8 * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
        thread_out_tris[t]  = (uint32_t*)OS_AllocateRAM(NULL, MAX_OUTPUT_VERTS * sizeof(uint32_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    }
    
    #pragma omp parallel for schedule(dynamic)
    for (uint32_t i = 0; i < loadedModelsMaxIndex; ++i) {
        int32_t parserIdx = indexToParser[i];
        if (unlikely(parserIdx < 0 || parserIdx >= (int32_t)mpars.count)) continue;

        modelHasAnimation[i] = (mpars.entries[parserIdx].entflags & ENTFLAG_ANIMATED);
        const char* data = rawModels[i].data;
        int file_size = rawModels[i].size;
        if (unlikely(!data || file_size <= 0)) continue;

        int tid = omp_get_thread_num();
        float minx, miny, minz, maxx, maxy, maxz;
        if (unlikely(!ParseOBJ(data, file_size, thread_temp_pos[tid], thread_temp_nrm[tid], thread_temp_uv[tid], thread_out_verts[tid], thread_out_tris[tid], &modelVertices[i], &modelVertexCounts[i], &modelTriangles[i], &modelTriangleCounts[i], &minx, &miny, &minz, &maxx, &maxy, &maxz))) continue;

        uint32_t base_idx = i * BOUNDS_ATTRIBUTES_COUNT;
        modelBounds[base_idx + BOUNDS_DATA_OFFSET_MINX] = minx;
        modelBounds[base_idx + BOUNDS_DATA_OFFSET_MINY] = miny;
        modelBounds[base_idx + BOUNDS_DATA_OFFSET_MINZ] = minz;
        modelBounds[base_idx + BOUNDS_DATA_OFFSET_MAXX] = maxx;
        modelBounds[base_idx + BOUNDS_DATA_OFFSET_MAXY] = maxy;
        modelBounds[base_idx + BOUNDS_DATA_OFFSET_MAXZ] = maxz;

        float r = vmax(0.0f, vabs(minx)); r = vmax(r, vabs(miny)); r = vmax(r, vabs(minz));
        r = vmax(r, maxx); r = vmax(r, maxy); r = vmax(r, maxz);
        modelBounds[base_idx + BOUNDS_DATA_OFFSET_RADIUS] = r;
    }

    DualLog(" took %f secs for second parallel loop...",get_time() - timeAtStartOfSecondLoop);
    for (int t = 0; t < num_parse_threads; ++t) {
        OS_DeallocateRAM(thread_temp_pos[t],  MAX_VERT_ELEMENT_SIZE * 3 * sizeof(float));
        OS_DeallocateRAM(thread_temp_nrm[t],  MAX_VERT_ELEMENT_SIZE * 3 * sizeof(float));
        OS_DeallocateRAM(thread_temp_uv[t],   MAX_VERT_ELEMENT_SIZE * 2 * sizeof(float));
        OS_DeallocateRAM(thread_out_verts[t], MAX_OUTPUT_VERTS * 8 * sizeof(float));
        OS_DeallocateRAM(thread_out_tris[t],  MAX_OUTPUT_VERTS * sizeof(uint32_t));
    }
    OS_DeallocateRAM(thread_temp_pos,  (size_t)num_parse_threads * sizeof(float*));
    OS_DeallocateRAM(thread_temp_nrm,  (size_t)num_parse_threads * sizeof(float*));
    OS_DeallocateRAM(thread_temp_uv,   (size_t)num_parse_threads * sizeof(float*));
    OS_DeallocateRAM(thread_out_verts, (size_t)num_parse_threads * sizeof(float*));
    OS_DeallocateRAM(thread_out_tris,  (size_t)num_parse_threads * sizeof(uint32_t*));
    for (uint32_t i = 0; i < loadedModelsMaxIndex; ++i) {
        if (rawModels[i].data) OS_DeallocateRAM((void*)rawModels[i].data, (size_t)rawModels[i].size);
    }
    
    DebugRAM("after model load loop");
    OS_DeallocateRAM(indexToParser,indexToParser_size);
    glGenBuffers(loadedModelsMaxIndex, Sys_Render.vbos);
    glGenBuffers(loadedModelsMaxIndex, Sys_Render.tbos);
    uint32_t totalVertices = 0, totalTris = 0;
    for (int i = 0; i < loadedModelsMaxIndex; ++i) {
        if (unlikely(modelVertexCounts[i] == 0)) continue;

        size_t vertSize = modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
        totalVertices += modelVertexCounts[i];
        size_t triSize  = modelTriangleCounts[i] * 3 * sizeof(uint32_t);
        totalTris += (uint32_t)triSize;
        glBindBuffer(GL_ARRAY_BUFFER, Sys_Render.vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertSize, NULL, GL_STATIC_DRAW);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, (GLsizeiptr)vertSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        __builtin_memcpy(ptr, modelVertices[i], vertSize);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Sys_Render.tbos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)triSize, NULL, GL_STATIC_DRAW);
        ptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, (GLsizeiptr)triSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        __builtin_memcpy(ptr, modelTriangles[i], triSize);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    DebugRAM("after to model to gpu transfer");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glFlush(); glFinish();
    OS_DeallocateRAM(mpars.entries,mpars.count * sizeof(Entity));
    DualLog(" total vertices: %u, total tris: %u, took %f secs\n", totalVertices, totalTris, get_time() - start_time);
    DebugRAM("After Load Models");
}
