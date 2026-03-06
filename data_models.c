// data_models.c - Load 3D Models
#include "os.h" // Operating System calls shim layer.
#include <omp.h>
#include "voxen.h"
float** modelVertices = NULL;
uint32_t** modelTriangles = NULL;
uint32_t modelVertexCounts[MODEL_IDX_MAX] = {0};
uint32_t modelTriangleCounts[MODEL_IDX_MAX] = {0};
bool modelHasAnimation[MODEL_IDX_MAX] = {0};
float modelBounds[MODEL_IDX_MAX * BOUNDS_ATTRIBUTES_COUNT] = {0};
uint16_t loadedModelsMaxIndex = 0;
GLuint SetupSSBO(GLuint* id, GLuint bindingIndex, GLsizeiptr size, const void* data, GLenum usage);

static inline __attribute__((always_inline)) uint32_t parse_numberu32_pure(const char* str) {
    if (str == 0 || *str == '\0') return 0;
    
    while (CharacterIsEmpty((char)*str)) str++;
    while (CharacterIsEmpty(*str)) str++;
    if (*str == '+') str++;
    if (*str == '-') return 0;
    
    unsigned long result = 0;
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        result = result * 10uL + (unsigned long)digit;
        str++;
    }

    return (uint32_t)result;
}

static inline __attribute__((always_inline)) float parse_float_pure(const char* str) {
    if (str == 0 || *str == '\0') return 0.0f;
    
    while (CharacterIsEmpty(*str)) str++;
    bool negative = false;
    if (*str == '-') { negative = true; str++; }
    else if (*str == '+') { str++; }

    double value = 0.0;
    bool has_digit = false;
    while (*str >= '0' && *str <= '9') { // Integer part
        value = value * 10.0 + (*str - '0');
        str++;
        has_digit = true;
    }

    if (*str == '.') { // Decimal part
        str++;
        double frac = 0.0;
        double place = 0.1;
        while (*str >= '0' && *str <= '9') {
            frac += (*str - '0') * place;
            place *= 0.1;
            str++;
            has_digit = true;
        }

        value += frac;
    }

    if (!has_digit) return 0.0f;

    if (negative) value = -value;
    return (float)value;
}

bool LoadOBJ(const char* filepath, float** out_vertices, uint32_t* out_vertex_count, uint32_t** out_triangles, uint32_t* out_triangle_count, float* out_minx, float* out_miny, float* out_minz, float* out_maxx, float* out_maxy, float* out_maxz) {
    *out_vertices = NULL;
    *out_triangles = NULL;
    *out_vertex_count = 0;
    *out_triangle_count = 0;
    OsFileHandle dummy_fd;
    int file_size = 0;
    const char* data = (const char*)OS_OpenAndAllocateFileBufferReadonly(filepath, &dummy_fd, &file_size);
    if (!data || file_size <= 0) return false;

    // Pass 1: count
    uint32_t vcount = 0, vtcount = 0, vncount = 0, fcount = 0;
    const char* p = data;
    while (p < data + file_size) {
        const char* line_start = p;
        while (p < data + file_size && *p != '\n' && *p != '\r') ++p;
        uint32_t len = (uint32_t)(p - line_start);
        if (len > 2) {
            if (line_start[0] == 'v') {
                if (line_start[1] == ' ')      vcount++;
                else if (line_start[1] == 't') vtcount++;
                else if (line_start[1] == 'n') vncount++;
            }
            else if (line_start[0] == 'f') fcount++;
        }
        
        if (p < data + file_size && *p == '\r') ++p;
        if (p < data + file_size && *p == '\n') ++p;
    }

    if (vcount == 0) { OS_DeallocateRAM((void*)data, (size_t)file_size); return false; }

    if (!vtcount) vtcount = vcount;
    if (!vncount) vncount = vcount;
    float* temp_pos = (float*)OS_AllocateRAM(NULL, vcount * 3 * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    float* temp_nrm = (float*)OS_AllocateRAM(NULL, vncount * 3 * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    float* temp_uv  = (float*)OS_AllocateRAM(NULL, vtcount * 2 * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);

    // Pass 2: vertices
    uint32_t vi = 0, ni = 0, ui = 0;
    uint32_t current_line = 0;
    p = data;
    while (p < data + file_size) {
        const char* line_start = p;
        while (p < data + file_size && *p != '\n' && *p != '\r') ++p;
        uint32_t len = (uint32_t)(p - line_start);

        if (len > 2 && line_start[0] == 'v') {
            const char* num = line_start + 2;
            if (line_start[1] == ' ') {
                temp_pos[vi*3+0] = parse_float_pure(num);
                while (*num && *num != ' ' && *num != '\t') ++num;
                ++num;
                temp_pos[vi*3+1] = parse_float_pure(num);
                while (*num && *num != ' ' && *num != '\t') ++num;
                ++num;
                temp_pos[vi*3+2] = parse_float_pure(num);
                vi++;
            } else if (line_start[1] == 'n' && ni < vncount) {
                num = line_start + 3;
                temp_nrm[ni*3+0] = parse_float_pure(num);
                while (*num && *num != ' ' && *num != '\t') ++num;
                ++num;
                temp_nrm[ni*3+1] = parse_float_pure(num);
                while (*num && *num != ' ' && *num != '\t') ++num;
                ++num;
                temp_nrm[ni*3+2] = parse_float_pure(num);
                ni++;
            } else if (line_start[1] == 't' && ui < vtcount) {
                num = line_start + 3;
                temp_uv[ui*2+0] = parse_float_pure(num);
                while (*num && *num != ' ' && *num != '\t') ++num;
                ++num;
                temp_uv[ui*2+1] = parse_float_pure(num);
                ui++;
            }
        }
        if (p < data + file_size && *p == '\r') ++p;
        if (p < data + file_size && *p == '\n') ++p;
        current_line++;
    }

    // Pass 3: faces
    size_t max_verts = (size_t)fcount * 3;
    float* final_verts = (float*)OS_AllocateRAM(NULL, max_verts * 8 * sizeof(float), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    uint32_t* final_tris = (uint32_t*)OS_AllocateRAM(NULL, (size_t)fcount * 3 * sizeof(uint32_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);

    uint32_t vert_idx = 0, tri_idx = 0;
    float minx = 1e9f, miny = 1e9f, minz = 1e9f;
    float maxx = -1e9f, maxy = -1e9f, maxz = -1e9f;

    current_line = 0;
    p = data;
    while (p < data + file_size) {
        const char* line_start = p;
        while (p < data + file_size && *p != '\n' && *p != '\r') ++p;
        uint32_t len = (uint32_t)(p - line_start);

        if (len > 2 && line_start[0] == 'f') {
            const char* num = line_start + 2;
            while (*num == ' ' || *num == '\t') ++num;

            uint32_t v[3] = {0}, vt[3] = {0}, vn[3] = {0};
            uint32_t idx_count = 0;

            for (int i = 0; i < 3 && idx_count < 3; ++i) {
                if (*num < '0' || (*num > '9' && *num != '-')) break;

                v[i] = parse_numberu32_pure(num);
                while (*num && *num != ' ' && *num != '\t' && *num != '/') ++num;
                if (*num == '/') {
                    ++num;
                    if (*num == '/') { ++num; vn[i] = parse_numberu32_pure(num); }
                    else {
                        vt[i] = parse_numberu32_pure(num);
                        while (*num && *num != ' ' && *num != '\t' && *num != '/') ++num;
                        if (*num == '/') { ++num; vn[i] = parse_numberu32_pure(num); }
                    }
                }
                while (*num && *num != ' ' && *num != '\t') ++num;
                while (*num == ' ' || *num == '\t') ++num;
                idx_count++;
            }

            if (idx_count != 3) goto next_line; // skip quads/ngons

            for (int i = 0; i < 3; ++i) {
                uint32_t vi_idx = v[i] - 1;
                uint32_t ti_idx = (vt[i] > 0 && vt[i] <= ui) ? vt[i]-1 : 0;
                uint32_t ni_idx = (vn[i] > 0 && vn[i] <= ni) ? vn[i]-1 : 0;
                float* dst = final_verts + vert_idx * 8;
                dst[0] = temp_pos[vi_idx*3];   dst[1] = temp_pos[vi_idx*3+1];   dst[2] = temp_pos[vi_idx*3+2];
                dst[3] = (ni_idx < ni) ? temp_nrm[ni_idx*3]   : 0.0f;
                dst[4] = (ni_idx < ni) ? temp_nrm[ni_idx*3+1] : 0.0f;
                dst[5] = (ni_idx < ni) ? temp_nrm[ni_idx*3+2] : 0.0f;
                dst[6] = (ti_idx < ui) ? temp_uv[ti_idx*2]    : 0.0f;
                dst[7] = (ti_idx < ui) ? temp_uv[ti_idx*2+1]  : 0.0f;
                float x = dst[0], y = dst[1], z = dst[2];
                if (x < minx) minx = x;
                if (x > maxx) maxx = x;
                if (y < miny) miny = y;
                if (y > maxy) maxy = y;
                if (z < minz) minz = z;
                if (z > maxz) maxz = z;
                final_tris[tri_idx*3 + i] = vert_idx;
                vert_idx++;
            }
            tri_idx++;
        }

    next_line:
        if (p < data + file_size && *p == '\r') ++p;
        if (p < data + file_size && *p == '\n') ++p;
        current_line++;
    }

    OS_DeallocateRAM((void*)data, (size_t)file_size);
    OS_DeallocateRAM(temp_pos, vcount * 3 * sizeof(float));
    OS_DeallocateRAM(temp_nrm, vncount * 3 * sizeof(float));
    OS_DeallocateRAM(temp_uv,  vtcount * 2 * sizeof(float));
    *out_vertices      = final_verts;
    *out_vertex_count  = vert_idx;
    *out_triangles     = final_tris;
    *out_triangle_count = tri_idx;
    *out_minx = minx; *out_miny = miny; *out_minz = minz;
    *out_maxx = maxx; *out_maxy = maxy; *out_maxz = maxz;
    return true;
}

void LoadModels(void) {
    if (loadedModelsMaxIndex > 0) return;
    double start_time = get_time();
    DataParser mpars;
    if (!parse_data_file(&mpars, MODEL_IDX_MAX, "./Data/models.txt")) { DualLogError("Could not parse ./Data/models.txt!\n"); OS_Exit(1); }

    int32_t maxIndex = -1;
    for (uint32_t k = 0; k < mpars.count; k++) {
        if (mpars.entries[k].index > maxIndex && mpars.entries[k].index != UINT16_MAX) maxIndex = mpars.entries[k].index;
    }
    
    loadedModelsMaxIndex = (uint16_t)maxIndex + 1U;
    DualLog("Loading models( %d/%d) with max index %d ...", loadedModelsMaxIndex, mpars.count, maxIndex);
    modelVertices = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(float*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    modelTriangles = OS_AllocateRAM(NULL, loadedModelsMaxIndex * sizeof(uint32_t*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
    size_t indexToParser_size = loadedModelsMaxIndex * sizeof(int32_t);
    int32_t* indexToParser = OS_AllocateRAM(NULL, indexToParser_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
    __builtin_memset(indexToParser, -1, indexToParser_size);
    for (uint32_t k = 0; k < mpars.count; k++) {
        if (mpars.entries[k].index != UINT16_MAX) indexToParser[mpars.entries[k].index] = (int32_t)k;
    }

//     #pragma omp parallel for schedule(dynamic)
    for (uint32_t i = 0; i < loadedModelsMaxIndex; ++i) {
        int32_t parserIdx = indexToParser[i];
        if (parserIdx < 0 || parserIdx >= (int32_t)mpars.count) continue;
        
        modelHasAnimation[i] = (mpars.entries[parserIdx].entflags & ENTFLAG_ANIMATED);
        const char* base = mpars.entries[parserIdx].path;
        float minx, miny, minz, maxx, maxy, maxz;
        if (!LoadOBJ(base, &modelVertices[i], &modelVertexCounts[i], &modelTriangles[i], &modelTriangleCounts[i], &minx, &miny, &minz, &maxx, &maxy, &maxz)) continue;
        
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

    OS_DeallocateRAM(indexToParser, indexToParser_size);
    OS_DeallocateRAM(mpars.entries, mpars.count * sizeof(Entity));
    glGenBuffers(loadedModelsMaxIndex, Sys_Render.vbos);
    glGenBuffers(loadedModelsMaxIndex, Sys_Render.tbos);
    uint32_t totalVertices = 0, totalTris = 0;
    for (int i = 0; i < loadedModelsMaxIndex; ++i) {
        if (modelVertexCounts[i] == 0) continue;
        size_t vertSize = modelVertexCounts[i] * VERTEX_ATTRIBUTES_COUNT * sizeof(float);
        totalVertices += modelVertexCounts[i];
        size_t triSize = modelTriangleCounts[i] * 3 * sizeof(uint32_t);
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
        glFlush(); glFinish();
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glFlush(); glFinish();
    DualLog(" total vertices: %u, total tris: %u, took %f secs\n", totalVertices, totalTris, get_time() - start_time);
}
