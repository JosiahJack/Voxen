#include "os.h"
#include "gl.h"
#include "voxen.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>
uint32_t totalPixels;
uint32_t totalPaletteColors;
uint32_t* texturePaletteOffsets;
int32_t* textureSizes;
#define STBI_ARENA_SIZE 16*1024*1024
uint8_t* stbi__arena_base = NULL;
uint8_t* stbi__arena_cursor = NULL;
uint8_t* stbi__arena_end = NULL;
static int num_parse_threads = 0;
uint8_t* stbi_load_from_memory(const uint8_t* buffer, int32_t len, int32_t* x, int32_t* y);
static StbiArena* thread_stbi_arenas = NULL;
typedef struct { uint16_t index; bool transparent; bool doublesided; char path[128]; } TextureData;
typedef struct { TextureData* entries; uint32_t count; uint32_t capacity; } TextureDataParser;
typedef struct { const char* data; int size; } RawTexture;
typedef struct TextureParseTask { uint32_t start_tex; uint32_t end_tex; RawTexture* raw_textures; int32_t* index_to_parser; const TextureDataParser* parser; int tid; } TextureParseTask;
static uint8_t** textureIndexBuffers = NULL;
static uint32_t** texturePaletteBuffers = NULL;
static uint32_t* texturePaletteSizes = NULL;
static int32_t* textureWidths = NULL;
static int32_t* textureHeights = NULL;
void stbi__arena_init(void) {
    if (!stbi__arena_base) {
        stbi__arena_base = OS_AllocateRAM(NULL,STBI_ARENA_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
        stbi__arena_cursor = stbi__arena_base;
        stbi__arena_end = stbi__arena_base + STBI_ARENA_SIZE;
    }
}

void stbi__arena_init_thread(StbiArena* arena) {
    if (!arena->base) {
        arena->base = OS_AllocateRAM(NULL,STBI_ARENA_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
        arena->cursor = arena->base;
        arena->end = arena->base + STBI_ARENA_SIZE;
    }
}

void* stbi__arena_alloc(size_t size) {
    if (!stbi__arena_base) {
        DualLogError("stbi__arena_base was invalid\n");
        return NULL;
    }

    uint8_t* aligned = stbi__arena_cursor;
    if (aligned + size > stbi__arena_end) {
        DualLogError("stbi__arena_alloc failed buffer overflowed with %zu vs %zu\n",(size_t)aligned + size,(size_t)stbi__arena_end);
        return NULL;
    }

    stbi__arena_cursor = aligned + size;
    return aligned;
}

void* stbi__arena_alloc_thread(StbiArena* arena, size_t size) {
    if (!arena->base) return NULL;
    uint8_t* aligned = arena->cursor;
    if (aligned + size > arena->end) return NULL;
    arena->cursor = aligned + size;
    return aligned;
}

typedef struct {
    uint32_t img_x, img_y;
    int32_t img_n, img_out_n;
    uint8_t* img_buffer, *img_buffer_end;
} stbi__context;

typedef struct {
    stbi__context* s;
    uint8_t* idata, *expanded, *out;
} stbi__png;

enum {
    STBI__F_none = 0,
    STBI__F_sub = 1,
    STBI__F_up = 2,
    STBI__F_avg = 3,
    STBI__F_paeth = 4,
    STBI__F_avg_first,
    STBI__F_paeth_first
};

inline static uint32_t stbi__get32be(stbi__context* s) {
    const uint8_t* p = s->img_buffer;
    s->img_buffer += 4;
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

typedef struct {
    uint16_t fast[1 << 9];
    uint16_t firstcode[16];
    int32_t maxcode[17];
    uint16_t firstsymbol[16];
    uint8_t size[288];
    uint16_t value[288];
} stbi__zhuffman;

typedef struct {
    uint8_t* zbuffer, *zbuffer_end;
    int32_t num_bits;
    uint32_t code_buffer;
    uint8_t* zout;
    uint8_t* zout_start;
    stbi__zhuffman z_length, z_distance;
} stbi__zbuf;

inline static int32_t stbi__bit_reverse(int32_t n, int32_t bits) {
    n = ((n & 0xAAAA) >> 1) | ((n & 0x5555) << 1);
    n = ((n & 0xCCCC) >> 2) | ((n & 0x3333) << 2);
    n = ((n & 0xF0F0) >> 4) | ((n & 0x0F0F) << 4);
    n = ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
    return n >> (16 - bits);
}

static int32_t stbi__zbuild_huffman(stbi__zhuffman* z, const uint8_t* sizelist, int32_t num) {
    int32_t i, k = 0, next_code[16], sizes[17];
    __builtin_memset(sizes, 0, sizeof(sizes));
    __builtin_memset(z->fast, 0, sizeof(z->fast));
    if (num != 32) {
        for (i = 0; i < num; ++i) ++sizes[sizelist[i]];
    }

    sizes[0] = 0;
    for (i = 1; i < 16; ++i) {
        if (sizes[i] > (1 << i)) return 0;
    }

    int32_t code = 0;
    for (i = 1; i < 16; ++i) {
        next_code[i] = code;
        z->firstcode[i] = (uint16_t)code;
        z->firstsymbol[i] = (uint16_t)k;
        code = (code + sizes[i]);
        if (sizes[i]) {
            if (code - 1 >= (1 << i)) return 0;
        }

        z->maxcode[i] = code << (16 - i);
        code <<= 1;
        k += sizes[i];
    }

    z->maxcode[16] = 0x10000;
    for (i = 0; i < num; ++i) {
        int s = num == 32 ? 5 : sizelist[i];
        if (s) {
            int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
            uint16_t fastv = (uint16_t)((s << 9) | i);
            z->size[c] = (unsigned char)s;
            z->value[c] = (uint16_t)i;
            if (s <= 9) {
                int j = stbi__bit_reverse(next_code[s], s);
                while (j < (1 << 9)) {
                    z->fast[j] = fastv;
                    j += (1 << s);
                }
            }
            ++next_code[s];
        }
    }
    return 1;
}

inline static uint32_t stbi__zreceive(stbi__zbuf* z, int n) {
    if (z->num_bits < n) {
        #pragma GCC unroll 24
        do {
            z->code_buffer |= (unsigned int)(*z->zbuffer++) << z->num_bits;
            z->num_bits += 8;
        } while (z->num_bits <= 24);
    }

    uint32_t k = z->code_buffer & ((1u << (uint32_t)n) - 1u);
    z->code_buffer >>= n;
    z->num_bits -= n;
    return k;
}

inline static uint32_t stbi__zhuffman_decode(stbi__zbuf* a, stbi__zhuffman* z) {
    int s;
    if (a->num_bits < 16) {
        #pragma GCC unroll 24
        do {
            a->code_buffer |= (unsigned int)(*a->zbuffer++) << a->num_bits;
            a->num_bits += 8;
        } while (a->num_bits <= 24);
    }
    int b = z->fast[a->code_buffer & ((1 << 9) - 1)];
    if (b) {
        s = b >> 9;
        a->code_buffer >>= s;
        a->num_bits -= s;
        return b & 511;
    }

    int k = stbi__bit_reverse((int32_t)a->code_buffer, 16);
    for (s = 10;; ++s) {
        if (k < z->maxcode[s]) break;
    }
    b = (k >> (16 - s)) - z->firstcode[s] + z->firstsymbol[s];
    a->code_buffer >>= s;
    a->num_bits -= s;
    return (uint32_t)z->value[b];
}

static const int stbi__zlength_base[31] = {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258,0,0};
static const int stbi__zlength_extra[31] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0};
static const int stbi__zdist_base[32] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
static const int stbi__zdist_extra[32] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int stbi__parse_huffman_block(stbi__zbuf* a) {
    uint8_t* zout = a->zout;
    for (;;) {
        int z = stbi__zhuffman_decode(a, &a->z_length);
        if (z < 256) {
            *zout++ = (uint8_t)z;
        } else {
            int len, dist;
            if (z == 256) {
                a->zout = zout;
                return 1;
            }

            z -= 257;
            len = stbi__zlength_base[z];
            if (stbi__zlength_extra[z]) len += stbi__zreceive(a, stbi__zlength_extra[z]);
            z = stbi__zhuffman_decode(a, &a->z_distance);
            dist = stbi__zdist_base[z];
            if (stbi__zdist_extra[z]) dist += stbi__zreceive(a, stbi__zdist_extra[z]);
            uint8_t* p = (uint8_t*)(zout - dist);
            if (dist == 1) {
                uint8_t v = *p;
                if (len) {
                    do *zout++ = v;
                    while (--len);
                }
            } else {
                if (len) {
                    do *zout++ = *p++;
                    while (--len);
                }
            }
        }
    }
}

static int stbi__compute_huffman_codes(stbi__zbuf* a) {
    static const uint8_t length_dezigzag[19] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
    stbi__zhuffman z_codelength;
    uint8_t lencodes[286 + 32 + 137];
    uint8_t codelength_sizes[19];
    uint32_t hlit = stbi__zreceive(a, 5) + 257;
    uint32_t hdist = stbi__zreceive(a, 5) + 1;
    uint32_t hclen = stbi__zreceive(a, 4) + 4;
    uint32_t ntot = hlit + hdist;
    __builtin_memset(codelength_sizes, 0, sizeof(codelength_sizes));
    for (uint32_t i = 0; i < hclen; ++i) {
        uint32_t s = stbi__zreceive(a, 3);
        codelength_sizes[length_dezigzag[i]] = (uint8_t)s;
    }

    stbi__zbuild_huffman(&z_codelength, codelength_sizes, 19);
    uint32_t n = 0;
    while (n < ntot) {
        uint32_t c = stbi__zhuffman_decode(a, &z_codelength);
        if (c < 16) {
            lencodes[n++] = (uint8_t)c;
        } else {
            uint8_t fill = 0;
            if (c == 16) {
                c = stbi__zreceive(a, 2) + 3;
                fill = lencodes[n - 1];
            } else if (c == 17) {
                c = stbi__zreceive(a, 3) + 3;
            } else if (c == 18) {
                c = stbi__zreceive(a, 7) + 11;
            } else return 0;

            __builtin_memset(lencodes + n, fill, c);
            n += c;
        }
    }

    stbi__zbuild_huffman(&a->z_length, lencodes, hlit);
    stbi__zbuild_huffman(&a->z_distance, lencodes + hlit, hdist);
    return 1;
}

static int stbi__parse_uncompressed_block(stbi__zbuf* a) {
    uint8_t header[4];
    if (a->num_bits & 7) stbi__zreceive(a, a->num_bits & 7);
    int32_t k = 0;
    while (a->num_bits > 0) {
        header[k] = (uint8_t)(a->code_buffer & 255);
        a->code_buffer >>= 8;
        a->num_bits -= 8;
        ++k;
    }

    if (k <= 0) header[0] = *a->zbuffer++;
    if (k <= 1) header[1] = *a->zbuffer++;
    if (k <= 2) header[2] = *a->zbuffer++;
    if (k <= 3) header[3] = *a->zbuffer++;
    int32_t len = header[1] * 256 + header[0];
    __builtin_memcpy(a->zout, a->zbuffer, len);
    a->zbuffer += len;
    a->zout += len;
    return 1;
}

static const uint8_t stbi__zdefault_length[288] = {
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8
};

uint8_t* stbi_zlib_decode_malloc_guesssize_headerflag(const uint8_t* buffer, int32_t len, int32_t initial_size, int32_t* outlen) {
    stbi__zbuf a = {0};
    uint8_t* p = (uint8_t*)stbi__arena_alloc(initial_size);
    a.zbuffer = (uint8_t*)buffer;
    a.zbuffer_end = (uint8_t*)buffer + len;
    a.zout_start = p;
    a.zout = p;
    a.zbuffer++;
    a.zbuffer++;
    a.num_bits = 0;
    a.code_buffer = 0;
    int32_t finalOne, type;
    do {
        finalOne = stbi__zreceive(&a, 1);
        type = stbi__zreceive(&a, 2);
        if (type == 0) stbi__parse_uncompressed_block(&a);
        else {
            if (type == 1) {
                stbi__zbuild_huffman(&a.z_length, stbi__zdefault_length, 288);
                stbi__zbuild_huffman(&a.z_distance, NULL, 32);
            } else {
                stbi__compute_huffman_codes(&a);
            }
            stbi__parse_huffman_block(&a);
        }
    } while (!finalOne);

    if (outlen) *outlen = (int32_t)(a.zout - a.zout_start);
    return a.zout_start;
}

uint8_t* stbi_zlib_decode_malloc_guesssize_headerflag_arena(const uint8_t* buffer, int32_t len, int32_t initial_size, int32_t* outlen, StbiArena* arena) {
    stbi__zbuf a = {0};
    uint8_t* p = (uint8_t*)stbi__arena_alloc_thread(arena, initial_size);
    a.zbuffer = (uint8_t*)buffer;
    a.zbuffer_end = (uint8_t*)buffer + len;
    a.zout_start = a.zout = p;
    a.zbuffer += 2;
    a.num_bits = 0;
    a.code_buffer = 0;
    int32_t finalOne, type;
    do {
        finalOne = stbi__zreceive(&a, 1);
        type = stbi__zreceive(&a, 2);
        if (type == 0) stbi__parse_uncompressed_block(&a);
        else {
            if (type == 1) {
                stbi__zbuild_huffman(&a.z_length, stbi__zdefault_length, 288);
                stbi__zbuild_huffman(&a.z_distance, NULL, 32);
            } else stbi__compute_huffman_codes(&a);
            stbi__parse_huffman_block(&a);
        }
    } while (!finalOne);
    if (outlen) *outlen = (int32_t)(a.zout - a.zout_start);
    return a.zout_start;
}

static uint8_t first_row_filter[5] = {
    STBI__F_none,
    STBI__F_sub,
    STBI__F_none,
    STBI__F_avg_first,
    STBI__F_paeth_first
};

inline static int32_t stbi__paeth(int32_t a, int32_t b, int32_t c) {
    int32_t p = a + b - c;
    int32_t pa = vabs(p - a);
    int32_t pb = vabs(p - b);
    int32_t pc = vabs(p - c);
    if (pa <= pb && pa <= pc) return a;
    if (pb <= pc) return b;
    return c;
}

static int32_t stbi__create_png_image_raw_arena(StbiArena* arena, stbi__png* a, uint8_t* raw, uint32_t raw_len, int32_t out_n, uint32_t x, uint32_t y, int32_t img_n) {
    uint32_t i, stride = x * out_n;
    uint32_t img_len, img_width_bytes;
    int32_t k;
    int32_t output_bytes = out_n;
    int32_t filter_bytes = img_n;
    a->out = (uint8_t*)stbi__arena_alloc_thread(arena, (size_t)x * y * output_bytes);
    img_width_bytes = (((img_n * x * 8) + 7) >> 3);
    img_len = (img_width_bytes + 1) * y;
    if (raw_len < img_len) return 0;

    for (uint32_t j = 0; j < y; ++j) {
        uint8_t* cur = a->out + stride * j;
        uint8_t* prior;
        int filter = *raw++;
        if (filter > 4) return 0;

        if (j > 0) prior = cur - stride;
        else prior = a->out;

        if (j == 0) filter = first_row_filter[filter];
        for (k = 0; k < filter_bytes; ++k) {
            switch (filter) {
                case STBI__F_none: cur[k] = raw[k]; break;
                case STBI__F_sub: cur[k] = raw[k]; break;
                case STBI__F_up: cur[k] = (raw[k] + prior[k]); break;
                case STBI__F_avg: cur[k] = (raw[k] + (prior[k] >> 1)); break;
                case STBI__F_paeth: cur[k] = (raw[k] + stbi__paeth(0, prior[k], 0)); break;
                case STBI__F_avg_first: cur[k] = raw[k]; break;
            }
        }

        if (img_n != out_n) cur[img_n] = 255;
        raw += img_n;
        cur += out_n;
        prior += out_n;

        #define STBI__CASE(f) \
            case f: \
                for (i = x - 1; i >= 1; --i, cur[filter_bytes] = 255, raw += filter_bytes, cur += output_bytes, prior += output_bytes) \
                    for (k = 0; k < filter_bytes; ++k)
        switch (filter) {
            STBI__CASE(STBI__F_none) { cur[k] = raw[k]; } break;
            STBI__CASE(STBI__F_sub) { cur[k] = (raw[k] + cur[k - output_bytes]); } break;
            STBI__CASE(STBI__F_up) { cur[k] = (raw[k] + prior[k]); } break;
            STBI__CASE(STBI__F_avg) { cur[k] = (raw[k] + ((prior[k] + cur[k - output_bytes]) >> 1)); } break;
            STBI__CASE(STBI__F_paeth) { cur[k] = (raw[k] + stbi__paeth(cur[k - output_bytes], prior[k], prior[k - output_bytes])); } break;
            STBI__CASE(STBI__F_avg_first) { cur[k] = (raw[k] + (cur[k - output_bytes] >> 1)); } break;
        }
        #undef STBI__CASE
    }

    return 1;
}

static int32_t stbi__create_png_image_raw(stbi__png* a, uint8_t* raw, uint32_t raw_len, int32_t out_n, uint32_t x, uint32_t y, int32_t img_n) {
    uint32_t i, stride = x * out_n;
    uint32_t img_len, img_width_bytes;
    int32_t k;
    int32_t output_bytes = out_n;
    int32_t filter_bytes = img_n;
    a->out = (uint8_t*)stbi__arena_alloc(x * y * output_bytes);
    img_width_bytes = (((img_n * x * 8) + 7) >> 3);
    img_len = (img_width_bytes + 1) * y;
    if (raw_len < img_len) return 0;

    for (uint32_t j = 0; j < y; ++j) {
        uint8_t* cur = a->out + stride * j;
        uint8_t* prior;
        int filter = *raw++;
        if (filter > 4) return 0;

        if (j > 0) prior = cur - stride;
        else prior = a->out;

        if (j == 0) filter = first_row_filter[filter];
        for (k = 0; k < filter_bytes; ++k) {
            switch (filter) {
                case STBI__F_none: cur[k] = raw[k]; break;
                case STBI__F_sub: cur[k] = raw[k]; break;
                case STBI__F_up: cur[k] = (raw[k] + prior[k]); break;
                case STBI__F_avg: cur[k] = (raw[k] + (prior[k] >> 1)); break;
                case STBI__F_paeth: cur[k] = (raw[k] + stbi__paeth(0, prior[k], 0)); break;
                case STBI__F_avg_first: cur[k] = raw[k]; break;
            }
        }

        if (img_n != out_n) cur[img_n] = 255;
        raw += img_n;
        cur += out_n;
        prior += out_n;

        #define STBI__CASE(f) \
            case f: \
                for (i = x - 1; i >= 1; --i, cur[filter_bytes] = 255, raw += filter_bytes, cur += output_bytes, prior += output_bytes) \
                    for (k = 0; k < filter_bytes; ++k)
        switch (filter) {
            STBI__CASE(STBI__F_none) { cur[k] = raw[k]; } break;
            STBI__CASE(STBI__F_sub) { cur[k] = (raw[k] + cur[k - output_bytes]); } break;
            STBI__CASE(STBI__F_up) { cur[k] = (raw[k] + prior[k]); } break;
            STBI__CASE(STBI__F_avg) { cur[k] = (raw[k] + ((prior[k] + cur[k - output_bytes]) >> 1)); } break;
            STBI__CASE(STBI__F_paeth) { cur[k] = (raw[k] + stbi__paeth(cur[k - output_bytes], prior[k], prior[k - output_bytes])); } break;
            STBI__CASE(STBI__F_avg_first) { cur[k] = (raw[k] + (cur[k - output_bytes] >> 1)); } break;
        }
        #undef STBI__CASE
    }

    return 1;
}

uint8_t* stbi_load_from_memory_arena(const uint8_t* buffer, int len, int* x, int* y, StbiArena* arena) {
    if (arena->base) arena->cursor = arena->base;
    stbi__context s;
    s.img_n = s.img_out_n = 0;
    s.img_buffer = (uint8_t*)buffer;
    s.img_buffer_end = (uint8_t*)buffer + len;
    void* result = NULL;
    stbi__png z = {0};
    z.s = &s;
    uint32_t ioff = 0;
    z.expanded = z.idata = z.out = NULL;
    s.img_buffer += 8;
    s.img_x = s.img_y = 1;
    for (;;) {
        uint32_t length = stbi__get32be(&s);
        uint32_t type = stbi__get32be(&s);
        switch (type) {
            case 0x49484452: {
                s.img_x = stbi__get32be(&s);
                s.img_y = stbi__get32be(&s);
                s.img_buffer++;
                int32_t color = (*s.img_buffer++);
                s.img_buffer += 3;
                s.img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
                break;
            }

            case 0x49444154: {
                if (!z.idata) { z.idata = stbi__arena_alloc_thread(arena, len + 16); ioff = 0; }
                __builtin_memcpy(z.idata + ioff, s.img_buffer, length);
                s.img_buffer += length;
                ioff += length;
                break;
            }

            case 0x49454E44: {
                uint32_t bpl = (s.img_x);
                uint32_t raw_len = bpl * s.img_y * s.img_n + s.img_y;
                z.expanded = (uint8_t*)stbi_zlib_decode_malloc_guesssize_headerflag_arena((uint8_t*)z.idata, ioff, raw_len, (int32_t*)(&raw_len), arena);
                if (s.img_n + 1 == 4) s.img_out_n = s.img_n + 1;
                else s.img_out_n = s.img_n;

                stbi__create_png_image_raw_arena(arena, &z, z.expanded, raw_len, s.img_out_n, s.img_x, s.img_y, z.s->img_n);
                stbi__get32be(&s);
                goto Label_parsesuccess;
            }

            default: s.img_buffer += length; break;
        }

        stbi__get32be(&s);
    }

    Label_parsesuccess:
    result = z.out;
    z.out = NULL;
    *x = z.s->img_x;
    *y = z.s->img_y;
    return (unsigned char*)result;
}

extern uint8_t* stbi_load_from_memory(const uint8_t* buffer, int len, int* x, int* y) {
    if (stbi__arena_base) stbi__arena_cursor = stbi__arena_base;
    stbi__context s;
    s.img_n = s.img_out_n = 0;
    s.img_buffer = (uint8_t*)buffer;
    s.img_buffer_end = (uint8_t*)buffer + len;
    void* result = NULL;
    stbi__png z = {0};
    z.s = &s;
    uint32_t ioff = 0;
    z.expanded = z.idata = z.out = NULL;
    s.img_buffer += 8;
    s.img_x = s.img_y = 1;
    for (;;) {
        uint32_t length = stbi__get32be(&s);
        uint32_t type = stbi__get32be(&s);
        switch (type) {
            case 0x49484452: {
                s.img_x = stbi__get32be(&s);
                s.img_y = stbi__get32be(&s);
                s.img_buffer++;
                int32_t color = (*s.img_buffer++);
                s.img_buffer += 3;
                s.img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
                break;
            }

            case 0x49444154: {
                if (!z.idata) { z.idata = stbi__arena_alloc(len + 16); ioff = 0; }
                __builtin_memcpy(z.idata + ioff, s.img_buffer, length);
                s.img_buffer += length;
                ioff += length;
                break;
            }

            case 0x49454E44: {
                uint32_t bpl = (s.img_x);
                uint32_t raw_len = bpl * s.img_y * s.img_n + s.img_y;
                z.expanded = (uint8_t*)stbi_zlib_decode_malloc_guesssize_headerflag((uint8_t*)z.idata, ioff, raw_len, (int32_t*)(&raw_len));
                if (s.img_n + 1 == 4) s.img_out_n = s.img_n + 1;
                else s.img_out_n = s.img_n;

                stbi__create_png_image_raw(&z, z.expanded, raw_len, s.img_out_n, s.img_x, s.img_y, z.s->img_n);
                stbi__get32be(&s);
                goto Label_parsesuccess;
            }

            default: s.img_buffer += length; break;
        }

        stbi__get32be(&s);
    }

    Label_parsesuccess:
    result = z.out;
    z.out = NULL;
    *x = z.s->img_x;
    *y = z.s->img_y;
    return (unsigned char*)result;
}

static void* TextureParsingWorker(void* argument) {
    TextureParseTask* task = (TextureParseTask*)argument;
    for (uint32_t i = task->start_tex; i < task->end_tex; ++i) {
        int32_t parserIdx = task->index_to_parser[i];
        if (unlikely(parserIdx < 0 || parserIdx >= (int32_t)task->parser->count)) continue;

        doubleSidedTexture[i] = task->parser->entries[parserIdx].doublesided;
        transparentTexture[i] = task->parser->entries[parserIdx].transparent;
        const char* data = task->raw_textures[i].data;
        int size = task->raw_textures[i].size;
        if (unlikely(!data || size <= 0)) continue;

        int w = 0, h = 0;
        uint8_t* pixels = stbi_load_from_memory_arena((const uint8_t*)data, size, &w, &h, &thread_stbi_arenas[task->tid]);
        if (!pixels || w < 1 || h < 1) {
            OS_DeallocateRAM((void*)data, (size_t)size);
            continue;
        }

        uint32_t numPixels = (uint32_t)w * h;
        uint8_t* indices = (uint8_t*)OS_AllocateRAM(NULL, numPixels, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
        uint32_t* palette = (uint32_t*)OS_AllocateRAM(NULL, 256 * sizeof(uint32_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, OS_INVALID_HANDLE);
        uint32_t pal_size = 0;
        uint8_t color_hash[1024] = {0};

        for (uint32_t p = 0; p < numPixels; ++p) {
            uint32_t color = ((uint32_t*)pixels)[p];
            uint32_t slot = (color * 0x9e3779b9u) & 1023;
            while (color_hash[slot]) {
                if (palette[color_hash[slot] - 1] == color) {
                    indices[p] = color_hash[slot] - 1;
                    goto found;
                }
                slot = (slot + 1) & 1023;
            }
            if (pal_size >= 256) {
                // Find closest existing palette entry instead of breaking
                uint32_t bestIdx = 0;
                uint32_t bestDist = UINT32_MAX;
                uint8_t r1 = color & 0xFF, g1 = (color>>8) & 0xFF;
                uint8_t b1 = (color>>16) & 0xFF, a1 = color>>24;
                for (uint32_t k = 0; k < pal_size; k++) {
                    int32_t dr = (int32_t)(palette[k] & 0xFF) - r1;
                    int32_t dg = (int32_t)((palette[k]>>8) & 0xFF) - g1;
                    int32_t db = (int32_t)((palette[k]>>16) & 0xFF) - b1;
                    int32_t da = (int32_t)(palette[k]>>24) - a1;
                    uint32_t dist = dr*dr + dg*dg + db*db + da*da;
                    if (dist < bestDist) { bestDist = dist; bestIdx = k; }
                }
                indices[p] = bestIdx;
                continue;
            }
            
            palette[pal_size] = color;
            indices[p] = pal_size;
            color_hash[slot] = (uint8_t)(pal_size + 1);
            ++pal_size;
        found:;
        }

        textureIndexBuffers[i] = indices;
        texturePaletteBuffers[i] = palette;
        texturePaletteSizes[i] = pal_size;
        textureWidths[i] = w;
        textureHeights[i] = h;

        OS_DeallocateRAM((void*)data, (size_t)size);
    }
    return NULL;
}

static bool ParseTextureData(TextureDataParser *parser, uint16_t maxSize, const char *filename) {
    OsFileHandle fd; int st_size; char* data = OS_OpenAndAllocateFileBufferReadonly(filename,&fd,&st_size);
    char* cursor = data; char* end = data + st_size;
    uint32_t lineNum = 0, max_index = 0;
    while (cursor < end) { // First pass: count entries and find max index
        char* start = cursor;
        while (cursor < end && *cursor != '\n' && *cursor != '\r') cursor++;
        size_t lineLen = cursor - start;
        lineNum++;
        if (lineLen <= 0) { cursor++; continue; }

        while (CharacterIsEmpty(*start)) start++; // Trim leading whitespace
        char *lineend = start + lineLen - 1;
        while (lineend > start && CharacterIsEmpty(*lineend)) lineend--; // Trim trailing whitespace
        if (*start == '\0' || (start[0] == '/' && start[1] == '/')) continue; // Skip empty lines and commented lines
        if (start[0] == '#') { continue; } // Skip entry start marker, only count ones with valid index thereafter in the key|value block lines

        char *colon = StringFindFirstCharWithin(start, ':');
        if (colon && StringCompareUpToLength(start, "index", colon - start) == 0) {
            char *value = colon + 1;
            while (CharacterIsEmpty(*value)) value++;
            uint32_t idx = parse_numberu32(value, start, lineNum);
            if (idx > max_index) max_index = idx;
       }
       
       if (cursor < end && *cursor == '\r') cursor++;
       if (cursor < end && *cursor == '\n') cursor++;
    }
    
    if (max_index == 0) { DualLogWarn("No entries found in %s\n", filename); OS_DeallocateRAM(data,st_size); return true; }
    if (max_index >= maxSize) { DualLogWarn("Too large of index found in %s, %u exceeds limit %u\n", filename, max_index, maxSize); OS_DeallocateRAM(data,st_size); return true; }

    uint32_t entry_count = max_index + 1;
    TextureData *new_entries = OS_AllocateRAM(NULL,entry_count * sizeof(TextureData),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);  
    parser->entries = new_entries;
    for (uint32_t i = 0; i < entry_count; ++i) { parser->entries[i] = (TextureData){ .index=UINT16_MAX, .transparent=false, .doublesided=false, .path={0} }; }
    parser->capacity = entry_count;
    parser->count = entry_count;
    TextureData entry = (TextureData){ .index=UINT16_MAX, .transparent=false, .doublesided=false, .path={0} };
    lineNum = 0;
    cursor = data; end = data + st_size; // Rewind
    while (cursor < end) {
        char* start = cursor;
        while (cursor < end && *cursor != '\n' && *cursor != '\r') cursor++;
        size_t lineLen = cursor - start;
        lineNum++;
        if (lineLen < 3) { cursor++; continue; } // Must have at least k:v, skip if shorter

        while (CharacterIsEmpty(*start)) start++; // Trim leading whitespace
        char *lineend = start + lineLen - 1;
        while (lineend > start && CharacterIsEmpty(*lineend)) lineend--; // Trim trailing whitespace
        if (start[0] == '/' && start[1] == '/') continue; // Skip comment(ed out) line

        if (*start == '#') {
            if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) parser->entries[entry.index] = entry;
            entry = (TextureData){ .index=UINT16_MAX, .transparent=false, .doublesided=false, .path={0} };
            if (lineend > start) {
                size_t actualLen = lineend - (start + 1) + 1;
                if (actualLen >= sizeof(entry.path)) actualLen = sizeof(entry.path) - 1;
                __builtin_memcpy(entry.path, start + 1, actualLen);
                entry.path[actualLen] = '\0';
            }
            continue;
        }

        // Handle key-value pair
        char *colon = StringFindFirstCharWithin(start, ':');
        if (colon) {
            char *key = start;
            char *value = colon + 1;
            while (CharacterIsEmpty(*key) && key < colon) key++;
            while (CharacterIsEmpty(*value) && value < lineend) value++;
            size_t keylen = colon - key; size_t vallen = (lineend >= value) ? (lineend - value + 1) : 0;
            if (keylen > 0 && vallen > 0) {
                char trimmed_key[256];
                char trimmed_value[256];
                StringCopyInto_A_SubstringFrom_B(trimmed_key, keylen, key, 256);
                StringCopyInto_A_SubstringFrom_B(trimmed_value, vallen, value, 256);
                char *key_end = trimmed_key + GetStringLength(trimmed_key) - 1;
                char *val_end = trimmed_value + GetStringLength(trimmed_value) - 1;
                while (key_end > trimmed_key && CharacterIsEmpty(*key_end)) *key_end-- = '\0';
                while (val_end > trimmed_value && CharacterIsEmpty(*val_end)) *val_end-- = '\0';
                     if (StringsEqual(trimmed_key,"index"))       entry.index = parse_numberu16(trimmed_value,start,lineNum);
                else if (StringsEqual(trimmed_key,"transparent")) entry.transparent = parse_bool(trimmed_value,start,lineNum);
                else if (StringsEqual(trimmed_key,"doublesided")) entry.doublesided = parse_bool(trimmed_value,start,lineNum);
            } else DualLogWarn("Invalid key-value pair at line %u: %s\n",lineNum,start);
        } else DualLogWarn("No colon found in line %u: %s\n",lineNum,start);
    }

    if (entry.path[0] && entry.index != UINT16_MAX && entry.index < parser->capacity) parser->entries[entry.index] = entry; // Store last entry
    OS_DeallocateRAM(data,st_size);
    return true;
}

void LoadTextures(void) {
    if (unlikely(loadedTexturesMaxIndex > 0)) return;

    double start_time = get_time();
    DebugRAM("start of LoadTextures");
    loadedTexturesMaxIndex = totalPixels = totalPaletteColors = 0u;
    TextureDataParser texture_parser;
    if (unlikely(!ParseTextureData(&texture_parser, MAX_VALID_TEXTURE, "./Data/textures.txt"))) {
        DualLogError("Could not parse ./Data/textures.txt!\n");
        OS_Exit(1);
    }

    int32_t maxIndex = -1;
    for (uint32_t k = 0; k < texture_parser.count; ++k) {
        if (texture_parser.entries[k].index > maxIndex && texture_parser.entries[k].index != UINT16_MAX) maxIndex = texture_parser.entries[k].index;
    }
    loadedTexturesMaxIndex = (uint16_t)(maxIndex + 1);
    if (loadedTexturesMaxIndex == 0) {
        DualLogError("No textures found in textures.txt\n");
        OS_Exit(1);
    }

    int32_t* indexToParser = OS_AllocateRAM(NULL, loadedTexturesMaxIndex * sizeof(int32_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, OS_INVALID_HANDLE);
    __builtin_memset(indexToParser, -1, loadedTexturesMaxIndex * sizeof(int32_t));
    for (uint32_t k = 0; k < texture_parser.count; ++k) {
        if (texture_parser.entries[k].index < loadedTexturesMaxIndex) indexToParser[texture_parser.entries[k].index] = (int32_t)k;
    }

    DualLog("Loading textures (%u) ... ", texture_parser.count);
    RawTexture* rawTextures = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * sizeof(RawTexture),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    __builtin_memset(rawTextures,0,loadedTexturesMaxIndex * sizeof(RawTexture));
    for (uint32_t i = 0; i < loadedTexturesMaxIndex; ++i) {
        int32_t parserIdx = indexToParser[i];
        if (parserIdx < 0) continue;
        const char* path = texture_parser.entries[parserIdx].path;
        OsFileHandle dummy_fd;
        int size = 0;
        rawTextures[i].data = (const char*)OS_OpenAndAllocateFileBufferReadonly(path,&dummy_fd,&size);
        rawTextures[i].size = size;
    }

    num_parse_threads = OS_GetNumThreads();
    if (num_parse_threads < 1) num_parse_threads = 1;
    if (num_parse_threads > 32) num_parse_threads = 32;
    thread_stbi_arenas = (StbiArena*)OS_AllocateRAM(NULL,(size_t)num_parse_threads * sizeof(StbiArena),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    for (int t = 0; t < num_parse_threads; ++t) {
        thread_stbi_arenas[t].base = NULL;
        stbi__arena_init_thread(&thread_stbi_arenas[t]);
    }

    textureIndexBuffers = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * sizeof(uint8_t*),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    texturePaletteBuffers = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * sizeof(uint32_t*),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    texturePaletteSizes = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * sizeof(uint32_t),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    textureWidths = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * sizeof(int32_t),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    textureHeights = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * sizeof(int32_t),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    __builtin_memset(textureIndexBuffers,0,loadedTexturesMaxIndex * sizeof(uint8_t*));
    __builtin_memset(texturePaletteBuffers,0,loadedTexturesMaxIndex * sizeof(uint32_t*));
    __builtin_memset(texturePaletteSizes,0,loadedTexturesMaxIndex * sizeof(uint32_t));
    __builtin_memset(textureWidths,0,loadedTexturesMaxIndex * sizeof(int32_t));
    __builtin_memset(textureHeights,0,loadedTexturesMaxIndex * sizeof(int32_t));
    TextureParseTask tasks[32];
    uint32_t chunk = (loadedTexturesMaxIndex + (uint32_t)num_parse_threads - 1U) / (uint32_t)num_parse_threads;
    for (int t = 0; t < num_parse_threads; ++t) {
        tasks[t].start_tex = (uint32_t)t * chunk;
        tasks[t].end_tex = tasks[t].start_tex + chunk;
        if (tasks[t].end_tex > loadedTexturesMaxIndex) tasks[t].end_tex = loadedTexturesMaxIndex;
        tasks[t].raw_textures = rawTextures;
        tasks[t].index_to_parser = indexToParser;
        tasks[t].parser = &texture_parser;
        tasks[t].tid = t;
    }

    pthread_t workers[32];
    for (int t = 0; t < num_parse_threads; ++t) pthread_create(&workers[t], NULL, TextureParsingWorker, &tasks[t]);
    for (int t = 0; t < num_parse_threads; ++t) pthread_join(workers[t], NULL);
    totalPixels = totalPaletteColors = 0u;
    for (uint16_t i = 0; i < loadedTexturesMaxIndex; ++i) {
        if (textureIndexBuffers[i]) {
            totalPixels += (uint32_t)textureWidths[i] * textureHeights[i];
            totalPaletteColors += texturePaletteSizes[i];
        }
    }

    size_t offsets_size = loadedTexturesMaxIndex * sizeof(uint32_t);
    size_t palettes_size = totalPaletteColors * sizeof(uint32_t);
    size_t indices_size = totalPixels;
    size_t arena_size = offsets_size + palettes_size + indices_size;
    void* arena = OS_AllocateRAM(NULL,arena_size,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE,OS_INVALID_HANDLE);
    uint8_t* cur = (uint8_t*)arena;
    uint32_t* textureOffsets = (uint32_t*)cur; cur += offsets_size;
    textureSizes = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * 2 * sizeof(int32_t),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    texturePaletteOffsets = OS_AllocateRAM(NULL,loadedTexturesMaxIndex * sizeof(uint32_t),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,OS_INVALID_HANDLE);
    uint32_t* texturePalettes = (uint32_t*)cur; cur += palettes_size;
    uint8_t* all_indices = cur;
    uint32_t pixel_base = 0, color_base = 0;
    for (uint16_t i=0;i<loadedTexturesMaxIndex;++i) {
        if (!textureIndexBuffers[i]) continue;
        uint32_t numP = (uint32_t)textureWidths[i] * textureHeights[i];
        uint32_t palS = texturePaletteSizes[i];
        textureOffsets[i] = pixel_base;
        texturePaletteOffsets[i] = color_base;
        textureSizes[i*2]     = textureWidths[i];
        textureSizes[i*2 + 1] = textureHeights[i];
        __builtin_memcpy(all_indices + pixel_base,textureIndexBuffers[i],numP);
        __builtin_memcpy(texturePalettes + color_base,texturePaletteBuffers[i],palS * sizeof(uint32_t));
        pixel_base += numP;
        color_base += palS;
        OS_DeallocateRAM(textureIndexBuffers[i],numP);
        OS_DeallocateRAM(texturePaletteBuffers[i],palS * sizeof(uint32_t));
    }

    DebugRAM("After loop for load textures");
    DualLog("total palette colors: %u, total pixels: %u...", totalPaletteColors,totalPixels);
    int32_t packed_size = ((int32_t)totalPixels + 3) / 4 * sizeof(uint32_t);
    glBindBuffer(GL_SSBO,Sys_Render.colorBufferID);
    void* dst = glMapBufferRange(GL_SSBO,0,packed_size,GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_RANGE_BIT);
    __builtin_memcpy(dst,all_indices,packed_size);
    glUnmapBuffer(GL_SSBO);
    glBindBuffer(GL_SSBO,Sys_Render.texturePalettesID);
    glBufferData(GL_SSBO,totalPaletteColors * sizeof(uint32_t),texturePalettes,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,Sys_Render.textureOffsetsID);
    glBufferData(GL_SSBO,loadedTexturesMaxIndex * sizeof(uint32_t),textureOffsets,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,Sys_Render.textureSizesID);
    glBufferData(GL_SSBO,loadedTexturesMaxIndex * 2 * sizeof(int32_t),textureSizes,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,Sys_Render.texturePaletteOffsetsID);
    glBufferData(GL_SSBO,loadedTexturesMaxIndex * sizeof(uint32_t),texturePaletteOffsets,GL_STATIC_DRAW);
    glBindBuffer(GL_SSBO,0);
    glFlush(); glFinish();
    OS_DeallocateRAM(texture_parser.entries,texture_parser.count * sizeof(TextureData));
    OS_DeallocateRAM(arena,arena_size);
    OS_DeallocateRAM(rawTextures,loadedTexturesMaxIndex * sizeof(RawTexture));
    OS_DeallocateRAM(indexToParser,loadedTexturesMaxIndex * sizeof(int32_t));
    OS_DeallocateRAM(textureIndexBuffers,loadedTexturesMaxIndex * sizeof(uint8_t*));
    OS_DeallocateRAM(texturePaletteBuffers,loadedTexturesMaxIndex * sizeof(uint32_t*));
    OS_DeallocateRAM(texturePaletteSizes,loadedTexturesMaxIndex * sizeof(uint32_t));
    OS_DeallocateRAM(textureWidths,loadedTexturesMaxIndex * sizeof(int32_t));
    OS_DeallocateRAM(textureHeights,loadedTexturesMaxIndex * sizeof(int32_t));
    for (int t=0;t<num_parse_threads;++t) OS_DeallocateRAM(thread_stbi_arenas[t].base,STBI_ARENA_SIZE);
    OS_DeallocateRAM(thread_stbi_arenas,(size_t)num_parse_threads * sizeof(StbiArena));
    double end_time = get_time();
    DualLog(" took %.6f secs\n",end_time - start_time);
    DebugRAM("After LoadTextures and after deallocation");
}
