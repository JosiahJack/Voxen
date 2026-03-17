// data_models.c - Load 3D Models
#include "os.h"
#include "gl.h"
#include "voxen.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

float** modelVertices = NULL;
uint32_t** modelTriangles = NULL;
uint32_t modelVertexCounts[MODEL_IDX_MAX] = {0};
uint32_t modelTriangleCounts[MODEL_IDX_MAX] = {0};
bool modelHasAnimation[MODEL_IDX_MAX] = {0};
float modelBounds[MODEL_IDX_MAX * BOUNDS_ATTRIBUTES_COUNT] = {0};
uint16_t loadedModelsMaxIndex = 0;
GLuint SetupSSBO(GLuint* id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage);
#define MAX_VERT_ELEMENT_SIZE 6964
#define MAX_OUTPUT_VERTS      20892
static float**    thread_temp_pos   = NULL;
static float**    thread_temp_nrm   = NULL;
static float**    thread_temp_uv    = NULL;
static float**    thread_out_verts  = NULL;
static uint32_t** thread_out_tris   = NULL;
static int        num_parse_threads = 0;
typedef struct { const char* data; int size; } RawOBJ;

static inline float fast_atof(const char** p) { // Changed to pointer to pointer for pointer bumping
    float value = 0.0f, sign = 1.0f;
    while (**p == ' ' || **p == '\t') (*p)++;
    if (**p == '-') { sign = -1.0f; (*p)++; }
    while (**p >= '0' && **p <= '9') value = value * 10.0f + (*(*p)++ - '0');
    if (**p == '.') {
        (*p)++;
        float sub = 0.1f;
        while (**p >= '0' && **p <= '9') { value += (*(*p)++ - '0') * sub; sub *= 0.1f; }
    }
    return sign * value;
}

static inline int32_t fast_atoi(const char** p) {
    int32_t val = 0; int32_t sign = 1;
    while (**p == ' ' || **p == '\t') (*p)++;
    if (**p == '-') { sign = -1; (*p)++; }
    while (**p >= '0' && **p <= '9') val = val * 10 + (*(*p)++ - '0');
    return val * sign;
}

static __attribute__((hot)) __attribute__((flatten)) bool ParseOBJ(const char* __restrict data, int file_size, float* __restrict temp_pos, float* __restrict temp_nrm, float* __restrict temp_uv, float* __restrict scratch_verts, uint32_t* __restrict scratch_tris, float** out_vertices, uint32_t* out_vertex_count, uint32_t** out_triangles, uint32_t* out_triangle_count, float* out_minx, float* out_miny, float* out_minz, float* out_maxx, float* out_maxy, float* out_maxz) {
    *out_vertices = NULL; *out_triangles = NULL;
    *out_vertex_count = *out_triangle_count = 0;
    if (unlikely(!data || file_size <= 0)) return false;

    uint32_t pos_count = 0, norm_count = 0, uv_count = 0;
    uint32_t expanded_count = 0;
    float minx = 1e9f, miny = 1e9f, minz = 1e9f;
    float maxx = -1e9f, maxy = -1e9f, maxz = -1e9f;
    const char* p = data;
    const char* const end = data + file_size;
    while (likely(p < end)) {
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) ++p;
        if (p >= end) break;

        if (*p == '#') {
            while (p < end && *p != '\n') ++p;
            continue;
        }

        if (*p == 'v') {
            ++p;
            if (*p == ' ') {
                if (unlikely(pos_count >= MAX_VERT_ELEMENT_SIZE)) return false;
                ++p;
                temp_pos[pos_count*3]   = fast_atof(&p);
                temp_pos[pos_count*3+1] = fast_atof(&p);
                temp_pos[pos_count*3+2] = fast_atof(&p);
                ++pos_count;
            } else if (*p == 'n' && p[1] == ' ') {
                if (unlikely(norm_count >= MAX_VERT_ELEMENT_SIZE)) return false;
                p += 2;
                temp_nrm[norm_count*3]   = fast_atof(&p);
                temp_nrm[norm_count*3+1] = fast_atof(&p);
                temp_nrm[norm_count*3+2] = fast_atof(&p);
                ++norm_count;
            } else if (*p == 't' && p[1] == ' ') {
                if (unlikely(uv_count >= MAX_VERT_ELEMENT_SIZE)) return false;
                p += 2;
                temp_uv[uv_count*2]   = fast_atof(&p);
                temp_uv[uv_count*2+1] = fast_atof(&p);
                ++uv_count;
            }
        } else if (*p == 'f' && p[1] == ' ') {
            p += 2;
            uint32_t vert_ids[3] = {0}, tex_ids[3] = {0}, norm_ids[3] = {0};
            for (int k = 0; k < 3; ++k) {
                long raw = fast_atoi(&p);
                uint32_t vidx = (raw > 0) ? (uint32_t)raw : (raw < 0) ? (uint32_t)((int32_t)pos_count + raw) : 0;
                vert_ids[k] = vidx;

                if (*p == '/') {
                    ++p;
                    if (*p != '/') {
                        raw = fast_atoi(&p);
                        uint32_t tidx = (raw > 0) ? (uint32_t)raw : (raw < 0) ? (uint32_t)((int32_t)uv_count + raw) : 0;
                        tex_ids[k] = tidx;
                    }
                    if (*p == '/') {
                        ++p;
                        raw = fast_atoi(&p);
                        uint32_t nidx = (raw > 0) ? (uint32_t)raw : (raw < 0) ? (uint32_t)((int32_t)norm_count + raw) : 0;
                        norm_ids[k] = nidx;
                    }
                }
            }
            if (likely(vert_ids[0] && vert_ids[1] && vert_ids[2])) {
                if (unlikely(expanded_count + 3 > MAX_OUTPUT_VERTS)) return false;
                for (int k = 0; k < 3; ++k) {
                    uint32_t vi_idx = vert_ids[k] - 1;
                    uint32_t ti_idx = (tex_ids[k] && tex_ids[k] <= uv_count) ? tex_ids[k] - 1 : 0;
                    uint32_t ni_idx = (norm_ids[k] && norm_ids[k] <= norm_count) ? norm_ids[k] - 1 : 0;
                    float* dst = scratch_verts + (expanded_count << 3);
                    dst[0] = temp_pos[vi_idx*3];   dst[1] = temp_pos[vi_idx*3+1];   dst[2] = temp_pos[vi_idx*3+2];
                    dst[3] = (ni_idx < norm_count) ? temp_nrm[ni_idx*3]   : 0.0f;
                    dst[4] = (ni_idx < norm_count) ? temp_nrm[ni_idx*3+1] : 0.0f;
                    dst[5] = (ni_idx < norm_count) ? temp_nrm[ni_idx*3+2] : 0.0f;
                    dst[6] = (ti_idx < uv_count)   ? temp_uv[ti_idx*2]    : 0.0f;
                    dst[7] = (ti_idx < uv_count)   ? temp_uv[ti_idx*2+1]  : 0.0f;
                    float x = dst[0], y = dst[1], z = dst[2];
                    minx = (x < minx) ? x : minx; maxx = (x > maxx) ? x : maxx;
                    miny = (y < miny) ? y : miny; maxy = (y > maxy) ? y : maxy;
                    minz = (z < minz) ? z : minz; maxz = (z > maxz) ? z : maxz;
                    scratch_tris[expanded_count] = expanded_count;
                    ++expanded_count;
                }
            }
        } else {
            while (p < end && *p != '\n') ++p;
        }
    }

    if (unlikely(expanded_count == 0)) return false;

    #define HASH_SIZE 32768
    uint32_t hash_table[HASH_SIZE];
    __builtin_memset(hash_table, 0xFF, sizeof(hash_table));
    float* unique_verts = scratch_verts;
    uint32_t* remap = (uint32_t*)scratch_tris;
    uint32_t unique_cnt = 0;
    for (uint32_t i = 0; i < expanded_count; ++i) {
        const float* v = scratch_verts + (i << 3);
        uint64_t h = *(uint64_t*)(v+0) ^ *(uint64_t*)(v+2) ^ *(uint64_t*)(v+4) ^ *(uint64_t*)(v+6);
        uint32_t slot = (uint32_t)(h ^ (h >> 32)) & (HASH_SIZE-1);
        while (hash_table[slot] != 0xFFFFFFFF) {
            const float* candidate = unique_verts + (hash_table[slot] << 3);
            if (__builtin_memcmp(candidate, v, 32) == 0) {
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
    size_t ibytes = (size_t)expanded_count * sizeof(uint32_t);
    uint32_t* final_tris = (uint32_t*)OS_AllocateRAM(NULL, ibytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    for (uint32_t i = 0; i < expanded_count; ++i) final_tris[i] = remap[i];
    *out_vertices       = final_verts;
    *out_vertex_count   = unique_cnt;
    *out_triangles      = final_tris;
    *out_triangle_count = expanded_count / 3;
    *out_minx = minx; *out_miny = miny; *out_minz = minz;
    *out_maxx = maxx; *out_maxy = maxy; *out_maxz = maxz;
    return true;
}

typedef struct ModelParseTask {
    uint32_t start_model;
    uint32_t end_model;
    RawOBJ* raw_models;
    int32_t* index_to_parser;
    const DataParser* data_parser;
    int thread_id;
} ModelParseTask;

static void* ModelParsingWorker(void* argument) {
    ModelParseTask* task = (ModelParseTask*)argument;
    for (uint32_t current_model = task->start_model; current_model < task->end_model; ++current_model) {
        int32_t parser_index = task->index_to_parser[current_model];
        if (unlikely(parser_index < 0 || parser_index >= (int32_t)task->data_parser->count)) continue;

        modelHasAnimation[current_model] = (task->data_parser->entries[parser_index].entflags & ENTFLAG_ANIMATED);
        const char* model_data = task->raw_models[current_model].data;
        int model_file_size = task->raw_models[current_model].size;
        if (unlikely(!model_data || model_file_size <= 0)) continue;

        int tid = task->thread_id;
        float min_x, min_y, min_z, max_x, max_y, max_z;
        if (unlikely(!ParseOBJ(model_data, model_file_size, thread_temp_pos[tid], thread_temp_nrm[tid], thread_temp_uv[tid], thread_out_verts[tid], thread_out_tris[tid], &modelVertices[current_model], &modelVertexCounts[current_model], &modelTriangles[current_model], &modelTriangleCounts[current_model], &min_x, &min_y, &min_z, &max_x, &max_y, &max_z))) continue;

        uint32_t bounds_base = current_model * BOUNDS_ATTRIBUTES_COUNT;
        modelBounds[bounds_base + BOUNDS_DATA_OFFSET_MINX] = min_x;
        modelBounds[bounds_base + BOUNDS_DATA_OFFSET_MINY] = min_y;
        modelBounds[bounds_base + BOUNDS_DATA_OFFSET_MINZ] = min_z;
        modelBounds[bounds_base + BOUNDS_DATA_OFFSET_MAXX] = max_x;
        modelBounds[bounds_base + BOUNDS_DATA_OFFSET_MAXY] = max_y;
        modelBounds[bounds_base + BOUNDS_DATA_OFFSET_MAXZ] = max_z;
        float radius = vmax(0.0f, vabs(min_x));
        radius = vmax(radius, vabs(min_y));
        radius = vmax(radius, vabs(min_z));
        radius = vmax(radius, max_x);
        radius = vmax(radius, max_y);
        radius = vmax(radius, max_z);
        modelBounds[bounds_base + BOUNDS_DATA_OFFSET_RADIUS] = radius;
    }
    return NULL;
}

void LoadModels(void) {
    if (unlikely(loadedModelsMaxIndex > 0)) return;

    double start_time = get_time();
    DataParser mpars;
    if (unlikely(!parse_data_file(&mpars, MODEL_IDX_MAX, "./Data/models.txt"))) { DualLogError("Could not parse ./Data/models.txt!\n"); OS_Exit(1); }

    int32_t max_index = -1;
    for (uint32_t k = 0; k < mpars.count; ++k) {
        if (mpars.entries[k].index > max_index && mpars.entries[k].index != UINT16_MAX) max_index = mpars.entries[k].index;
    }

    loadedModelsMaxIndex = (uint16_t)max_index + 1U;
    DualLog("Loading models (%d) ...",mpars.count);
    modelVertices  = OS_AllocateRAM(NULL,loadedModelsMaxIndex * sizeof(float*),   PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    modelTriangles = OS_AllocateRAM(NULL,loadedModelsMaxIndex * sizeof(uint32_t*),PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    size_t index_map_size = loadedModelsMaxIndex * sizeof(int32_t);
    int32_t* index_to_parser = OS_AllocateRAM(NULL, index_map_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
    __builtin_memset(index_to_parser, -1, index_map_size);
    for (uint32_t k = 0; k < mpars.count; ++k) {
        if (likely(mpars.entries[k].index != UINT16_MAX)) index_to_parser[mpars.entries[k].index] = (int32_t)k;
    }

    RawOBJ* raw_models = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(RawOBJ), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    __builtin_memset(raw_models, 0, loadedModelsMaxIndex * sizeof(RawOBJ));
    for (uint32_t i = 0; i < loadedModelsMaxIndex; ++i) {
        int32_t parser_index = index_to_parser[i];
        if (unlikely(parser_index < 0 || parser_index >= (int32_t)mpars.count)) continue;
        const char* path = mpars.entries[parser_index].path;
        OsFileHandle dummy_fd;
        int size = 0;
        raw_models[i].data = (const char*)OS_OpenAndAllocateFileBufferReadonly(path, &dummy_fd, &size);
        raw_models[i].size = size;
    }

    double time_at_start_of_parse = get_time();
    num_parse_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (num_parse_threads < 1) num_parse_threads = 1;
    if (num_parse_threads > 32) num_parse_threads = 32; // Cap to the task size (avoids vla)
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

    ModelParseTask tasks[32];
    uint32_t chunk_size = (loadedModelsMaxIndex + (uint32_t)num_parse_threads - 1U) / (uint32_t)num_parse_threads;
    for (int t = 0; t < num_parse_threads; ++t) {
        tasks[t].start_model = (uint32_t)t * chunk_size;
        tasks[t].end_model = tasks[t].start_model + chunk_size;
        if (tasks[t].end_model > loadedModelsMaxIndex) tasks[t].end_model = loadedModelsMaxIndex;
        tasks[t].raw_models = raw_models;
        tasks[t].index_to_parser = index_to_parser;
        tasks[t].data_parser = &mpars;
        tasks[t].thread_id = t;
    }

    pthread_t worker_threads[32];
    for (int t = 0; t < num_parse_threads; ++t) pthread_create(&worker_threads[t],NULL,ModelParsingWorker,&tasks[t]);
    for (int t = 0; t < num_parse_threads; ++t) pthread_join(worker_threads[t],NULL);
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
        if (raw_models[i].data) OS_DeallocateRAM((void*)raw_models[i].data, (size_t)raw_models[i].size);
    }
    
    OS_DeallocateRAM(raw_models, loadedModelsMaxIndex * sizeof(RawOBJ));
    DebugRAM("after model load loop");
    OS_DeallocateRAM(index_to_parser, index_map_size);
    glGenBuffers(loadedModelsMaxIndex, Sys_Render.vbos);
    glGenBuffers(loadedModelsMaxIndex, Sys_Render.tbos);
    uint32_t total_vertices = 0, total_tris = 0;
    for (int i = 0; i < loadedModelsMaxIndex; ++i) {
        if (unlikely(modelVertexCounts[i] == 0)) continue;

        size_t vert_size = modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
        total_vertices += modelVertexCounts[i];
        size_t tri_size = modelTriangleCounts[i] * 3 * sizeof(uint32_t);
        total_tris += (uint32_t)tri_size;

        glBindBuffer(GL_ARRAY_BUFFER, Sys_Render.vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vert_size, NULL, GL_STATIC_DRAW);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, (GLsizeiptr)vert_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        __builtin_memcpy(ptr, modelVertices[i], vert_size);
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Sys_Render.tbos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)tri_size, NULL, GL_STATIC_DRAW);
        ptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, (GLsizeiptr)tri_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        __builtin_memcpy(ptr, modelTriangles[i], tri_size);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    DebugRAM("after to model to gpu transfer");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glFlush(); glFinish();
    OS_DeallocateRAM(mpars.entries, mpars.count * sizeof(Entity));
    DualLog(" total vertices: %u, total tris: %u, took %f secs\n", total_vertices, total_tris, get_time() - start_time);
    DebugRAM("After Load Models");
}
